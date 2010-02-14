// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DISTANCE_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DISTANCE_HPP

#include <boost/mpl/if.hpp>
#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>
#include <boost/static_assert.hpp>

#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/core/is_multi.hpp>
#include <boost/geometry/core/reverse_dispatch.hpp>

#include <boost/geometry/geometries/segment.hpp>
#include <boost/geometry/geometries/concepts/check.hpp>

#include <boost/geometry/strategies/distance.hpp>
#include <boost/geometry/strategies/distance_result.hpp>
#include <boost/geometry/algorithms/within.hpp>

/*!
\defgroup distance distance: calculate distance between two geometries
The distance algorithm returns the distance between two geometries.
\par Coordinate systems and strategies:
With help of strategies the distance function returns the appropriate distance.
If the input is in cartesian coordinates, the Euclidian distance (Pythagoras) is calculated.
If the input is in spherical coordinates (either degree or radian), the distance over the sphere is returned.
If the input is in geographic coordinates, distance is calculated over the globe and returned in meters.

\par Distance result:
Depending on calculation type the distance result is either a structure, convertable
to a double, or a double value. In case of Pythagoras it makes sense to not draw the square root in the
strategy itself. Taking a square root is relative expensive and is not necessary when comparing distances.

\par Geometries:
Currently implemented, for both cartesian and spherical/geographic:
- POINT - POINT
- POINT - SEGMENT and v.v.
- POINT - LINESTRING and v.v.

Not yet implemented:
- POINT - RING etc, note that it will return a zero if the point is anywhere within the ring

\par Example:
Example showing distance calculation of two points, in xy and in latlong coordinates
\dontinclude doxygen_1.cpp
\skip example_distance_point_point
\line {
\until }
*/

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace distance {

template <typename P1, typename P2, typename Strategy>
struct point_to_point
{
    static inline typename Strategy::return_type apply(P1 const& p1,
                P2 const& p2, Strategy const& strategy)
    {
        return strategy.apply(p1, p2);
    }
};


template<typename Point, typename Segment, typename Strategy>
struct point_to_segment
{
    static inline typename Strategy::return_type apply(Point const& point,
                Segment const& segment, Strategy const& strategy)
    {
        typename strategy_distance_segment
            <
                typename cs_tag<Point>::type,
                typename cs_tag<Segment>::type,
                Point,
                typename point_type<Segment>::type
            >::type segment_strategy;

        // See remark below.
        // TODO: avoid .first, .second
        return segment_strategy.apply(point, segment.first, segment.second);
    }
};


template<typename Point, typename Range, typename PPStrategy, typename PSStrategy>
struct point_to_range
{
    typedef typename PPStrategy::return_type return_type;

    static inline return_type apply(Point const& point, Range const& range,
            PPStrategy const& pp_strategy, PSStrategy const& ps_strategy)
    {
        typedef segment<const typename point_type<Range>::type> segment_type;

        if (boost::size(range) == 0)
        {
            return return_type(0);
        }

        // line of one point: return point square_distance
        typedef typename boost::range_const_iterator<Range>::type iterator_type;
        iterator_type it = boost::begin(range);
        iterator_type prev = it++;
        if (it == boost::end(range))
        {
            return pp_strategy.apply(point, *boost::begin(range));
        }


        // start with first segment distance
        return_type d = ps_strategy.apply(point, *prev, *it);

        // check if other segments are closer
        prev = it++;
        while(it != boost::end(range))
        {
            return_type ds = ps_strategy.apply(point, *prev, *it);
            if (geometry::close_to_zero(ds))
            {
                return return_type(0);
            }
            else if (ds < d)
            {
                d = ds;
            }
            prev = it++;
        }

        return d;
    }
};


template<typename Point, typename Ring, typename PPStrategy, typename PSStrategy>
struct point_to_ring
{
    typedef std::pair<typename PPStrategy::return_type, bool> distance_containment;

    static inline distance_containment apply(Point const& point, 
                Ring const& ring,
                PPStrategy const& pp_strategy, PSStrategy const& ps_strategy)
    {
        return distance_containment
            (
                point_to_range
                    <
                        Point,
                        Ring,
                        PPStrategy,
                        PSStrategy
                    >::apply(point, ring, pp_strategy, ps_strategy), 
                geometry::within(point, ring)
            );
    }
};



template<typename Point, typename Polygon, typename PPStrategy, typename PSStrategy>
struct point_to_polygon
{
    typedef typename PPStrategy::return_type return_type;
    typedef std::pair<typename PPStrategy::return_type, bool> distance_containment;

    static inline distance_containment apply(Point const& point, 
                Polygon const& polygon,
                PPStrategy const& pp_strategy, PSStrategy const& ps_strategy)
    {
        // Check distance to all rings
        typedef point_to_ring
            <
                Point,
                typename ring_type<Polygon>::type,
                PPStrategy,
                PSStrategy
            > per_ring;

        distance_containment dc = per_ring::apply(point, 
                        exterior_ring(polygon), pp_strategy, ps_strategy);

        for (typename boost::range_const_iterator
                <
                    typename interior_type<Polygon>::type
                >::type it = boost::begin(interior_rings(polygon));
             it != boost::end(interior_rings(polygon));
             ++it)
        {
            distance_containment dcr = per_ring::apply(point, 
                            *it, pp_strategy, ps_strategy);
            if (dcr.first < dc.first)
            {
                dc.first = dcr.first;
            }
            // If it was inside, and also inside inner ring,
            // turn off the inside-flag, it is outside the polygon
            if (dc.second && dcr.second)
            {
                dc.second = false;
            }
        }
        return dc;
    }
};


}} // namespace detail::distance
#endif // DOXYGEN_NO_DETAIL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template
<
    typename GeometryTag1, typename GeometryTag2,
    typename G1, typename G2,
    typename StrategyTag, typename Strategy,
    bool IsMulti1, bool IsMulti2
