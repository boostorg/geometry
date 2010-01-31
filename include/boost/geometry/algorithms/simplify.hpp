// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_SIMPLIFY_HPP
#define BOOST_GEOMETRY_ALGORITHMS_SIMPLIFY_HPP

#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>


#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/core/ring_type.hpp>
#include <boost/geometry/core/exterior_ring.hpp>
#include <boost/geometry/core/interior_rings.hpp>

#include <boost/geometry/geometries/concepts/check.hpp>

#include <boost/geometry/strategies/agnostic/simplify_douglas_peucker.hpp>
#include <boost/geometry/strategies/concepts/simplify_concept.hpp>

#include <boost/geometry/algorithms/clear.hpp>



/*!
\defgroup simplify simplify: remove points from a geometry, keeping shape (simplification or generalization)
\par Source description:
- Wikipedia: given a 'curve' composed of line segments to find a curve
    not too dissimilar but that has fewer points

\see http://en.wikipedia.org/wiki/Ramer-Douglas-Peucker_algorithm

\par Performance
- Performance is measured on simplification of a collection of rings,
    such that 10% of the points is kept.
- 2776 counties of US are simplified in 0.7 seconds
(http://trac.osgeo.org/ggl/wiki/Performance#Simplify1)

\par Geometries
- \b linestring:
\image html svg_simplify_road.png
This US Road originally contained 34 points, the simplified version contains 7 points

- \b polygon:
\image html svg_simplify_country.png
This country (Belgium) originally contained 55 points, the simplified version contains 24 points

\note simplifying a valid simple polygon (which never intersects itself)
    might result in an invalid polygon, where the simplified rings intersect
    themselves or one of the other outer or inner rings.
Efficient simplification of a ring/polygon is still an "Open Problem"
(http://maven.smith.edu/~orourke/TOPP/P24.html#Problem.24)

- \b multi_linestring
- \b multi_polygon


*/

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace simplify {

template<typename Range, typename Strategy>
struct simplify_range_inserter
{
    template <typename OutputIterator>
    static inline void apply(Range const& range, OutputIterator out,
                    double max_distance, Strategy const& strategy)
    {
        if (boost::size(range) <= 2 || max_distance < 0)
        {
            std::copy(boost::begin(range), boost::end(range), out);
        }
        else
        {
            strategy.apply(range, out, max_distance);
        }
    }
};


template<typename Range, typename Strategy>
struct simplify_copy
{
    static inline void apply(Range const& range, Range& out,
                    double max_distance, Strategy const& strategy)
    {
        std::copy
            (
                boost::begin(range), boost::end(range), std::back_inserter(out)
            );
    }
};


template<typename Range, typename Strategy, std::size_t Minimum>
struct simplify_range
{
    static inline void apply(Range const& range, Range& out,
                    double max_distance, Strategy const& strategy)
    {
        // Call do_container for a linestring / ring

        /* For a RING:
            The first/last point (the closing point of the ring) should maybe
            be excluded because it lies on a line with second/one but last.
            Here it is never excluded.

            Note also that, especially if max_distance is too large,
            the output ring might be self intersecting while the input ring is
            not, although chances are low in normal polygons

            Finally the inputring might have 4 points (=correct),
                the output < 4(=wrong)
        */

        if (boost::size(range) <= int(Minimum) || max_distance < 0.0)
        {
            simplify_copy<Range, Strategy>::apply
                (
                    range, out, max_distance, strategy
                );
        }
        else
        {
            simplify_range_inserter<Range, Strategy>::apply
                (
                    range, std::back_inserter(out), max_distance, strategy
                );
        }
    }
};

template<typename Polygon, typename Strategy>
struct simplify_polygon
{
    static inline void apply(Polygon const& poly_in, Polygon& poly_out,
                    double max_distance, Strategy const& strategy)
    {
        typedef typename ring_type<Polygon>::type ring_type;

        typedef typename boost::range_iterator
            <typename interior_type<Polygon>::type>::type iterator_type;
        typedef typename boost::range_const_iterator
            <typename interior_type<Polygon>::type>::type const_iterator_type;

        // Note that if there are inner rings, and distance is too large,
        // they might intersect with the outer ring in the output,
        // while it didn't in the input.
        simplify_range<ring_type, Strategy, 4>::apply(exterior_ring(poly_in),
                        exterior_ring(poly_out),
                        max_distance, strategy);

        // Note: here a resizeable container is assumed.
        // Maybe we should make this part of the concept.
        interior_rings(poly_out).resize(num_interior_rings(poly_in));

        iterator_type it_out = boost::begin(interior_rings(poly_out));

        for (const_iterator_type it_in = boost::begin(interior_rings(poly_in));
            it_in != boost::end(interior_rings(poly_in));
            ++it_in, ++it_out)
        {
            simplify_range<ring_type, Strategy, 4>::apply(*it_in,
                        *it_out, max_distance, strategy);
        }
    }
};


}} // namespace detail::simplify
#endif // DOXYGEN_NO_DETAIL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template <typename Tag, typename Geometry, typename Strategy>
struct simplify
{
};

template <typename Point, typename Strategy>
struct simplify<point_tag, Point, Strategy>
{
    static inline void apply(Point const& point, Point& out,
                    double max_distance, Strategy const& strategy)
    {
        copy_coordinates(point, out);
    }
};


