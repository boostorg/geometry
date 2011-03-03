// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2010, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_OVERLAY_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_OVERLAY_HPP


#include <deque>
#include <map>

#include <boost/range.hpp>
#include <boost/mpl/assert.hpp>


#include <boost/geometry/algorithms/detail/overlay/calculate_distance_policy.hpp>
#include <boost/geometry/algorithms/detail/overlay/enrich_intersection_points.hpp>
#include <boost/geometry/algorithms/detail/overlay/enrichment_info.hpp>
#include <boost/geometry/algorithms/detail/overlay/get_turns.hpp>
#include <boost/geometry/algorithms/detail/overlay/traverse.hpp>
#include <boost/geometry/algorithms/detail/overlay/traversal_info.hpp>
#include <boost/geometry/algorithms/detail/overlay/turn_info.hpp>


#include <boost/geometry/algorithms/num_points.hpp>
#include <boost/geometry/algorithms/reverse.hpp>

#include <boost/geometry/iterators/range_type.hpp>

#include <boost/geometry/algorithms/detail/overlay/add_rings.hpp>
#include <boost/geometry/algorithms/detail/overlay/assign_parents.hpp>
#include <boost/geometry/algorithms/detail/overlay/ring_properties.hpp>
#include <boost/geometry/algorithms/detail/overlay/select_rings.hpp>


#ifdef BOOST_GEOMETRY_DEBUG_ASSEMBLE
#  include <boost/geometry/util/write_dsv.hpp>
#endif


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace overlay
{


// Skip for assemble process
template <typename TurnInfo>
inline bool skip(TurnInfo const& turn_info)
{
    return (turn_info.discarded || turn_info.both(operation_union))
        && ! turn_info.any_blocked()
        && ! turn_info.both(operation_intersection)
        ;
}




template <typename TurnPoints, typename Map>
inline void map_turns(Map& map, TurnPoints const& turn_points)
{
    typedef typename boost::range_value<TurnPoints>::type turn_point_type;
    typedef typename turn_point_type::container_type container_type;

    int index = 0;
    for (typename boost::range_iterator<TurnPoints const>::type
            it = boost::begin(turn_points);
         it != boost::end(turn_points);
         ++it, ++index)
    {
        if (! skip(*it))
        {
            int op_index = 0;
            for (typename boost::range_iterator<container_type const>::type
                    op_it = boost::begin(it->operations);
                op_it != boost::end(it->operations);
                ++op_it, ++op_index)
            {
                ring_identifier ring_id
                    (
                        op_it->seg_id.source_index,
                        op_it->seg_id.multi_index,
                        op_it->seg_id.ring_index
                    );
                map[ring_id]++;
            }
        }
    }
}



template
<
    typename Geometry1, typename Geometry2,
    bool Reverse1, bool Reverse2, bool ReverseOut,
    typename OutputIterator, typename GeometryOut,
    overlay_type Direction,
    typename Strategy
>
struct overlay
{
    static inline OutputIterator apply(
                Geometry1 const& geometry1, Geometry2 const& geometry2,
                OutputIterator out,
                Strategy const& )
    {
        if (geometry::num_points(geometry1) == 0 && geometry::num_points(geometry2) == 0)
        {
            return out;
        }

        typedef typename geometry::point_type<GeometryOut>::type point_type;
        typedef detail::overlay::traversal_turn_info<point_type> turn_info;
        typedef std::deque<turn_info> container_type;

        // "Use" rangetype for ringtype:
        // for polygon, it is the type of the exterior ring.
        // for ring, it is the ring itself. That is what is
        // for multi-polygon, it is also the type of the ring.
        typedef typename geometry::range_type<GeometryOut>::type ring_type;
        typedef std::deque<ring_type> ring_container_type;

        // If one input is empty, output the other one for a union.
        // For an intersection, the intersection is empty.
        // TODO: for a difference, take one of them.
        if (geometry::num_points(geometry1) == 0
            || geometry::num_points(geometry2) == 0)
        {
            if (Direction == overlay_union)
            {
                std::map<ring_identifier, int> map;
                ring_container_type rings;
                ////return assemble<GeometryOut>(rings, map, geometry1, geometry2, Direction, false, false, out);
            }
            return out;
        }

        container_type turn_points;

#ifdef BOOST_GEOMETRY_DEBUG_ASSEMBLE
std::cout << "get turns" << std::endl;
#endif
        detail::get_turns::no_interrupt_policy policy;
        boost::geometry::get_turns
            <
                Reverse1, Reverse2,
                detail::overlay::calculate_distance_policy
            >(geometry1, geometry2, turn_points, policy);

#ifdef BOOST_GEOMETRY_DEBUG_ASSEMBLE
std::cout << "enrich" << std::endl;
#endif
        typename Strategy::side_strategy_type side_strategy;
        geometry::enrich_intersection_points<Reverse1, Reverse2>(turn_points,
                Direction == overlay_union
                    ? boost::geometry::detail::overlay::operation_union
                    : boost::geometry::detail::overlay::operation_intersection,
                    geometry1, geometry2,
                    side_strategy);

#ifdef BOOST_GEOMETRY_DEBUG_ASSEMBLE
std::cout << "traverse" << std::endl;
#endif
        ring_container_type rings;
        geometry::traverse<Reverse1, Reverse2>(geometry1, geometry2,
                Direction == overlay_union
                    ? boost::geometry::detail::overlay::operation_union
                    : boost::geometry::detail::overlay::operation_intersection,
                turn_points, rings);

        // TEMP condition, reversal should be done in traverse by calling "push_front"
        if (ReverseOut && (Reverse1 || Reverse2))
        {
            for (typename boost::range_iterator<ring_container_type>::type
                    it = boost::begin(rings);
                    it != boost::end(rings);
                    ++it)
            {
                geometry::reverse(*it);
            }
        }


        std::map<ring_identifier, int> map;
        map_turns(map, turn_points);
        //return assemble<GeometryOut>(rings, map, geometry1, geometry2, Direction, false, false, out);

        typedef ring_properties<typename geometry::point_type<Geometry1>::type> info;

        std::map<ring_identifier, info> selected;

        select_rings<Direction>(geometry1, geometry2, map, selected);

        // Add rings from container
        {
            ring_identifier id(2, 0, -1);
            for (typename boost::range_iterator<ring_container_type>::type
                    it = boost::begin(rings);
                    it != boost::end(rings);
                    ++it)
            {
                selected[id] = info(*it);
                id.multi_index++;
            }
        }

        assign_parents(geometry1, geometry2, rings, selected);
        return add_rings<GeometryOut>(selected, geometry1, geometry2, rings, out);

    }
};


// Metafunction helper for intersection and union
template <order_selector Selector, bool Reverse>
struct do_reverse {};

template <>
struct do_reverse<clockwise, false> : boost::false_type {};

template <>
struct do_reverse<clockwise, true> : boost::true_type {};

template <>
struct do_reverse<counterclockwise, false> : boost::true_type {};

template <>
struct do_reverse<counterclockwise, true> : boost::false_type {};



}} // namespace detail::overlay
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_OVERLAY_HPP
