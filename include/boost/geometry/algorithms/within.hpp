// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_WITHIN_HPP
#define BOOST_GEOMETRY_ALGORITHMS_WITHIN_HPP

/*!
\defgroup within within: detect if a geometry is inside another geometry, a.o. point-in-polygon

\par Source descriptions:
- OGC: Returns 1 (TRUE) if this geometric object is "spatially within"
        another Geometry.

\par Performance
- 2776 within determinations using bounding box and polygon are done
    in 0.05 seconds (http://trac.osgeo.org/ggl/wiki/Performance#Within1)
- note that using another strategy the performance can be increased:
  - winding            : 0.093 s
  - franklin           : 0.062 s
  - Crossings-multiply : 0.047 s
- but note also that the last two do not detect point-on-border cases

\par Geometries:
- \b point + \b polygon: The well-known point-in-polygon, returning true if
    a point falls within a polygon (and not
within one of its holes) \image html within_polygon.png
- \b point + \b ring: returns true if point is completely within
    a ring \image html within_ring.png
- \b point + \b box
- \b box + \b box

\par Example:
The within algorithm is used as following:
\dontinclude doxygen_1.cpp
\skip example_within
\line {
\until }

*/


#include <cstddef>

#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>

#include <boost/geometry/algorithms/make.hpp>

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/exterior_ring.hpp>
#include <boost/geometry/core/interior_rings.hpp>
#include <boost/geometry/core/cs.hpp>

#include <boost/geometry/geometries/concepts/check.hpp>

#include <boost/geometry/strategies/point_in_poly.hpp>
#include <boost/geometry/strategies/concepts/within_concept.hpp>


namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace within {


/*!
    \brief Implementation for boxes
    \ingroup boolean_relations
    \note Should have strategy for e.g. Wrangel
 */
template
<
    typename Point,
    typename Box,
    typename Strategy,
    std::size_t Dimension,
    std::size_t DimensionCount
>
struct point_in_box
{
    static inline bool apply(Point const& p, Box const& b, Strategy const& s)
    {
        assert_dimension_equal<Point, Box>();

        if (get<Dimension>(p) <= get<min_corner, Dimension>(b)
            || get<Dimension>(p) >= get<max_corner, Dimension>(b))
        {
            return false;
        }

        return point_in_box
            <
                Point,
                Box,
                Strategy,
                Dimension + 1,
                DimensionCount
            >::apply(p, b, s);
    }
};

template
<
    typename Point,
    typename Box,
    typename Strategy,
    std::size_t DimensionCount
>
struct point_in_box<Point, Box, Strategy, DimensionCount, DimensionCount>
{
    static inline bool apply(Point const& , Box const& , Strategy const& )
    {
        return true;
    }
};


template
<
    typename Box1,
    typename Box2,
    typename Strategy,
    std::size_t Dimension,
    std::size_t DimensionCount
>
struct box_in_box
{
    static inline bool apply(Box1 const& b1, Box2 const& b2, Strategy const& s)
    {
        assert_dimension_equal<Box1, Box2>();

        if (get<min_corner, Dimension>(b1) <= get<min_corner, Dimension>(b2)
            || get<max_corner, Dimension>(b1) >= get<max_corner, Dimension>(b2))
        {
            return false;
        }

        return box_in_box
            <
                Box1,
                Box2,
                Strategy,
                Dimension + 1,
                DimensionCount
            >::apply(b1, b2, s);
    }
};

template
<
    typename Box1,
    typename Box2,
    typename Strategy,
    std::size_t DimensionCount
>
struct box_in_box<Box1, Box2, Strategy, DimensionCount, DimensionCount>
{
    static inline bool apply(Box1 const& , Box2 const& , Strategy const&)
    {
        return true;
    }
};


template<typename Point, typename Ring, typename Strategy>
struct point_in_ring
{
    BOOST_CONCEPT_ASSERT( (geometry::concept::WithinStrategy<Strategy>) );

    static inline bool apply(Point const& point, Ring const& ring,
            Strategy const& strategy)
    {
        if (boost::size(ring) < 4)
        {
            return false;
        }

        typedef typename boost::range_const_iterator<Ring>::type iterator_type;

        typename Strategy::state_type state;

        iterator_type it = boost::begin(ring);
        for (iterator_type previous = it++;
            it != boost::end(ring);
            previous = it++)
        {
            if (! strategy.apply(point, *previous, *it, state))
            {
                return false;
            }
        }
        return strategy.result(state);
    }
};

// Polygon: in exterior ring, and if so, not within interior ring(s)
template<typename Point, typename Polygon, typename Strategy>
struct point_in_polygon
{
    BOOST_CONCEPT_ASSERT( (geometry::concept::WithinStrategy<Strategy>) );

    static inline bool apply(Point const& point, Polygon const& poly,
            Strategy const& strategy)
    {

        typedef point_in_ring
            <
                Point,
                typename ring_type<Polygon>::type,
                Strategy
            > per_ring;

        if (per_ring::apply(point, exterior_ring(poly), strategy))
        {

            for (typename boost::range_const_iterator
                    <
                        typename interior_type<Polygon>::type
                    >::type it = boost::begin(interior_rings(poly));
                 it != boost::end(interior_rings(poly));
                 ++it)
            {
                if (per_ring::apply(point, *it, strategy))
                {
                    return false;
                }
            }
            return true;
        }
        return false;
    }
};

}} // namespace detail::within
#endif // DOXYGEN_NO_DETAIL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template
<
    typename Tag1,
    typename Tag2,
    typename Geometry1,
    typename Geometry2,
    typename Strategy
