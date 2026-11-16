// Boost.Geometry
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_GEOMETRY_STRATEGIES_SPHERICAL_CLIP_BOX_HPP
#define BOOST_GEOMETRY_STRATEGIES_SPHERICAL_CLIP_BOX_HPP

#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>
#include <vector>

#include <boost/geometry/core/radian_access.hpp>
#include <boost/geometry/formulas/spherical.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/util/math.hpp>
#include <boost/geometry/util/select_most_precise.hpp>

namespace boost { namespace geometry { namespace strategy { namespace intersection
{

namespace clipping_detail
{

template <typename T>
struct angular_point
{
    T lon, lat;
};

template <typename T>
T longitude_difference(T value)
{
    T const pi = math::pi<T>();
    T const period = 2 * pi;
    value = std::fmod(value, period);
    if (value > pi)
        value -= period;
    else if (value < -pi)
        value += period;
    return value;
}

template <typename T>
bool angular_equal(T a, T b)
{
    return std::abs(a - b) <= 64 * std::numeric_limits<T>::epsilon()
        * (1 + (std::max)(std::abs(a), std::abs(b)));
}

template <typename Curve>
class geodesic_box_intervals
{
    using T = typename Curve::calculation_type;
    Curve& m_curve;
    T m_start_lon, m_delta_lon;

    T longitude(T t)
    {
        T const expected = m_start_lon + t * m_delta_lon;
        return expected + longitude_difference(m_curve.at(t).lon - expected);
    }

    T latitude(T t)
    {
        return m_curve.at(t).lat;
    }

    T extremum(bool maximum)
    {
        T low = 0, high = 1;
        for (unsigned i = 0; i < 96; ++i)
        {
            T const left = (2 * low + high) / 3;
            T const right = (low + 2 * high) / 3;
            if (left == low || right == high)
                break;
            bool const rising = latitude(left) < latitude(right);
            if (rising == maximum)
                low = left;
            else
                high = right;
        }
        return (low + high) / 2;
    }

    template <typename Function>
    static void roots(Function value, T low, T high, T bound, std::vector<T>& cuts)
    {
        T const first = value(low), last = value(high);
        bool const first_on = angular_equal(first, bound);
        bool const last_on = angular_equal(last, bound);
        if (first_on)
            cuts.push_back(low);
        if (last_on)
            cuts.push_back(high);
        if (first_on || last_on || (first < bound) == (last < bound) || first == last)
            return;
        bool const increasing = first < last;
        for (unsigned i = 0; i < 96; ++i)
        {
            T const mid = (low + high) / 2;
            if (mid == low || mid == high)
                break;
            if ((value(mid) < bound) == increasing)
                low = mid;
            else
                high = mid;
        }
        cuts.push_back((low + high) / 2);
    }

    static bool between(T value, T low, T high)
    {
        return (value >= low || angular_equal(value, low))
            && (value <= high || angular_equal(value, high));
    }

public:
    explicit geodesic_box_intervals(Curve& curve)
        : m_curve(curve), m_start_lon(curve.at(0).lon),
          m_delta_lon(longitude_difference(curve.at(1).lon - m_start_lon))
    {}

