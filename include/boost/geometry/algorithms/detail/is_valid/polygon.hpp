// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_IS_VALID_POLYGON_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_IS_VALID_POLYGON_HPP

#include <algorithm>
#include <deque>
#include <set>

#include <boost/range.hpp>

#include <boost/geometry/core/exterior_ring.hpp>
#include <boost/geometry/core/interior_rings.hpp>
#include <boost/geometry/core/point_order.hpp>
#include <boost/geometry/core/point_type.hpp>
#include <boost/geometry/core/ring_type.hpp>
#include <boost/geometry/core/tags.hpp>

#include <boost/geometry/util/range.hpp>

#include <boost/geometry/policies/predicate_based_interrupt_policy.hpp>
#include <boost/geometry/policies/robustness/segment_ratio_type.hpp>
#include <boost/geometry/policies/robustness/get_rescale_policy.hpp>

#include <boost/geometry/algorithms/num_interior_rings.hpp>
#include <boost/geometry/algorithms/within.hpp>

#include <boost/geometry/algorithms/detail/check_iterator_range.hpp>

#include <boost/geometry/algorithms/detail/overlay/get_turn_info.hpp>
#include <boost/geometry/algorithms/detail/overlay/turn_info.hpp>
#include <boost/geometry/algorithms/detail/overlay/self_turn_points.hpp>

#include <boost/geometry/algorithms/detail/is_valid/ring.hpp>
#include <boost/geometry/algorithms/detail/is_valid/complement_graph.hpp>
#include <boost/geometry/algorithms/detail/is_valid/debug_print_turns.hpp>
#include <boost/geometry/algorithms/detail/is_valid/debug_validity_phase.hpp>
#include <boost/geometry/algorithms/detail/is_valid/debug_complement_graph.hpp>

