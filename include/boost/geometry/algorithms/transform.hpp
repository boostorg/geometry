// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_TRANSFORM_HPP
#define BOOST_GEOMETRY_ALGORITHMS_TRANSFORM_HPP

#include <cmath>
#include <iterator>

#include <boost/range.hpp>

#include <boost/geometry/algorithms/clear.hpp>
#include <boost/geometry/algorithms/assign.hpp>

#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/core/exterior_ring.hpp>
#include <boost/geometry/core/interior_rings.hpp>
#include <boost/geometry/core/ring_type.hpp>

#include <boost/geometry/geometries/concepts/check.hpp>

#include <boost/geometry/strategies/transform.hpp>


/*!
\defgroup transform transform: apply transformations on geometries
\brief Transforms from one geometry to another geometry, optionally using a strategy
\details The transform algorithm automatically transforms from one coordinate system to another coordinate system.
If the coordinate system of both geometries are the same, the geometry is copied. All point(s of the geometry)
are transformed.

There is a version without a strategy, transforming automatically, and there is a version with a strategy.

This function has a lot of appliances, for example
- transform from spherical coordinates to cartesian coordinates, and back
- transform from geographic coordinates to cartesian coordinates (projections) and back
- transform from degree to radian, and back
- transform from and to cartesian coordinates (mapping, translations, etc)

The automatic transformations look to the coordinate system family, and dimensions, of the point type and by this
apply the strategy (internally bounded by traits classes).

\par Examples:
The example below shows automatic transformations to go from one coordinate system to another one:
\dontinclude doxygen_2.cpp
\skip example_for_transform()
\skipline XYZ
\until endl;

The next example takes another approach and transforms from Cartesian to Cartesian:
\skipline XY
\until endl;

\note Not every possibility is yet worked out, e.g. polar coordinate system is ignored until now
\note This "transform" is broader then geodetic datum transformations, those are currently not worked out

*/

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace transform
{

template <typename Point1, typename Point2, typename Strategy>
struct transform_point
{
    static inline bool apply(Point1 const& p1, Point2& p2,
                Strategy const& strategy)
    {
        return strategy.apply(p1, p2);
    }
};

template <typename Box1, typename Box2, typename Strategy>
struct transform_box
{
    static inline bool apply(Box1 const& b1, Box2& b2,
                Strategy const& strategy)
    {
        typedef typename point_type<Box1>::type point_type1;
        typedef typename point_type<Box2>::type point_type2;

        point_type1 lower_left, upper_right;
        detail::assign::assign_box_2d_corner<min_corner, min_corner>(
                    b1, lower_left);
        detail::assign::assign_box_2d_corner<max_corner, max_corner>(
                    b1, upper_right);

        point_type2 p1, p2;
        if (strategy.apply(lower_left, p1) && strategy.apply(upper_right, p2))
        {
            // Create a valid box and therefore swap if necessary
            typedef typename coordinate_type<point_type2>::type coordinate_type;
            coordinate_type x1 = geometry::get<0>(p1)
                    , y1  = geometry::get<1>(p1)
                    , x2  = geometry::get<0>(p2)
                    , y2  = geometry::get<1>(p2);

            if (x1 > x2) { std::swap(x1, x2); }
            if (y1 > y2) { std::swap(y1, y2); }

            set<min_corner, 0>(b2, x1);
            set<min_corner, 1>(b2, y1);
            set<max_corner, 0>(b2, x2);
            set<max_corner, 1>(b2, y2);

            return true;
        }
        return false;
    }
};

template
<
    typename PointOut,
    typename OutputIterator,
    typename Range,
    typename Strategy
>
inline bool transform_range_out(Range const& range,
    OutputIterator out, Strategy const& strategy)
{
    PointOut point_out;
    for(typename boost::range_const_iterator<Range>::type
        it = boost::begin(range);
        it != boost::end(range);
        ++it)
    {
        if (! transform_point
                <
                    typename point_type<Range>::type,
                    PointOut,
                    Strategy
                >::apply(*it, point_out, strategy))
        {
            return false;
        }
        *out = point_out;
        ++out;
    }
    return true;
}

template <typename Polygon1, typename Polygon2, typename Strategy>
struct transform_polygon
{
    static inline bool apply(const Polygon1& poly1, Polygon2& poly2,
                Strategy const& strategy)
    {
        typedef typename interior_type<Polygon1>::type interior1_type;
        typedef typename interior_type<Polygon2>::type interior2_type;
        typedef typename ring_type<Polygon1>::type ring1_type;
        typedef typename ring_type<Polygon2>::type ring2_type;
        typedef typename point_type<Polygon2>::type point2_type;

        geometry::clear(poly2);

        if (!transform_range_out<point2_type>(exterior_ring(poly1),
                    std::back_inserter(exterior_ring(poly2)), strategy))
        {
            return false;
        }

        interior_rings(poly2).resize(num_interior_rings(poly1));

        typedef typename boost::range_const_iterator
            <
                interior1_type
            >::type iterator1_type;
        typedef typename boost::range_iterator
            <
                interior2_type
            >::type iterator2_type;

        iterator1_type it1 = boost::begin(interior_rings(poly1));
        iterator2_type it2 = boost::begin(interior_rings(poly2));
        for ( ; it1 != boost::end(interior_rings(poly1));
            ++it1, ++it2)
        {
            if (!transform_range_out<point2_type>(*it1,
                std::back_inserter(*it2), strategy))
            {
                return false;
            }
        }

        return true;
    }
};


template <typename Point1, typename Point2>
struct select_strategy
{
    typedef typename strategy_transform
        <
        typename cs_tag<Point1>::type,
        typename cs_tag<Point2>::type,
        typename coordinate_system<Point1>::type,
        typename coordinate_system<Point2>::type,
        dimension<Point1>::type::value,
        dimension<Point2>::type::value,
        typename point_type<Point1>::type,
        typename point_type<Point2>::type
        >::type type;
};

template <typename Range1, typename Range2, typename Strategy>
struct transform_range
{
    static inline bool apply(Range1 const& range1,
            Range2& range2, Strategy const& strategy)
    {
        typedef typename point_type<Range2>::type point_type;

        geometry::clear(range2);
        return transform_range_out<point_type>(range1,
                std::back_inserter(range2), strategy);
    }
};

}} // namespace detail::transform
#endif // DOXYGEN_NO_DETAIL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template
<
    typename Tag1, typename Tag2,
    typename Geometry1, typename Geometry2,
    typename Strategy