    template <typename Box>
    std::vector<std::pair<T, T>> apply(Box const& box)
    {
        T const pi = math::pi<T>(), period = 2 * pi;
        T const west = get_as_radian<min_corner, 0>(box);
        T const east = get_as_radian<max_corner, 0>(box);
        T const south = get_as_radian<min_corner, 1>(box);
        T const north = get_as_radian<max_corner, 1>(box);
        T width = east - west;
        if (width < 0)
            width += period;
        bool const all_longitudes = width >= period || angular_equal(width, period);

        auto inside = [&](T t)
        {
            auto const p = m_curve.at(t);
            if (!between(p.lat, south, north))
                return false;
            if (all_longitudes || angular_equal(std::abs(p.lat), pi / 2))
                return true;
            T offset = std::fmod(p.lon - west, period);
            if (offset < 0)
                offset += period;
            return between(offset, T(0), width) || angular_equal(offset, period);
        };

        std::vector<T> monotone = {0, extremum(false), extremum(true), 1};
        std::sort(monotone.begin(), monotone.end());
        monotone.erase(std::unique(monotone.begin(), monotone.end(), angular_equal<T>), monotone.end());
        monotone.back() = 1;
        std::vector<T> cuts = monotone;
        for (std::size_t i = 1; i < monotone.size(); ++i)
        {
            T const low = monotone[i - 1], high = monotone[i];
            auto lat = [&](T t) { return latitude(t); };
            roots(lat, low, high, south, cuts);
            roots(lat, low, high, north, cuts);
            if (!all_longitudes)
            {
                T const lon1 = longitude(low), lon2 = longitude(high);
                T const minimum = (std::min)(lon1, lon2);
                T const maximum = (std::max)(lon1, lon2);
                auto lon = [&](T t) { return longitude(t); };
                for (T edge : {west, west + width})
                {
                    T first = std::ceil((minimum - edge) / period);
                    T last = std::floor((maximum - edge) / period);
                    for (T shift = first; shift <= last; ++shift)
                        roots(lon, low, high, edge + shift * period, cuts);
                }
            }
        }

        std::sort(cuts.begin(), cuts.end());
        cuts.erase(std::unique(cuts.begin(), cuts.end(), angular_equal<T>), cuts.end());
        cuts.back() = 1;
        std::vector<std::pair<T, T>> intervals;
        for (std::size_t i = 0; i < cuts.size(); ++i)
        {
            if (inside(cuts[i]))
                intervals.emplace_back(cuts[i], cuts[i]);
            if (i + 1 < cuts.size() && inside((cuts[i] + cuts[i + 1]) / 2))
                intervals.emplace_back(cuts[i], cuts[i + 1]);
        }
        std::vector<std::pair<T, T>> merged;
        for (auto const& interval : intervals)
        {
            if (!merged.empty() && angular_equal(merged.back().second, interval.first))
                merged.back().second = interval.second;
            else
                merged.push_back(interval);
        }
        return merged;
    }
};

template <typename T>
class spherical_clip_curve
{
    using point_type = model::point<T, 2, cs::spherical_equatorial<radian>>;
    using vector_type = model::point<T, 3, cs::cartesian>;
    point_type m_first, m_last;
    vector_type m_origin, m_tangent;
    T m_angle;

public:
    using calculation_type = T;

    template <typename Point>
    spherical_clip_curve(Point const& first, Point const& last)
        : m_first(get_as_radian<0>(first), get_as_radian<1>(first)),
          m_last(get_as_radian<0>(last), get_as_radian<1>(last))
    {
        m_origin = formula::sph_to_cart3d<vector_type>(m_first);
        auto const end = formula::sph_to_cart3d<vector_type>(m_last);
        auto const normal = geometry::cross_product(m_origin, end);
        T const sine = std::sqrt(geometry::dot_product(normal, normal));
        m_angle = std::atan2(sine, geometry::dot_product(m_origin, end));
        if (sine != 0)
        {
            auto const tangent = geometry::cross_product(normal, m_origin);
            m_tangent = vector_type(get<0>(tangent) / sine,
                                    get<1>(tangent) / sine,
                                    get<2>(tangent) / sine);
        }
    }

    angular_point<T> at(T t)
    {
        if (t == 0)
            return {get<0>(m_first), get<1>(m_first)};
        if (t == 1)
            return {get<0>(m_last), get<1>(m_last)};
        if (m_angle == 0)
            return {get<0>(m_first) + t * longitude_difference(get<0>(m_last) - get<0>(m_first)),
                    get<1>(m_first) + t * (get<1>(m_last) - get<1>(m_first))};
        T const cosine = std::cos(t * m_angle), sine = std::sin(t * m_angle);
        T const x = get<0>(m_origin) * cosine + get<0>(m_tangent) * sine;
        T const y = get<1>(m_origin) * cosine + get<1>(m_tangent) * sine;
        T const z = get<2>(m_origin) * cosine + get<2>(m_tangent) * sine;
        return {std::atan2(y, x), std::atan2(z, std::hypot(x, y))};
    }
};

} // namespace clipping_detail

template <typename CalculationType = void>
struct spherical_box_clip
{
    template <typename Point>
    auto curve(Point const& first, Point const& last) const
    {
        using T = typename select_most_precise<coordinate_type_t<Point>, double, CalculationType>::type;
        return clipping_detail::spherical_clip_curve<T>(first, last);
    }
};

}}}} // namespace boost::geometry::strategy::intersection

#endif
