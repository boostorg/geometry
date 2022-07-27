// Boost.Geometry

// Copyright (c) 2022 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_BUFFER_JOIN_ROUND_HPP
#define BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_BUFFER_JOIN_ROUND_HPP

#include <cstddef>

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
class geographic_join_round
{
public :

    //! \brief Constructs the strategy
    //! \param points_per_circle points which would be used for a full circle
    explicit inline geographic_join_round(std::size_t points_per_circle = 90)
        : m_points_per_circle(points_per_circle)
    {}

    //! Fills output_range with a rounded shape around a vertex
    template
    <
        typename Point,
        typename DistanceType,
        typename RangeOut
    >
    inline bool apply(Point const &ip, Point const &vertex,
                      Point const &perp1, Point const &perp2,
                      DistanceType const &buffer_distance,
                      RangeOut &range_out) const
    {
        using calc_t = typename select_calculation_type
            <
                Point,
                typename boost::range_value<RangeOut>::type,
                CalculationType
            >::type;

        calc_t const lon_rad = get_as_radian<0>(vertex);
        calc_t const lat_rad = get_as_radian<1>(vertex);
        calc_t const lon1_rad = get_as_radian<0>(perp1);
        calc_t const lat1_rad = get_as_radian<1>(perp1);
        calc_t const lon2_rad = get_as_radian<0>(perp2);
        calc_t const lat2_rad = get_as_radian<1>(perp2);

        // Define the types for the Formulas to calculate a point
        // at a certain distance (using <direct> with coordinates)
        // and to calculate the angle between two specified points
        // (using <inverse> with azimuth)
        // See also boost/geometry/strategies/geographic/parameters.hpp
        constexpr bool enable_azimuth = true;
        constexpr bool enable_coordinates = true;

        using direct_t = typename FormulaPolicy::template direct
            <
                calc_t, enable_coordinates, false, false, false
            >;

        using inverse_t = typename FormulaPolicy::template inverse
            <
                calc_t, false, enable_azimuth, false, false, false
            >;

        calc_t const two_pi = geometry::math::two_pi<calc_t>();

        // Calculate angles from vertex to perp1/perp2
        auto const inv1 = inverse_t::apply(lon_rad, lat_rad, lon1_rad, lat1_rad, m_spheroid);
        auto const inv2 = inverse_t::apply(lon_rad, lat_rad, lon2_rad, lat2_rad, m_spheroid);

        // For a sharp corner, perpendicular points are nearly opposite and the
        // angle between the two azimuths can be nearly 180, but not more.
        bool const wrapped = inv2.azimuth < inv1.azimuth;
        calc_t const angle_diff = wrapped
            ? ((two_pi + inv2.azimuth) - inv1.azimuth)
            : inv2.azimuth - inv1.azimuth;
        BOOST_GEOMETRY_ASSERT(angle_diff >= 0);
        BOOST_GEOMETRY_ASSERT(angle_diff <= geometry::math::pi<calc_t>());

        calc_t const circle_fraction = angle_diff / two_pi;
        std::size_t const n = (std::max)(static_cast<std::size_t>(
            std::ceil(m_points_per_circle * circle_fraction)), std::size_t(1));

        calc_t const diff = angle_diff / static_cast<calc_t>(n);
        calc_t azi = math::wrap_azimuth_in_radian(inv1.azimuth + diff);

        range_out.push_back(perp1);

        // Generate points between 0 and n, not including them
        // because perp1 and perp2 are inserted before and after this range.
        for (std::size_t i = 1; i < n; i++)
        {
            auto const d = direct_t::apply(lon_rad, lat_rad, buffer_distance, azi, m_spheroid);
            Point p;
            set_from_radian<0>(p, d.lon2);
            set_from_radian<1>(p, d.lat2);
            range_out.emplace_back(p);

            azi = math::wrap_azimuth_in_radian(azi + diff);
        }

        range_out.push_back(perp2);
        return true;
    }

    template <typename NumericType>
    static inline NumericType max_distance(NumericType const& distance)
    {
        return distance;
    }

private :
    std::size_t m_points_per_circle;
    Spheroid m_spheroid;
};


}} // namespace strategy::buffer

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_BUFFER_JOIN_ROUND_HPP
