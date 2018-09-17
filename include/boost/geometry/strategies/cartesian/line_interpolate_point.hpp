// Boost.Geometry

// Copyright (c) 2018, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_LINE_INTERPOLATE_POINT_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_LINE_INTERPOLATE_POINT_HPP

#include <boost/geometry/core/assert.hpp>
#include <boost/geometry/core/coordinate_dimension.hpp>
#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/strategies/line_interpolate_point.hpp>
#include <boost/geometry/util/select_most_precise.hpp>


namespace boost { namespace geometry
{

namespace strategy { namespace line_interpolate_point
{


/*!
\brief Interpolate point on a cartesian segment.
\ingroup strategies
\tparam CalculationType \tparam_calculation
*/
template
<
    typename CalculationType = void
>
class cartesian
{
public:

    template <typename Point, typename T>
    inline void apply(Point const& p0,
                      Point const& p1,
                      T const& fraction,
                      Point & p) const
    {
        typedef typename select_most_precise
            <
                typename coordinate_type<Point>::type,
                CalculationType
            >::type calc_t;

        typedef model::point
            <
                calc_t,
                geometry::dimension<Point>::value,
                cs::cartesian
            > calc_point_t;

        calc_point_t cp0, cp1;
        geometry::detail::conversion::convert_point_to_point(p0, cp0);
        geometry::detail::conversion::convert_point_to_point(p1, cp1);

        //segment convex combination: p0*fraction + p1*(1-fraction)
        T const one_minus_fraction = 1-fraction;
        for_each_coordinate(cp1, detail::value_operation<T, std::multiplies>(fraction));
        for_each_coordinate(cp0, detail::value_operation<T, std::multiplies>(one_minus_fraction));
        for_each_coordinate(cp1, detail::point_operation<calc_point_t, std::plus>(cp0));

        assert_dimension_equal<calc_point_t, Point>();
        geometry::detail::conversion::convert_point_to_point(cp1, p);
    }
};


#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS
namespace services
{

template <>
struct default_strategy<cartesian_tag>
{
    typedef strategy::line_interpolate_point::cartesian<> type;
};


} // namespace services
#endif // DOXYGEN_NO_STRATEGY_SPECIALIZATIONS


}} // namespace strategy::line_interpolate_point


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_CARTESIAN_LINE_INTERPOLATE_POINT_HPP
