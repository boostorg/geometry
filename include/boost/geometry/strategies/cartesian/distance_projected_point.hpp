// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_DISTANCE_PROJECTED_POINT_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_DISTANCE_PROJECTED_POINT_HPP


#include <boost/mpl/if.hpp>
#include <boost/type_traits.hpp>
#include <boost/type_traits/remove_const.hpp>

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/point_type.hpp>

#include <boost/geometry/arithmetic/arithmetic.hpp>
#include <boost/geometry/arithmetic/dot_product.hpp>

#include <boost/geometry/strategies/tags.hpp>
#include <boost/geometry/strategies/distance.hpp>
#include <boost/geometry/strategies/distance_result.hpp>
#include <boost/geometry/strategies/cartesian/distance_pythagoras.hpp>

#include <boost/geometry/util/select_coordinate_type.hpp>
#include <boost/geometry/util/copy.hpp>


// Helper geometry
#include <boost/geometry/geometries/segment.hpp>

namespace boost { namespace geometry {

namespace strategy { namespace distance {



/*!
    \brief Strategy for distance point to segment
    \ingroup distance
    \details Calculates distance using projected-point method, and (optionally) Pythagoras
    \author Adapted from: http://geometryalgorithms.com/Archive/algorithm_0102/algorithm_0102.htm
    \tparam P point type
    \tparam PointOfSegment segment type
    \tparam Strategy strategy, optional, defaults to pythagoras
    \par Concepts for Strategy:
    - cartesian_distance operator(Point,Point)
*/
template
<
    typename Point,
    typename PointOfSegment,
    typename Strategy = pythagoras
        <
            Point,
            typename point_type<PointOfSegment>::type
        >
>
struct projected_point
{
    typedef Point point_type;
    typedef PointOfSegment segment_point_type;
    typedef typename select_coordinate_type
        <
            Point,
            PointOfSegment
        >::type coordinate_type;
    typedef cartesian_distance<coordinate_type> return_type;

    typedef Strategy point_strategy_type;


    inline return_type apply(Point const& p,
                    PointOfSegment const& p1, PointOfSegment const& p2) const
    {
        assert_dimension_equal<Point, PointOfSegment>();

        /* Algorithm
        POINT v(x2 - x1, y2 - y1);
        POINT w(px - x1, py - y1);
        c1 = w . v
        c2 = v . v
        b = c1 / c2
        RETURN POINT(x1 + b * vx, y1 + b * vy);
        */


        // Take here the first point type. It should have a default constructor.
        // That is not required for the second point type.
        Point v, w;

        copy_coordinates(p2, v);
        copy_coordinates(p, w);
        subtract_point(v, p1);
        subtract_point(w, p1);

        Strategy strategy;

        coordinate_type zero = coordinate_type();
        coordinate_type c1 = dot_product(w, v);
        if (c1 <= zero)
        {
            return strategy.apply(p, p1);
        }
        coordinate_type c2 = dot_product(v, v);
        if (c2 <= c1)
        {
            return strategy.apply(p, p2);
        }

        // Even in case of char's, we have to turn to a point<double/float>
        // because of the division.
        typedef typename geometry::select_most_precise<coordinate_type, double>::type divisor_type;
        divisor_type b = c1 / c2;

        // Note that distances with integer coordinates do NOT work because
        // - the project point is integer
        // - if we solve that, the used distance_strategy cannot handle double points
        PointOfSegment projected;
        copy_coordinates(p1, projected);
        multiply_value(v, b);
        add_point(projected, v);

        return strategy.apply(p, projected);

    }

};

}} // namespace strategy::distance



#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS


template <typename Point, typename PointOfSegment>
struct strategy_distance_segment<cartesian_tag, cartesian_tag, Point, PointOfSegment>
{
    typedef typename point_type<PointOfSegment>::type segment_point_type;

    typedef strategy::distance::projected_point
    <
        Point,
        PointOfSegment,
        typename strategy_distance
        <
            cartesian_tag, cartesian_tag, Point, segment_point_type
        >::type
    > type;
};
#endif


template <typename Point, typename PointOfSegment, typename PPStrategy>
struct strategy_tag<strategy::distance::projected_point<Point, PointOfSegment, PPStrategy> >
{
    typedef strategy_tag_distance_point_segment type;
};



}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_STRATEGIES_CARTESIAN_DISTANCE_PROJECTED_POINT_HPP
