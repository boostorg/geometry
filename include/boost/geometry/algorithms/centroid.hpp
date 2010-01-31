// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Copyright (c) 2009 Mateusz Loskot <mateusz@loskot.net>
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_CENTROID_HPP
#define BOOST_GEOMETRY_ALGORITHMS_CENTROID_HPP

#include <cstddef>

#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>

#include <boost/geometry/algorithms/distance.hpp>
#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/core/coordinate_dimension.hpp>
#include <boost/geometry/core/exception.hpp>
#include <boost/geometry/core/exterior_ring.hpp>
#include <boost/geometry/core/interior_rings.hpp>
#include <boost/geometry/geometries/concepts/check.hpp>
#include <boost/geometry/iterators/segment_iterator.hpp>
#include <boost/geometry/strategies/centroid.hpp>
#include <boost/geometry/strategies/concepts/centroid_concept.hpp>
#include <boost/geometry/util/copy.hpp>
#include <boost/geometry/util/for_each_coordinate.hpp>


/*!
\defgroup centroid centroid: calculate centroid (center of gravity) of a geometry
\par Source descriptions:
- OGC description: The mathematical centroid for this Surface as a Point. The
    result is not guaranteed to be on this Surface.
- From Wikipedia: Informally, it is the "average" of all points
\see http://en.wikipedia.org/wiki/Centroid
\note Polygon should be closed, and can be orientated either way
\note The "centroid" functions are taking a non const reference to the centroid.
    The "make_centroid" functions return the centroid, the type has to be
    specified.
\note Both of them have an overloaded version where
    a centroid calculation strategy can be specified
\exception centroid_exception if calculation is not successful,
    e.g. because polygon didn't contain points

\par Example:
Example showing centroid calculation
\dontinclude doxygen_1.cpp
\skip example_centroid_polygon
\line {
\until }

\par Performance
2776 * 1000 centroid calculations are done in 0.16 seconds
(http://trac.osgeo.org/ggl/wiki/Performance#Centroid1)

\par Geometries:
- RING: \image html centroid_ring.png
- BOX: the centroid of a 2D or 3D box is the center of the box
- POLYGON \image html centroid_polygon.png
- POINT: the point is the centroid
- LINESTRING: the average of the centers of its segments
- MULTIPOINT: the average of the points
*/

namespace boost { namespace geometry
{

class centroid_exception : public geometry::exception
{
public:

    inline centroid_exception() {}