>
struct transform {};

template <typename Point1, typename Point2, typename Strategy>
struct transform<point_tag, point_tag, Point1, Point2, Strategy>
    : detail::transform::transform_point<Point1, Point2, Strategy>
{
};


template <typename Linestring1, typename Linestring2, typename Strategy>
struct transform
    <
        linestring_tag, linestring_tag,
        Linestring1, Linestring2, Strategy
    >
    : detail::transform::transform_range<Linestring1, Linestring2, Strategy>
{
};

template <typename Range1, typename Range2, typename Strategy>
struct transform<ring_tag, ring_tag, Range1, Range2, Strategy>
    : detail::transform::transform_range<Range1, Range2, Strategy>
{
};

template <typename Polygon1, typename Polygon2, typename Strategy>
struct transform<polygon_tag, polygon_tag, Polygon1, Polygon2, Strategy>
    : detail::transform::transform_polygon<Polygon1, Polygon2, Strategy>
{
};

template <typename Box1, typename Box2, typename Strategy>
struct transform<box_tag, box_tag, Box1, Box2, Strategy>
    : detail::transform::transform_box<Box1, Box2, Strategy>
{
};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


/*!
    \brief Transforms from one geometry to another geometry using a strategy
    \ingroup transform
    \tparam Geometry1 first geometry type
    \tparam Geometry2 second geometry type
    \tparam Strategy strategy
    \param geometry1 first geometry
    \param geometry2 second geometry
    \param strategy the strategy to be used for transformation
 */
template <typename Geometry1, typename Geometry2, typename Strategy>
inline bool transform(Geometry1 const& geometry1, Geometry2& geometry2,
            Strategy const& strategy)
{
    concept::check<const Geometry1>();
    concept::check<Geometry2>();

    typedef dispatch::transform
        <
            typename tag<Geometry1>::type,
            typename tag<Geometry2>::type,
            Geometry1,
            Geometry2,
            Strategy
        > transform_type;

    return transform_type::apply(geometry1, geometry2, strategy);
}

/*!
    \brief Transforms from one geometry to another geometry using a strategy
    \ingroup transform
    \tparam Geometry1 first geometry type
    \tparam Geometry2 second geometry type
    \param geometry1 first geometry
    \param geometry2 second geometry
    \return true if the transformation could be done
 */
template <typename Geometry1, typename Geometry2>
inline bool transform(Geometry1 const& geometry1, Geometry2& geometry2)
{
    concept::check<const Geometry1>();
    concept::check<Geometry2>();

    typename detail::transform::select_strategy<Geometry1, Geometry2>::type strategy;
    return transform(geometry1, geometry2, strategy);
}

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_TRANSFORM_HPP
