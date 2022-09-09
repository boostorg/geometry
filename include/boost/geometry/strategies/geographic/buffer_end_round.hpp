// Boost.Geometry

// Copyright (c) 2022 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_BUFFER_END_ROUND_HPP
#define BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_BUFFER_END_ROUND_HPP

#include <boost/range/value_type.hpp>

#include <boost/geometry/core/radian_access.hpp>

#include <boost/geometry/srs/spheroid.hpp>
#include <boost/geometry/strategies/buffer.hpp>
#include <boost/geometry/strategies/geographic/parameters.hpp>
#include <boost/geometry/util/math.hpp>
#include <boost/geometry/util/select_calculation_type.hpp>


namespace boost { namespace geometry
{

namespace strategy { namespace buffer
{

template
<
    typename FormulaPolicy = strategy::andoyer,
    typename Spheroid = srs::spheroid<double>,
    typename CalculationType = void
>
class geographic_end_round
{
    static bool const enable_azimuth = true;
    static bool const enable_coordinates = true;

    template <typename T>
    using inverse = typename FormulaPolicy::template inverse
        <
            T, false, enable_azimuth, false, false, false
        >;
    template <typename T>
    using direct = typename FormulaPolicy::template direct
        <
            T, enable_coordinates, false, false, false
        >;

public :

    //! \brief Constructs the strategy
    //! \param points_per_circle points which would be used for a full circle
    //! (if points_per_circle is smaller than 4, it is internally set to 4)
    explicit inline geographic_end_round(std::size_t points_per_circle = 90)
        : m_points_per_circle((points_per_circle < 4u) ? 4u : points_per_circle)
    {}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
    template <typename T, typename RangeOut>
    inline void generate(T lon_rad, T lat_rad, T distance, T azimuth, RangeOut& range_out) const
    {
        using point_t = typename boost::range_value<RangeOut const>::type;
        std::size_t const n = m_points_per_circle / 2;
        T const angle_diff = geometry::math::pi<T>() / n;
        T azi = math::wrap_azimuth_in_radian(azimuth + angle_diff);

        // Generate points between 0 and n, not including them
        // because left and right are inserted before and after this range.
        for (std::size_t i = 1; i < n; i++)
        {
            auto const d = direct<T>::apply(lon_rad, lat_rad, distance, azi, m_spheroid);
            point_t point;
            set_from_radian<0>(point, d.lon2);
            set_from_radian<1>(point, d.lat2);
            range_out.emplace_back(point);
            azi = math::wrap_azimuth_in_radian(azi + angle_diff);
        }
    }

    //! Fills output_range with a round end
    template <typename Point, typename RangeOut, typename DistanceStrategy>
    inline void apply(Point const& penultimate_point, Point const& perp_left_point,
                      Point const& ultimate_point, Point const& perp_right_point,
                      buffer_side_selector side, DistanceStrategy const& distance,
                      RangeOut& range_out) const
    {
        using calc_t = typename select_calculation_type
            <
                Point,
                typename boost::range_value<RangeOut>::type,
                CalculationType
            >::type;

        calc_t const dist_left = distance.apply(penultimate_point, ultimate_point, buffer_side_left);
        calc_t const dist_right = distance.apply(penultimate_point, ultimate_point, buffer_side_right);

        bool const reversed = (side == buffer_side_left && dist_right < 0 && -dist_right > dist_left)
                    || (side == buffer_side_right && dist_left < 0 && -dist_left > dist_right)
                    ;

        calc_t const lon_rad = get_as_radian<0>(ultimate_point);
        calc_t const lat_rad = get_as_radian<1>(ultimate_point);
        calc_t const lon1_rad = get_as_radian<0>(perp_left_point);
        calc_t const lat1_rad = get_as_radian<1>(perp_left_point);

        auto const azimuth = inverse<calc_t>::apply(lon_rad, lat_rad, lon1_rad, lat1_rad, m_spheroid).azimuth;

        if (reversed)
        {
            range_out.push_back(perp_right_point);
            // generate
            range_out.push_back(perp_left_point);
        }
        else
        {
            range_out.push_back(perp_left_point);

            if (geometry::math::equals(dist_left, dist_right))
            {
                generate(lon_rad, lat_rad, dist_left, azimuth, range_out);
            }
            else
            {
                static calc_t const two = 2.0;
                calc_t const dist_average = (dist_left + dist_right) / two;
                calc_t const dist_half
                        = (side == buffer_side_right
                        ? (dist_right - dist_left)
                        : (dist_left - dist_right)) / two;
                auto const shifted = direct<calc_t>::apply(lon_rad, lat_rad, dist_half, azimuth, m_spheroid);
                generate(shifted.lon2, shifted.lat2, dist_average, azimuth, range_out);
            }

            range_out.push_back(perp_right_point);
        }
    }

    template <typename NumericType>
    static inline NumericType max_distance(NumericType const& distance)
    {
        return distance;
    }

    //! Returns the piece_type (flat end)
    static inline piece_type get_piece_type()
    {
        return buffered_round_end;
    }
#endif // DOXYGEN_SHOULD_SKIP_THIS

private :
    std::size_t m_points_per_circle;
    Spheroid m_spheroid;
};


}} // namespace strategy::buffer

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_BUFFER_END_ROUND_HPP
