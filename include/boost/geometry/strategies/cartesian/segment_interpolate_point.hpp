// Boost.Geometry

// Copyright (c) 2018, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_SEGMENT_INTERPOLATE_POINT_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_SEGMENT_INTERPOLATE_POINT_HPP

#include <boost/geometry/core/assert.hpp>
#include <boost/geometry/core/coordinate_dimension.hpp>
#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/strategies/segment_interpolate_point.hpp>
#include <boost/geometry/strategies/cartesian/distance_pythagoras.hpp>
#include <boost/geometry/util/select_most_precise.hpp>


namespace boost { namespace geometry
{

namespace strategy { namespace segment_interpolate_point
{


/*!
\brief Interpolate point on a cartesian segment.
\ingroup strategies
\tparam CalculationType \tparam_calculation
*/
template
<
    typename CalculationType = void,
    typename Strategy = distance::pythagoras<CalculationType>
>
class cartesian
{
public:

    // point-point strategy getters
    struct distance_pp_strategy
    {
        typedef Strategy type;
    };

    inline typename distance_pp_strategy::type get_distance_pp_strategy() const
    {
        return typename distance_pp_strategy::type();
    }

    //result type
    template <typename Point>
    struct result_type
    {
        typedef typename select_most_precise
            <
                typename coordinate_type<Point>::type,
                CalculationType
            >::type calc_t;

        result_type() :
            distance(0)
        {}

        result_type(calc_t d) :
            distance(d)
        {}

        calc_t distance;
    };

    template <typename Point>
    inline result_type<Point> compute(Point const& p0,
                                      Point const& p1) const
    {
        return result_type<Point>(Strategy().apply(p0,p1));
    }

    template <typename Point, typename T1, typename T2>
    inline void apply(Point const& p0,
                      Point const& p1,
                      T1 const& fraction,
                      Point & p,
                      T2 const& distance) const
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
        T1 const one_minus_fraction = 1-fraction;
        for_each_coordinate(cp1, detail::value_operation<T1, std::multiplies>(fraction));
        for_each_coordinate(cp0, detail::value_operation<T1, std::multiplies>(one_minus_fraction));
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
    typedef strategy::segment_interpolate_point::cartesian<> type;
};


} // namespace services
#endif // DOXYGEN_NO_STRATEGY_SPECIALIZATIONS


}} // namespace strategy::segment_interpolate_point


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_CARTESIAN_SEGMENT_INTERPOLATE_POINT_HPP
