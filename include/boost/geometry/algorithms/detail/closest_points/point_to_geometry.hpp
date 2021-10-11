// Boost.Geometry

// Copyright (c) 2021, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_CLOSEST_POINTS_POINT_TO_GEOMETRY_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_CLOSEST_POINTS_POINT_TO_GEOMETRY_HPP

#include <iterator>
#include <type_traits>

#include <boost/core/ignore_unused.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/range/size.hpp>
#include <boost/range/value_type.hpp>

#include <boost/geometry/algorithms/assign.hpp>
#include <boost/geometry/algorithms/detail/closest_feature/geometry_to_range.hpp>
#include <boost/geometry/algorithms/detail/closest_feature/point_to_range.hpp>
#include <boost/geometry/algorithms/detail/distance/is_comparable.hpp>
#include <boost/geometry/algorithms/detail/distance/iterator_selector.hpp>
#include <boost/geometry/algorithms/detail/distance/strategy_utils.hpp>
#include <boost/geometry/algorithms/detail/within/point_in_geometry.hpp>
#include <boost/geometry/algorithms/dispatch/distance.hpp>

#include <boost/geometry/core/closure.hpp>
#include <boost/geometry/core/point_type.hpp>
#include <boost/geometry/core/exterior_ring.hpp>
#include <boost/geometry/core/interior_rings.hpp>
#include <boost/geometry/core/radian_access.hpp>
#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tags.hpp>

#include <boost/geometry/strategies/distance.hpp>
#include <boost/geometry/strategies/relate/services.hpp>
#include <boost/geometry/strategies/tags.hpp>

#include <boost/geometry/util/math.hpp>


namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace closest_points
{

template
<
    typename P1, typename P2, typename Strategies,
    bool IsUmbrella = strategies::detail::is_umbrella_strategy<Strategies>::value
>
struct point_to_point
{
    template <typename Segment>
    static inline void apply(P1 const& p1, P2 const& p2,  
                             Segment& shortest_seg, Strategies const& strategies)
    {
        set_segment_from_points::apply(p1, p2, shortest_seg);
    }
};

template
<
    typename Point, typename Segment, typename Strategies,
    bool IsUmbrella = strategies::detail::is_umbrella_strategy<Strategies>::value
>
struct point_to_segment
{
    template <typename OutputSegment>
    static inline void apply(Point const& point, Segment const& segment,
                             OutputSegment& shortest_seg, Strategies const& strategies)
    {
        typename point_type<Segment>::type p[2];
        geometry::detail::assign_point_from_index<0>(segment, p[0]);
        geometry::detail::assign_point_from_index<1>(segment, p[1]);

        boost::ignore_unused(strategies);

        auto closest_point = strategies.closest_points(point, segment)
            .apply(point, p[0], p[1]);

        set_segment_from_points::apply(point, closest_point, shortest_seg);    
    }
};

/*
template
<
    typename Point, typename Box, typename Strategies,
    bool IsUmbrella = strategies::detail::is_umbrella_strategy<Strategies>::value
>
struct point_to_box
{
    static inline auto apply(Point const& point, Box const& box,
                             Strategies const& strategies)
    {
        boost::ignore_unused(strategies);
        return strategies.closest_points(point, box).apply(point, box);
    }
};
*/

template
<
    typename Point,
    typename Range,
    closure_selector Closure,
    typename Strategies
>
class point_to_range
{
private:

    using point_to_point_range = detail::closest_feature::point_to_point_range
        <
            Point, Range, Closure
        >;

public:

    template <typename Segment>
    static inline void apply(Point const& point, Range const& range,
                             Segment& shortest_seg,
                             Strategies const& strategies)
    {
        if (boost::size(range) == 0)
        {
            set_segment_from_points::apply(point, point, shortest_seg);
            return;
        }

        distance::creturn_t<Point, Range, Strategies> cd_min;

        auto it_pair = point_to_point_range::apply(point, 
                                                   boost::begin(range),
                                                   boost::end(range), 
                                                   strategies.comparable_distance(point, range), 
                                                   cd_min);
        
        auto closest_point = strategies.closest_points(point, range)
            .apply(point, *it_pair.first, *it_pair.second);

        set_segment_from_points::apply(point, closest_point, shortest_seg);        
    }
};


template
<
    typename Point,
    typename Ring,
    closure_selector Closure,
    typename Strategies
>
struct point_to_ring
{
    template <typename Segment>
    static inline auto apply(Point const& point,
                             Ring const& ring,
                             Segment& shortest_seg,
                             Strategies const& strategies)
    {
        if (within::within_point_geometry(point, ring, strategies))
        {
            set_segment_from_points::apply(point, point, shortest_seg); 
        }
        else
        {
            point_to_range
            <
                Point, Ring, closure<Ring>::value, Strategies
            >::apply(point, ring, shortest_seg, strategies);
        }
            
    }
};


template
<
    typename Point,
    typename Polygon,
    closure_selector Closure,
    typename Strategies
>
class point_to_polygon
{
    using per_ring = point_to_range
        <
            Point, typename ring_type<Polygon>::type, Closure, Strategies
        >;

    struct distance_to_interior_rings
    {
        template <typename InteriorRingIterator, typename Segment>
        static inline void apply(Point const& point,
                                 InteriorRingIterator first,
                                 InteriorRingIterator last,
                                 Segment& shortest_seg,
                                 Strategies const& strategies)
        {
            for (InteriorRingIterator it = first; it != last; ++it)
            {
                if (within::within_point_geometry(point, *it, strategies))
                {
                    // the point is inside a polygon hole, so its distance
                    // to the polygon is its distance to the polygon's
                    // hole boundary
                    per_ring::apply(point, *it, shortest_seg, strategies);
                    return;
                }
            }
            set_segment_from_points::apply(point, point, shortest_seg); 
        }

        template <typename InteriorRings, typename Segment>
        static inline void apply(Point const& point, InteriorRings const& interior_rings,
                                 Segment& shortest_seg, Strategies const& strategies)
        {
            apply(point,
                  boost::begin(interior_rings),
                  boost::end(interior_rings),
                  shortest_seg,
                  strategies);
        }
    };


public:
    template <typename Segment>
    static inline void apply(Point const& point,
                             Polygon const& polygon,
                             Segment& shortest_seg,
                             Strategies const& strategies)
    {
        if (! within::covered_by_point_geometry(point, exterior_ring(polygon),
                                                strategies))
        {
            // the point is outside the exterior ring, so its distance
            // to the polygon is its distance to the polygon's exterior ring
            per_ring::apply(point, exterior_ring(polygon), shortest_seg, strategies);
            return;
        }

        // Check interior rings
        distance_to_interior_rings::apply(point,
                                          interior_rings(polygon),
                                          shortest_seg,
                                          strategies);
    }
};


template
<
    typename Point,
    typename MultiGeometry,
    typename Strategies,
    bool CheckCoveredBy = std::is_same
        <
            typename tag<MultiGeometry>::type, multi_polygon_tag
        >::value
>
class point_to_multigeometry
{
private:
    using geometry_to_range = detail::closest_feature::geometry_to_range;

public:

    template <typename Segment>
    static inline void apply(Point const& point,
                             MultiGeometry const& multigeometry,
                             Segment& shortest_seg,
                             Strategies const& strategies)
    {
        using selector_type = distance::iterator_selector<MultiGeometry const>;

        distance::creturn_t<Point, MultiGeometry, Strategies> cd;

        typename selector_type::iterator_type it_min
            = geometry_to_range::apply(point,
                                       selector_type::begin(multigeometry),
                                       selector_type::end(multigeometry),
                                       strategies.comparable_distance(point, multigeometry),
                                       cd);
        
        dispatch::closest_points
            <
                Point,
                typename std::iterator_traits
                    <
                        typename selector_type::iterator_type
                    >::value_type,
                Strategies
            >::apply(point, *it_min, shortest_seg, strategies);
    }
};


// this is called only for multipolygons, hence the change in the
// template parameter name MultiGeometry to MultiPolygon
template <typename Point, typename MultiPolygon, typename Strategies>
struct point_to_multigeometry<Point, MultiPolygon, Strategies, true>
{
    template <typename Segment>
    static inline void apply(Point const& point,
                             MultiPolygon const& multipolygon,
                             Segment& shortest_seg,
                             Strategies const& strategies)
    {
        if (within::covered_by_point_geometry(point, multipolygon, strategies))
        {
            set_segment_from_points::apply(point, point, shortest_seg); 
            return;
        }

        return point_to_multigeometry
            <
                Point, MultiPolygon, Strategies, false
            >::apply(point, multipolygon, shortest_seg, strategies);
    }
};


}} // namespace detail::closest_points
#endif // DOXYGEN_NO_DETAIL