>
struct distance
{};


template <typename P1, typename P2, typename Strategy>
struct distance
<
    point_tag, point_tag,
    P1, P2,
    strategy_tag_distance_point_point, Strategy,
    false, false
> : detail::distance::point_to_point<P1, P2, Strategy>
{};

/// Point-line version 1, where point-point strategy is specified
template <typename Point, typename Linestring, typename Strategy>
struct distance
<
    point_tag, linestring_tag,
    Point, Linestring,
    strategy_tag_distance_point_point, Strategy,
    false, false
>
{

    static inline typename Strategy::return_type apply(Point const& point,
            Linestring const& linestring,
            Strategy const& strategy)
    {
        //typedef segment<const > segment_type;
        typedef typename geometry::strategy_distance_segment
                    <
                            typename cs_tag<Point>::type,
                            typename cs_tag<Linestring>::type,
                            Point,
                            typename point_type<Linestring>::type
                    >::type ps_strategy_type;

        return detail::distance::point_to_range
            <
                Point, Linestring, Strategy, ps_strategy_type
            >::apply(point, linestring, strategy, ps_strategy_type());
    }
};


/// Point-line version 2, where point-segment strategy is specified
template <typename Point, typename Linestring, typename Strategy>
struct distance
<
    point_tag, linestring_tag,
    Point, Linestring,
    strategy_tag_distance_point_segment, Strategy,
    false, false
>
{
    static inline typename Strategy::return_type apply(Point const& point,
            Linestring const& linestring,
            Strategy const& strategy)
    {
        typedef typename Strategy::point_strategy_type pp_strategy_type;
        return detail::distance::point_to_range
            <
                Point, Linestring, pp_strategy_type, Strategy
            >::apply(point, linestring, pp_strategy_type(), strategy);
    }
};

/// Point-polygon , where point-segment strategy is specified
template <typename Point, typename Polygon, typename Strategy>
struct distance
<
    point_tag, polygon_tag,
    Point, Polygon,
    strategy_tag_distance_point_point, Strategy,
    false, false
>
{
    typedef typename Strategy::return_type return_type;

    static inline return_type apply(Point const& point,
            Polygon const& polygon,
            Strategy const& strategy)
    {
        typedef typename geometry::strategy_distance_segment
                    <
                            typename cs_tag<Point>::type,
                            typename cs_tag<Polygon>::type,
                            Point,
                            typename point_type<Polygon>::type
                    >::type ps_strategy_type;

        std::pair<return_type, bool>
            dc = detail::distance::point_to_polygon
            <
                Point, Polygon, Strategy, ps_strategy_type
            >::apply(point, polygon, strategy, ps_strategy_type());

        return dc.second ? return_type(0) : dc.first;
    }
};



// Point-segment version 1, with point-point strategy
template <typename Point, typename Segment, typename Strategy>
struct distance
<
    point_tag, segment_tag,
    Point, Segment,
    strategy_tag_distance_point_point, Strategy,
    false, false
> : detail::distance::point_to_segment<Point, Segment, Strategy>
{};

// Point-segment version 2, with point-segment strategy
template <typename Point, typename Segment, typename Strategy>
struct distance
<
    point_tag, segment_tag,
    Point, Segment,
    strategy_tag_distance_point_segment, Strategy,
    false, false
