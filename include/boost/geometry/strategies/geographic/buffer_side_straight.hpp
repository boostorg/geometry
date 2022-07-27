// Boost.Geometry

// Copyright (c) 2022 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_BUFFER_SIDE_STRAIGHT_HPP
#define BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_BUFFER_SIDE_STRAIGHT_HPP

#include <cstddef>

#include <boost/range/value_type.hpp>

#include <boost/geometry/srs/spheroid.hpp>
#include <boost/geometry/strategies/buffer.hpp>
#include <boost/geometry/strategies/geographic/parameters.hpp>
#include <boost/geometry/util/math.hpp>
#include <boost/geometry/util/select_calculation_type.hpp>
#include <boost/geometry/algorithms/azimuth.hpp>


namespace boost { namespace geometry
{

namespace strategy { namespace buffer
{

/*!
\brief Create a straight buffer along a side, on the Earth
\ingroup strategies
\details This strategy can be used as SideStrategy for the buffer algorithm.
 */
template
<
    typename FormulaPolicy = strategy::andoyer,
    typename Spheroid = srs::spheroid<double>,
    typename CalculationType = void
>
class geographic_side_straight
{
public :
#ifndef DOXYGEN_SHOULD_SKIP_THIS
    // Returns true if the buffer distance is always the same
    static inline bool equidistant()
    {
        return true;
    }


    template
    <
        typename Point,
        typename OutputRange,
        typename DistanceStrategy
    >
    inline result_code apply(Point const& input_p1, Point const& input_p2,
                             buffer_side_selector side,
                             DistanceStrategy const& distance_strategy,
                             OutputRange& output_range) const
    {
        using calc_t = typename select_calculation_type
            <
                Point,
                typename boost::range_value<OutputRange>::type,
                CalculationType
            >::type;

        calc_t const lon1_rad = get_as_radian<0>(input_p1);
        calc_t const lat1_rad = get_as_radian<1>(input_p1);
        calc_t const lon2_rad = get_as_radian<0>(input_p2);
        calc_t const lat2_rad = get_as_radian<1>(input_p2);
        if (lon1_rad == lon2_rad && lat1_rad == lat2_rad)
        {
            // Coordinates are simplified and therefore most often not equal.
            // But if simplify is skipped, or for lines with two
            // equal points, length is 0 and we cannot generate output.
            return result_no_output;
        }

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


        // Measure the angle from p1 to p2 with the Inverse transformation,
        // and subtract pi/2 to make it perpendicular.
        auto const inv = inverse_t::apply(lon1_rad, lat1_rad,
            lon2_rad, lat2_rad, m_spheroid);
        auto const angle = math::wrap_azimuth_in_radian(inv.azimuth -
            geometry::math::half_pi<calc_t>());

        // Calculate the distance and generate two points at that distance
        // with the Direct transformation
        auto const distance = distance_strategy.apply(input_p1, input_p2, side);
        auto const d1 = direct_t::apply(lon1_rad, lat1_rad, distance, angle, m_spheroid);
        auto const d2 = direct_t::apply(lon2_rad, lat2_rad, distance, angle, m_spheroid);

        output_range.resize(2);

        set_from_radian<0>(output_range.front(), d1.lon2);
        set_from_radian<1>(output_range.front(), d1.lat2);
        set_from_radian<0>(output_range.back(), d2.lon2);
        set_from_radian<1>(output_range.back(), d2.lat2);

        return result_normal;
    }
#endif // DOXYGEN_SHOULD_SKIP_THIS

private :
    Spheroid m_spheroid;
};


}} // namespace strategy::buffer

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_BUFFER_SIDE_STRAIGHT_HPP