template <typename Linestring, typename Strategy>
struct simplify<linestring_tag, Linestring, Strategy>
    : detail::simplify::simplify_range
            <
                Linestring,
                Strategy,
                2
            >
{};

template <typename Ring, typename Strategy>
struct simplify<ring_tag, Ring, Strategy>
    : detail::simplify::simplify_range
            <
                Ring,
                Strategy,
                4
            >
{};

template <typename Polygon, typename Strategy>
struct simplify<polygon_tag, Polygon, Strategy>
    : detail::simplify::simplify_polygon
            <
                Polygon,
                Strategy
            >
{};


template <typename Tag, typename Geometry, typename Strategy>
struct simplify_inserter
{
};


template <typename Linestring, typename Strategy>
struct simplify_inserter<linestring_tag, Linestring, Strategy>
    : detail::simplify::simplify_range_inserter
            <
                Linestring,
                Strategy
            >
{};

template <typename Ring, typename Strategy>
struct simplify_inserter<ring_tag, Ring, Strategy>
    : detail::simplify::simplify_range_inserter
            <
                Ring,
                Strategy
            >
{};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


/*!
    \brief Simplify a geometry using a specified strategy
    \ingroup simplify
    \param geometry input geometry, to be simplified
    \param out output geometry, simplified version of the input geometry
    \param max_distance distance (in units of input coordinates) of a vertex
        to other segments to be removed
    \param strategy simplify strategy to be used for simplification, might
        include point-distance strategy
 */
template<typename Geometry, typename Strategy>
inline void simplify(Geometry const& geometry, Geometry& out,
                     double max_distance, Strategy const& strategy)
{
    concept::check<Geometry>();

    BOOST_CONCEPT_ASSERT( (geometry::concept::SimplifyStrategy<Strategy>) );

    geometry::clear(out);

    dispatch::simplify
        <
            typename tag<Geometry>::type,
            Geometry,
            Strategy
        >::apply(geometry, out, max_distance, strategy);
}




/*!
    \brief Simplify a geometry
    \ingroup simplify
    \note This version of simplify simplifies a geometry using the default
        strategy (Douglas Peucker),
    \param geometry input geometry, to be simplified
    \param out output geometry, simplified version of the input geometry
    \param max_distance distance (in units of input coordinates) of a vertex
        to other segments to be removed
    \par Example:
    The simplify algorithm can be used as following:
    \dontinclude doxygen_1.cpp
    \skip example_simplify_linestring1
    \line {
    \until }
 */
template<typename Geometry>
inline void simplify(Geometry const& geometry, Geometry& out,
                     double max_distance)
{
    concept::check<Geometry>();

    typedef typename point_type<Geometry>::type point_type;
    typedef typename cs_tag<point_type>::type cs_tag;
    typedef typename strategy_distance_segment
        <
            cs_tag,
            cs_tag,
            point_type,
            point_type
        >::type ds_strategy_type;

    typedef strategy::simplify::douglas_peucker
        <
            point_type, ds_strategy_type
        > strategy_type;

    simplify(geometry, out, max_distance, strategy_type());
}


/*!
    \brief Simplify a geometry, using an output iterator
        and a specified strategy
    \ingroup simplify
    \param geometry input geometry, to be simplified
    \param out output iterator, outputs all simplified points
    \param max_distance distance (in units of input coordinates) of a vertex
        to other segments to be removed
    \param strategy simplify strategy to be used for simplification,
        might include point-distance strategy
    \par Example:
    simplify_inserter with strategy is used as following:
    \dontinclude doxygen_1.cpp
    \skip example_simplify_linestring2
    \line {
    \until }
 */
template<typename Geometry, typename OutputIterator, typename Strategy>
inline void simplify_inserter(Geometry const& geometry, OutputIterator out,
                              double max_distance, Strategy const& strategy)
{
    concept::check<const Geometry>();
    BOOST_CONCEPT_ASSERT( (geometry::concept::SimplifyStrategy<Strategy>) );

    dispatch::simplify_inserter
        <
            typename tag<Geometry>::type,
            Geometry,
            Strategy
        >::apply(geometry, out, max_distance, strategy);
}

/*!
    \brief Simplify a geometry, using an output iterator
    \ingroup simplify
    \param geometry input geometry, to be simplified
    \param out output iterator, outputs all simplified points
    \param max_distance distance (in units of input coordinates) of a vertex
        to other segments to be removed
 */
template<typename Geometry, typename OutputIterator>
inline void simplify_inserter(Geometry const& geometry, OutputIterator out,
                              double max_distance)
{
    typedef typename point_type<Geometry>::type point_type;

    // Concept: output point type = point type of input geometry
    concept::check<const Geometry>();
    concept::check<point_type>();

    typedef typename cs_tag<point_type>::type cs_tag;
    typedef typename strategy_distance_segment
        <
            cs_tag,
            cs_tag,
            point_type,
            point_type
        >::type ds_strategy_type;

    typedef strategy::simplify::douglas_peucker
        <
            point_type, ds_strategy_type
        > strategy_type;

    dispatch::simplify_inserter
        <
            typename tag<Geometry>::type,
            Geometry,
            strategy_type
        >::apply(geometry, out, max_distance, strategy_type());
}


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_SIMPLIFY_HPP