#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


template <typename P1, typename P2, typename Strategy>
struct closest_points
    <
        P1, P2, Strategy, point_tag, point_tag,
        strategy_tag_distance_point_point, false
    > : detail::closest_points::point_to_point<P1, P2, Strategy>
{};


template <typename Point, typename Linestring, typename Strategy>
struct closest_points
    <
        Point, Linestring, Strategy, point_tag, linestring_tag,
        strategy_tag_distance_point_segment, false
    > : detail::closest_points::point_to_range<Point, Linestring, closed, Strategy>
{};


template <typename Point, typename Ring, typename Strategy>
struct closest_points
    <
        Point, Ring, Strategy, point_tag, ring_tag,
        strategy_tag_distance_point_segment, false
    > : detail::closest_points::point_to_ring
        <
            Point, Ring, closure<Ring>::value, Strategy
        >
{};


template <typename Point, typename Polygon, typename Strategy>
struct closest_points
    <
        Point, Polygon, Strategy, point_tag, polygon_tag,
        strategy_tag_distance_point_segment, false
    > : detail::closest_points::point_to_polygon
        <
            Point, Polygon, closure<Polygon>::value, Strategy
        >
{};


template <typename Point, typename Segment, typename Strategy>
struct closest_points
    <
        Point, Segment, Strategy, point_tag, segment_tag,
        strategy_tag_distance_point_segment, false
    > : detail::closest_points::point_to_segment<Point, Segment, Strategy>
{};

/*
template <typename Point, typename Box, typename Strategy>
struct closest_points
    <
         Point, Box, Strategy, point_tag, box_tag,
         strategy_tag_distance_point_box, false
    > : detail::closest_points::point_to_box<Point, Box, Strategy>
{};
*/

template<typename Point, typename MultiPoint, typename Strategy>
struct closest_points
    <
        Point, MultiPoint, Strategy, point_tag, multi_point_tag,
        strategy_tag_distance_point_point, false
    > : detail::closest_points::point_to_multigeometry
        <
            Point, MultiPoint, Strategy
        >
{};


template<typename Point, typename MultiLinestring, typename Strategy>
struct closest_points
    <
        Point, MultiLinestring, Strategy, point_tag, multi_linestring_tag,
        strategy_tag_distance_point_segment, false
    > : detail::closest_points::point_to_multigeometry
        <
            Point, MultiLinestring, Strategy
        >
{};


template<typename Point, typename MultiPolygon, typename Strategy>
struct closest_points
    <
        Point, MultiPolygon, Strategy, point_tag, multi_polygon_tag,
        strategy_tag_distance_point_segment, false
    > : detail::closest_points::point_to_multigeometry
        <
            Point, MultiPolygon, Strategy
        >
{};


template <typename Point, typename Linear, typename Strategy>
struct closest_points
    <
         Point, Linear, Strategy, point_tag, linear_tag,
         strategy_tag_distance_point_segment, false
    > : closest_points
        <
            Point, Linear, Strategy,
            point_tag, typename tag<Linear>::type,
            strategy_tag_distance_point_segment, false
        >
{};


template <typename Point, typename Areal, typename Strategy>
struct closest_points
    <
         Point, Areal, Strategy, point_tag, areal_tag,
         strategy_tag_distance_point_segment, false
    > : closest_points
        <
            Point, Areal, Strategy,
            point_tag, typename tag<Areal>::type,
            strategy_tag_distance_point_segment, false
        >
{};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_CLOSEST_POINTS_POINT_TO_GEOMETRY_HPP