    virtual char const* what() const throw()
    {
        return "GGL Centroid calculation exception";
    }
};

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace centroid {

template<typename Point, typename PointCentroid, typename Strategy>
struct centroid_point
{
    static inline void apply(Point const& point, PointCentroid& centroid,
            Strategy const&)
    {
        copy_coordinates(point, centroid);
    }
};

template
<
    typename Box,
    typename Point,
    std::size_t Dimension,
    std::size_t DimensionCount
>
struct centroid_box_calculator
{
    typedef typename select_coordinate_type
        <
            Box, Point
        >::type coordinate_type;
    static inline void apply(Box const& box, Point& centroid)
    {
        coordinate_type const c1 = get<min_corner, Dimension>(box);
        coordinate_type const c2 = get<max_corner, Dimension>(box);
        coordinate_type m = c1 + c2;
        m /= 2.0;

        set<Dimension>(centroid, m);

        centroid_box_calculator
            <
                Box, Point,
                Dimension + 1, DimensionCount
            >::apply(box, centroid);
    }
};

template<typename Box, typename Point, std::size_t DimensionCount>
struct centroid_box_calculator<Box, Point, DimensionCount, DimensionCount>
{
    static inline void apply(Box const& , Point& )
    {
    }
};

template<typename Box, typename Point, typename Strategy>
struct centroid_box
{
    static inline void apply(Box const& box, Point& centroid,
            Strategy const&)
    {
        centroid_box_calculator
            <
                Box, Point,
                0, dimension<Box>::type::value
            >::apply(box, centroid);
    }
};

// There is one thing where centroid is different from e.g. within.
// If the ring has only one point, it might make sense that
// that point is the centroid.
template<typename Point, typename Range>
inline bool range_ok(Range const& range, Point& centroid)
{
    std::size_t const n = boost::size(range);
    if (n > 1)
    {
        return true;
    }
    else if (n <= 0)
    {
#if defined(CENTROID_WITH_CATCH)
        throw centroid_exception();
#endif
        return false;
    }
    else // if (n == 1)
    {
        // Take over the first point in a "coordinate neutral way"
        copy_coordinates(range.front(), centroid);
        return false;
    }
    return true;
}


/*!
    \brief Calculate the centroid of a ring.
*/
template<typename Ring, typename Strategy>
struct centroid_ring_state
{
    static inline void apply(Ring const& ring,
            Strategy const& strategy, typename Strategy::state_type& state)
    {
        typedef typename boost::range_const_iterator<Ring>::type iterator_type;
        iterator_type it = boost::begin(ring);
        for (iterator_type previous = it++;
            it != boost::end(ring);
            previous = it++)
        {
            Strategy::apply(*previous, *it, state);
        }

        /* using segment_iterator: nice, well looking, but much slower...
            normal iterator: 0.156 s
            segment iterator: 1.985 s...
        typedef segment_iterator
            <
                typename boost::range_const_iterator<Ring>::type,
                typename point_type<Ring>::type
            > iterator_type;

        iterator_type it(boost::begin(ring), boost::end(ring));
        iterator_type end(boost::end(ring));
        for(; it != end; ++it)
        {
            Strategy::apply(it->first, it->second, state);
        }
        */
    }
};

template<typename Ring, typename Point, typename Strategy>
struct centroid_ring
{
    static inline void apply(Ring const& ring, Point& centroid,
            Strategy const& strategy)
    {
        if (range_ok(ring, centroid))
        {
            typename Strategy::state_type state;
            centroid_ring_state
                <
                    Ring,
                    Strategy
                >::apply(ring, strategy, state);
            Strategy::result(state, centroid);
        }
    }
};

/*!
    \brief Centroid of a linestring.
*/
template<typename Linestring, typename Point, typename Strategy>
struct centroid_linestring
{
    static inline void apply(Linestring const& line, Point& centroid,
            Strategy const& strategy)
    {
        // First version, should
        // - be moved to a strategy
        // - be made dim-agnostic

        typedef typename point_type<Linestring>::type point_type;
        typedef typename boost::range_const_iterator<Linestring>::type point_iterator_type;
        typedef segment_iterator<point_iterator_type, point_type> segment_iterator;

        double length = double();
        std::pair<double, double> average_sum;

        segment_iterator it(boost::begin(line), boost::end(line));
        segment_iterator end(boost::end(line));
        while (it != end)
        {
            double const d = geometry::distance(it->first, it->second);
            length += d;

            double const mx = (get<0>(it->first) + get<0>(it->second)) / 2;
            double const my = (get<1>(it->first) + get<1>(it->second)) / 2;
            average_sum.first += d * mx;
            average_sum.second += d * my;
            ++it;
        }

        set<0>(centroid, average_sum.first / length );
        set<1>(centroid, average_sum.second / length );
    }
};

/*!
    \brief Centroid of a polygon.
    \note Because outer ring is clockwise, inners are counter clockwise,
    triangle approach is OK and works for polygons with rings.
*/
template<typename Polygon, typename Strategy>
struct centroid_polygon_state
{
    static inline void apply(Polygon const& poly,
            Strategy const& strategy, typename Strategy::state_type& state)
    {
        typedef centroid_ring_state
            <
                typename ring_type<Polygon>::type,
                Strategy
            > per_ring;

        per_ring::apply(exterior_ring(poly), strategy, state);

        for (typename boost::range_const_iterator
                <
                    typename interior_type<Polygon>::type
                >::type it = boost::begin(interior_rings(poly));
             it != boost::end(interior_rings(poly));
             ++it)
        {
            per_ring::apply(*it, strategy, state);
        }
    }
};

template<typename Polygon, typename Point, typename Strategy>
struct centroid_polygon
{
    static inline void apply(Polygon const& poly, Point& centroid,
            Strategy const& strategy)
    {
        if (range_ok(exterior_ring(poly), centroid))
        {
            typename Strategy::state_type state;
            centroid_polygon_state
                <
                    Polygon,
                    Strategy
                >::apply(poly, strategy, state);
            Strategy::result(state, centroid);
        }
    }
};


}} // namespace detail::centroid
#endif // DOXYGEN_NO_DETAIL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template
<
    typename Tag,
    typename Geometry,
    typename Point,
    typename Strategy
>
struct centroid {};

template
<
    typename Geometry,
    typename Point,
    typename Strategy
>
struct centroid<point_tag, Geometry, Point, Strategy>
    : detail::centroid::centroid_point<Geometry, Point, Strategy>
{};

template
<
    typename Box,
    typename Point,
    typename Strategy
>
struct centroid<box_tag, Box, Point, Strategy>
    : detail::centroid::centroid_box<Box, Point, Strategy>
{};

template <typename Ring, typename Point, typename Strategy>
struct centroid<ring_tag, Ring, Point, Strategy>
    : detail::centroid::centroid_ring<Ring, Point, Strategy>
{};

template <typename Linestring, typename Point, typename Strategy>
struct centroid<linestring_tag, Linestring, Point, Strategy>
    : detail::centroid::centroid_linestring<Linestring, Point, Strategy>
 {};

template <typename Polygon, typename Point, typename Strategy>
struct centroid<polygon_tag, Polygon, Point, Strategy>
    : detail::centroid::centroid_polygon<Polygon, Point, Strategy>
 {};

} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


