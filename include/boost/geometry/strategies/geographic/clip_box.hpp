// Boost.Geometry
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_CLIP_BOX_HPP
#define BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_CLIP_BOX_HPP

#include <boost/geometry/strategies/geographic/parameters.hpp>
#include <boost/geometry/strategies/spherical/clip_box.hpp>

namespace boost { namespace geometry { namespace strategy { namespace intersection
{

namespace clipping_detail
{

template <typename T, typename FormulaPolicy, typename Spheroid>
class geographic_clip_curve
{
    using inverse_type = typename FormulaPolicy::template inverse<T, true, true, false, false, false>;
    using direct_type = typename FormulaPolicy::template direct<T, true, false, false, false>;
    angular_point<T> m_first, m_last;
    Spheroid m_spheroid;
    T m_distance, m_azimuth;

    angular_point<T> endpoint_error(T distance, T azimuth) const
    {
        auto const end = direct_type::apply(m_first.lon, m_first.lat, distance,
                                             azimuth, m_spheroid);
        T const delta = longitude_difference(end.lon2 - m_last.lon);
        T const cos_lat = std::cos(end.lat2);
        return {cos_lat * std::sin(delta),
                std::sin(end.lat2) * std::cos(m_last.lat)
                    - cos_lat * std::cos(delta) * std::sin(m_last.lat)};
    }

    void refine_endpoint()
    {
        if (m_distance == 0)
            return;
        T const epsilon = std::numeric_limits<T>::epsilon();
        T const angle_step = std::sqrt(epsilon);
        T const distance_step = angle_step * (std::max)(
            std::abs(m_distance), T(get_radius<0>(m_spheroid)));
        for (unsigned iteration = 0; iteration < 12; ++iteration)
        {
            auto const error = endpoint_error(m_distance, m_azimuth);
            T const norm = math::sqr(error.lon) + math::sqr(error.lat);
            if (norm <= 64 * math::sqr(epsilon))
                break;
            auto const d_plus = endpoint_error(m_distance + distance_step, m_azimuth);
            auto const d_minus = endpoint_error(m_distance - distance_step, m_azimuth);
            auto const a_plus = endpoint_error(m_distance, m_azimuth + angle_step);
            auto const a_minus = endpoint_error(m_distance, m_azimuth - angle_step);
            T const dx = (d_plus.lon - d_minus.lon) / (2 * distance_step);
            T const dy = (d_plus.lat - d_minus.lat) / (2 * distance_step);
            T const ax = (a_plus.lon - a_minus.lon) / (2 * angle_step);
            T const ay = (a_plus.lat - a_minus.lat) / (2 * angle_step);
            T const determinant = dx * ay - dy * ax;
            if (determinant == 0)
                break;
            T const distance_delta = (error.lon * ay - error.lat * ax) / determinant;
            T const angle_delta = (dx * error.lat - dy * error.lon) / determinant;
            bool improved = false;
            for (T scale = 1; scale >= T(1) / 128; scale /= 2)
            {
                T const distance = m_distance - scale * distance_delta;
                T const azimuth = m_azimuth - scale * angle_delta;
                if (distance < 0)
                    continue;
                auto const candidate = endpoint_error(distance, azimuth);
                if (math::sqr(candidate.lon) + math::sqr(candidate.lat) < norm)
                {
                    m_distance = distance;
                    m_azimuth = azimuth;
                    improved = true;
                    break;
                }
            }
            if (!improved)
                break;
        }
    }

public:
    using calculation_type = T;

    template <typename Point>
    geographic_clip_curve(Point const& first, Point const& last, Spheroid const& spheroid)
        : m_first{get_as_radian<0>(first), get_as_radian<1>(first)},
          m_last{get_as_radian<0>(last), get_as_radian<1>(last)},
          m_spheroid(spheroid)
    {
        auto const inverse = inverse_type::apply(m_first.lon, m_first.lat,
                                                m_last.lon, m_last.lat, m_spheroid);
        m_distance = inverse.distance;
        m_azimuth = inverse.azimuth;
        refine_endpoint();
    }

    angular_point<T> at(T t)
    {
        if (t == 0 || m_distance == 0)
            return m_first;
        if (t == 1)
            return m_last;
        auto const point = direct_type::apply(m_first.lon, m_first.lat, t * m_distance,
                                             m_azimuth, m_spheroid);
        return {point.lon2, point.lat2};
    }
};

} // namespace clipping_detail

template <typename FormulaPolicy, typename Spheroid, typename CalculationType = void>
class geographic_box_clip
{
    Spheroid m_spheroid;

public:
    explicit geographic_box_clip(Spheroid const& spheroid) : m_spheroid(spheroid) {}

    template <typename Point>
    auto curve(Point const& first, Point const& last) const
    {
        using T = typename select_most_precise<coordinate_type_t<Point>, double, CalculationType>::type;
        return clipping_detail::geographic_clip_curve<T, FormulaPolicy, Spheroid>(first, last, m_spheroid);
    }
};

}}}} // namespace boost::geometry::strategy::intersection

#endif