#include <boost/geometry/algorithms/dispatch/is_valid.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace is_valid
{


template <order_selector Order /* counterclockwise */>
struct acceptable_operation
{
    static const detail::overlay::operation_type value =
        detail::overlay::operation_union;
};

template <>
struct acceptable_operation<clockwise>
{
    static const detail::overlay::operation_type value =
        detail::overlay::operation_intersection;
};



template <typename Polygon, bool AllowDuplicates>
class is_valid_polygon
{
private:
    template <typename RingIterator, typename ExteriorRing, typename IndexSet>
    static inline bool are_holes_inside(RingIterator first,
                                        RingIterator beyond,
                                        ExteriorRing const& exterior_ring,
                                        IndexSet const& rings_with_turns)
    {
        int idx = 0;
        for (RingIterator it = first; it != beyond; ++it, ++idx)
        {
            // check only rings whose index is not associated to any turn
            if ( rings_with_turns.find(idx) == rings_with_turns.end()
                 && !geometry::within(range::front(*it), exterior_ring) )
            {
                return false;
            }
        }

        // for those rings that do not have any associated turns,
        // check if they lie inside another ring
        idx = 0;
        for (RingIterator it1 = first; it1 != beyond; ++it1, ++idx)
        {
            if ( rings_with_turns.find(idx) == rings_with_turns.end() )
            {
                for (RingIterator it2 = first; it2 != beyond; ++it2)
                {
                    if ( it1 != it2
                         && geometry::within(range::front(*it1), *it2) )
                    {
                        return false;
                    }
                }
            }
        }
        return true;
    }

    template <typename InteriorRings, typename ExteriorRing, typename IndexSet>
    static inline bool are_holes_inside(InteriorRings const& interior_rings,
                                        ExteriorRing const& exterior_ring,
                                        IndexSet const& rings_with_turns)
    {
        return are_holes_inside(boost::begin(interior_rings),
                                boost::end(interior_rings),
                                exterior_ring,
                                rings_with_turns);
    }


    template <typename Turn, typename Method, typename Operation>
    static inline bool check_turn(Turn const& turn,
                                  Method method,
                                  Operation operation)
    {
        return turn.method == method
            && turn.operations[0].operation == operation
            && turn.operations[1].operation == operation;
    }

    struct is_acceptable_turn
    {
        template <typename Turn>
        static inline bool apply(Turn const& turn)
        {
            if ( turn.operations[0].seg_id.ring_index
                 == turn.operations[0].other_id.ring_index )
            {
                return false;
            }

            detail::overlay::operation_type const op = acceptable_operation
                <
                    geometry::point_order<Polygon>::value
                >::value;

            return check_turn(turn, detail::overlay::method_touch_interior, op)
                || check_turn(turn, detail::overlay::method_touch, op)
                ;
        }
    };


    template <typename InteriorRings>
    static bool are_valid_interior_rings(InteriorRings const& interior_rings)
    {
        return
            detail::check_iterator_range
                <
                    detail::is_valid::is_valid_ring
                        <
                            typename boost::range_value<InteriorRings>::type,
                            AllowDuplicates,
                            false, // do not check self-intersections
                            true // indicate that the ring is interior
                        >
                >::apply(boost::begin(interior_rings),
                         boost::end(interior_rings));
    }



public:
    static inline bool apply(Polygon const& polygon)
    {
        typedef typename point_type<Polygon>::type point_type;
        typedef typename ring_type<Polygon>::type ring_type;

        typedef debug_validity_phase<Polygon> debug_phase;

        // check validity of exterior ring
        debug_phase::apply(1);

        if ( !detail::is_valid::is_valid_ring
                 <
                     ring_type,
                     AllowDuplicates,
                     false // do not check self intersections
                 >::apply(exterior_ring(polygon)) )
        {
            return false;
        }

        // check validity of interior rings
        debug_phase::apply(2);

        if ( !are_valid_interior_rings(geometry::interior_rings(polygon)) )
        {
            return false;
        }


        // compute turns and check if all are acceptable
        debug_phase::apply(3);

        typedef typename geometry::rescale_policy_type
            <
                point_type
            >::type rescale_policy_type;

        typedef detail::overlay::turn_info
            <
                point_type,
                typename geometry::segment_ratio_type
                    <
                        point_type,
                        rescale_policy_type
                    >::type
            > turn_info;

        typedef detail::overlay::get_turn_info
            <
                detail::overlay::assign_null_policy
            > turn_policy;

        rescale_policy_type robust_policy
            = geometry::get_rescale_policy<rescale_policy_type>(polygon);

        detail::overlay::stateless_predicate_based_interrupt_policy
            <
                is_acceptable_turn
            > interrupt_policy;

        std::deque<turn_info> turns;
        geometry::self_turns<turn_policy>(polygon,
                                          robust_policy,
                                          turns,
                                          interrupt_policy);

        if ( interrupt_policy.has_intersections )
        {
            return false;
        }

        debug_print_turns(turns.begin(), turns.end());

        // put the ring id's that are associated with turns in a
        // container with fast lookup (std::set)
        std::set<int> rings_with_turns;
        for (typename std::deque<turn_info>::const_iterator tit = turns.begin();
             tit != turns.end(); ++tit)
        {
            rings_with_turns.insert(tit->operations[0].seg_id.ring_index);
            rings_with_turns.insert(tit->operations[0].other_id.ring_index);
        }


        // check if all interior rings are inside the exterior ring
        debug_phase::apply(4);

        if ( !are_holes_inside(geometry::interior_rings(polygon),
                               geometry::exterior_ring(polygon),
                               rings_with_turns) )
        {
            return false;
        }


        // check whether the interior of the polygon is a connected set
        debug_phase::apply(5);

        typedef graph_vertex<typename turn_info::point_type> graph_vertex;
        typedef complement_graph<graph_vertex> graph;

        graph g(geometry::num_interior_rings(polygon) + 1);
        for (typename std::deque<turn_info>::const_iterator tit = turns.begin();
             tit != turns.end(); ++tit)
        {
            typename graph::vertex_handle v1
                = g.add_vertex(tit->operations[0].seg_id.ring_index);
            typename graph::vertex_handle v2
                = g.add_vertex(tit->operations[0].other_id.ring_index);
            typename graph::vertex_handle vip = g.add_vertex(tit->point);

            g.add_edge(v1, vip);
            g.add_edge(v2, vip);
        }

        debug_print_complement_graph(std::cout, g);

        return !g.has_cycles();
    }
};


}} // namespace dispatch
#endif // DOXYGEN_NO_DETAIL



#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


// A Polygon is always a simple geometric object provided that it is valid.
//
// Reference (for validity of Polygons): OGC 06-103r4 (§6.1.11.1)
template <typename Polygon, bool AllowSpikes, bool AllowDuplicates>
struct is_valid<Polygon, polygon_tag, AllowSpikes, AllowDuplicates>
    : detail::is_valid::is_valid_polygon<Polygon, AllowDuplicates>
{};


// Not clear what the definition is.
// Right now we check that each element is simple (in fact valid), and
// that the MultiPolygon is also valid.
//
// Reference (for validity of MultiPolygons): OGC 06-103r4 (§6.1.14)
template <typename MultiPolygon, bool AllowSpikes, bool AllowDuplicates>
struct is_valid<MultiPolygon, multi_polygon_tag, AllowSpikes, AllowDuplicates>
{
    static inline bool apply(MultiPolygon const& multipolygon)
    {
        if ( !detail::check_iterator_range
                  <
                      detail::is_valid::is_valid_polygon
                          <
                              typename boost::range_value<MultiPolygon>::type,
                              AllowDuplicates
                          >,
                      false // do not allow empty multi-polygons
                  >::apply(boost::begin(multipolygon),
                           boost::end(multipolygon)) )
        {
            return false;
        }

        // MK::need to check that they are (almost) disjoint
        return true;
    }

};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_IS_VALID_POLYGON_HPP