/*!
    \brief Calculate centroid using a specified strategy
    \ingroup centroid
    \param geometry the geometry to calculate centroid from
    \param c reference to point which will contain the centroid
    \param strategy Calculation strategy for centroid
 */
template<typename Geometry, typename Point, typename Strategy>
inline void centroid(Geometry const& geometry, Point& c,
        Strategy const& strategy)
{
    //BOOST_CONCEPT_ASSERT( (geometry::concept::CentroidStrategy<Strategy>) );

    concept::check_concepts_and_equal_dimensions<Point, const Geometry>();

    typedef typename point_type<Geometry>::type point_type;

    // Call dispatch apply method. That one returns true if centroid
    // should be taken from state.
    dispatch::centroid
        <
            typename tag<Geometry>::type,
            Geometry,
            Point,
            Strategy
        >::apply(geometry, c, strategy);
}


/*!
    \brief Calculate centroid
    \ingroup centroid
    \param geometry a geometry (e.g. closed ring or polygon)
    \param c reference to point which will contain the centroid
 */
template<typename Geometry, typename Point>
inline void centroid(Geometry const& geometry, Point& c)
{
    concept::check_concepts_and_equal_dimensions<Point, const Geometry>();

    typedef typename strategy_centroid
        <
            typename cs_tag<Geometry>::type,
            typename tag<Geometry>::type,
            dimension<Geometry>::type::value,
            Point,
            Geometry
        >::type strategy_type;

    centroid(geometry, c, strategy_type());
}


/*!
    \brief Calculate and return centroid
    \ingroup centroid
    \param geometry the geometry to calculate centroid from
    \return the centroid
 */
template<typename Point, typename Geometry>
inline Point make_centroid(Geometry const& geometry)
{
    concept::check_concepts_and_equal_dimensions<Point, const Geometry>();

    Point c;
    centroid(geometry, c);
    return c;
}

/*!
    \brief Calculate and return centroid, using a specified strategy
    \ingroup centroid
    \param geometry the geometry to calculate centroid from
    \param strategy Calculation strategy for centroid
    \return the centroid
 */
template<typename Point, typename Geometry, typename Strategy>
inline Point make_centroid(Geometry const& geometry, Strategy const& strategy)
{
    //BOOST_CONCEPT_ASSERT( (geometry::concept::CentroidStrategy<Strategy>) );

    concept::check_concepts_and_equal_dimensions<Point, const Geometry>();

    Point c;
    centroid(geometry, c, strategy);
    return c;
}

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_CENTROID_HPP