>
{
    static inline typename Strategy::return_type apply(Point const& point,
                Segment const& segment, Strategy const& strategy)
    {
        // TODO: We cannot use .first and .second here.
        // Segment strategy does not operate on segment (anymore), because:
        // all strategies do not operate on segments anymore, because
        // it turned out to be inconvenient (wrapping up things in segments);
        // The SIDE strategy must operate on three different point types,
        // and that might be for distance segment strategy as well
        // (though not very probable).

        return strategy.apply(point, segment.first, segment.second);
    }
};



// Strictly spoken this might be in namespace <impl> again
template
<
    typename GeometryTag1, typename GeometryTag2,
    typename G1, typename G2,
    typename StrategyTag, typename Strategy,
    bool IsMulti1, bool IsMulti2
>
struct distance_reversed
{
    static inline typename Strategy::return_type apply(G1 const& g1,
                G2 const& g2, Strategy const& strategy)
    {
        return distance
            <
                GeometryTag2, GeometryTag1,
                G2, G1,
                StrategyTag, Strategy,
                IsMulti2, IsMulti1
            >::apply(g2, g1, strategy);
    }
};

} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH

/*!
    \brief Calculate distance between two geometries with a specified strategy
    \ingroup distance
    \tparam Geometry1 first geometry type
    \tparam Geometry2 second geometry type
    \tparam S point-point-distance strategy type
    \param geometry1 first geometry
    \param geometry2 second geometry
    \param strategy strategy to calculate distance between two points
    \return the distance (either a double or a distance_result, (convertable to double))
    \note The strategy can be a point-point strategy. In case of distance point-line/point-polygon
        it may also be a point-segment strategy.
    \par Example:
    Example showing distance calculation of two lat long points, using the accurate Vincenty approximation
    \dontinclude doxygen_1.cpp
    \skip example_distance_point_point_strategy
    \line {
    \until }
 */
template <typename Geometry1, typename Geometry2, typename Strategy>
inline typename Strategy::return_type distance(Geometry1 const& geometry1,
            Geometry2 const& geometry2, Strategy const& strategy)
{
    typedef typename boost::remove_const<Geometry1>::type ncg1_type;
    typedef typename boost::remove_const<Geometry2>::type ncg2_type;

    return boost::mpl::if_
        <
            typename geometry::reverse_dispatch<Geometry1, Geometry2>::type,
            dispatch::distance_reversed
                <
                    typename tag<ncg1_type>::type,
                    typename tag<ncg2_type>::type,
                    ncg1_type,
                    ncg2_type,
                    typename strategy_tag<Strategy>::type,
                    Strategy,
                    is_multi<ncg1_type>::value,
                    is_multi<ncg2_type>::value
                >,
                dispatch::distance
                <
                    typename tag<ncg1_type>::type,
                    typename tag<ncg2_type>::type,
                    ncg1_type,
                    ncg2_type,
                    typename strategy_tag<Strategy>::type,
                    Strategy,
                    is_multi<ncg1_type>::value,
                    is_multi<ncg2_type>::value
                >
        >::type::apply(geometry1, geometry2, strategy);
}


/*!
    \brief Calculate distance between two geometries
    \ingroup distance
    \details The default strategy is used, belonging to the corresponding coordinate system of the geometries
    \tparam G1 first geometry type
    \tparam G2 second geometry type
    \param geometry1 first geometry
    \param geometry2 second geometry
    \return the distance (either a double or a distance result, convertable to double)
 */
template <typename Geometry1, typename Geometry2>
inline typename distance_result<Geometry1, Geometry2>::type distance(
                Geometry1 const& geometry1, Geometry2 const& geometry2)
{
    concept::check<const Geometry1>();
    concept::check<const Geometry2>();

    typedef typename point_type<Geometry1>::type point1_type;
    typedef typename point_type<Geometry2>::type point2_type;

    // Define a point-point-distance-strategy
    // for either the normal case, either the reversed case
    typedef typename boost::mpl::if_c
        <
            geometry::reverse_dispatch<Geometry1, Geometry2>::type::value,
            typename strategy_distance
                <
                    typename cs_tag<point2_type>::type,
                    typename cs_tag<point1_type>::type,
                    point2_type,
                    point1_type
                >::type,
            typename strategy_distance
                <
                    typename cs_tag<point1_type>::type,
                    typename cs_tag<point2_type>::type,
                    point1_type,
                    point2_type
                >::type
        >::type strategy;

    return distance(geometry1, geometry2, strategy());
}

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DISTANCE_HPP