>
struct within
{};


template <typename Point, typename Box, typename Strategy>
struct within<point_tag, box_tag, Point, Box, Strategy>
    : detail::within::point_in_box
            <
                Point,
                Box,
                Strategy,
                0,
                dimension<Point>::type::value
            >
{};

template <typename Box1, typename Box2, typename Strategy>
struct within<box_tag, box_tag, Box1, Box2, Strategy>
    : detail::within::box_in_box
            <
                Box1,
                Box2,
                Strategy,
                0,
                dimension<Box1>::type::value
            >
{};



template <typename Point, typename Ring, typename Strategy>
struct within<point_tag, ring_tag, Point, Ring, Strategy>
    : detail::within::point_in_ring
        <Point, Ring, Strategy>
{};

template <typename Point, typename Polygon, typename Strategy>
struct within<point_tag, polygon_tag, Point, Polygon, Strategy>
    : detail::within::point_in_polygon
        <Point, Polygon, Strategy>
{};

} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


/*!
    \brief Within, examine if a geometry is within another geometry
    \ingroup within
    \param geometry1 geometry which might be within the second geometry
    \param geometry2 geometry which might contain the first geometry
    \return true if geometry1 is completely contained within geometry2,
        else false
    \note The default strategy is used for within detection

 */
template<typename Geometry1, typename Geometry2>
inline bool within(Geometry1 const& geometry1, Geometry2 const& geometry2)
{
    concept::check<const Geometry1>();
    concept::check<const Geometry2>();

    typedef typename point_type<Geometry1>::type point_type1;
    typedef typename point_type<Geometry2>::type point_type2;

    typedef typename strategy_within
        <
            typename cs_tag<point_type1>::type,
            typename cs_tag<point_type2>::type,
            point_type1,
            point_type2
        >::type strategy_type;

    return dispatch::within
        <
            typename tag<Geometry1>::type,
            typename tag<Geometry2>::type,
            Geometry1,
            Geometry2,
            strategy_type
        >::apply(geometry1, geometry2, strategy_type());
}

/*!
    \brief Within, examine if a geometry is within another geometry,
        using a specified strategy
    \ingroup within
    \param geometry1 geometry which might be within the second geometry
    \param geometry2 geometry which might contain the first geometry
    \param strategy strategy to be used
    \return true if geometry1 is completely contained within geometry2,
        else false
 */
template<typename Geometry1, typename Geometry2, typename Strategy>
inline bool within(Geometry1 const& geometry1, Geometry2 const& geometry2,
        Strategy const& strategy)
{
    // Always assume a point-in-polygon strategy here.
    // Because for point-in-box, it makes no sense to specify one.
    BOOST_CONCEPT_ASSERT( (geometry::concept::WithinStrategy<Strategy>) );

    concept::check<const Geometry1>();
    concept::check<const Geometry2>();

    return dispatch::within
        <
            typename tag<Geometry1>::type,
            typename tag<Geometry2>::type,
            Geometry1,
            Geometry2,
            Strategy
        >::apply(geometry1, geometry2, strategy);
}

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_WITHIN_HPP
