// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_IS_VALID_POLYGON_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_IS_VALID_POLYGON_HPP

#include <cstddef>

#include <algorithm>
#include <deque>
#include <iterator>
#include <set>

#include <boost/range.hpp>

#include <boost/geometry/core/exterior_ring.hpp>
#include <boost/geometry/core/interior_rings.hpp>
#include <boost/geometry/core/ring_type.hpp>
#include <boost/geometry/core/tags.hpp>

#include <boost/geometry/util/range.hpp>

// MK:: the following include may not be needed
#include <boost/geometry/algorithms/covered_by.hpp>
#include <boost/geometry/algorithms/num_interior_rings.hpp>
#include <boost/geometry/algorithms/within.hpp>

#include <boost/geometry/algorithms/detail/check_iterator_range.hpp>

#include <boost/geometry/algorithms/detail/is_valid/complement_graph.hpp>
#include <boost/geometry/algorithms/detail/is_valid/has_valid_self_turns.hpp>
#include <boost/geometry/algorithms/detail/is_valid/is_acceptable_turn.hpp>
#include <boost/geometry/algorithms/detail/is_valid/ring.hpp>

#include <boost/geometry/algorithms/detail/is_valid/debug_print_turns.hpp>
#include <boost/geometry/algorithms/detail/is_valid/debug_validity_phase.hpp>
#include <boost/geometry/algorithms/detail/is_valid/debug_complement_graph.hpp>

#include <boost/geometry/algorithms/dispatch/is_valid.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace is_valid
{


template
<
    typename Polygon,
    bool AllowDuplicates,
    bool CheckRingValidityOnly = false
>
class is_valid_polygon
{
protected:
    typedef debug_validity_phase<Polygon> debug_phase;


    template <typename RingIterator, typename ExteriorRing>
    static inline bool are_holes_inside(RingIterator first,
                                        RingIterator beyond,
                                        ExteriorRing const& exterior_ring)
    {
        for (RingIterator it = first; it != beyond; ++it)
        {
            if ( !geometry::covered_by(range::front(*it), exterior_ring) )
            {
                return false;
            }
        }

        for (RingIterator it1 = first; it1 != beyond; ++it1)
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
        return true;
    }

    template <typename InteriorRings, typename ExteriorRing>
    static inline bool are_holes_inside(InteriorRings const& interior_rings,
                                        ExteriorRing const& exterior_ring)
    {
        return are_holes_inside(boost::begin(interior_rings),
                                boost::end(interior_rings),
                                exterior_ring);
    }

    struct has_holes_inside
    {    
        static inline bool apply(Polygon const& polygon)
        {
            return are_holes_inside(geometry::interior_rings(polygon),
                                    geometry::exterior_ring(polygon));
        }
    };



    template <typename InteriorRings>
    static bool has_valid_interior_rings(InteriorRings const& interior_rings)
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

    static inline bool has_valid_rings(Polygon const& polygon)
    {
        typedef typename ring_type<Polygon>::type ring_type;

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

        return has_valid_interior_rings(geometry::interior_rings(polygon));
    }



    template <typename TurnIterator>
    static inline bool has_connected_interior(Polygon const& polygon,
                                              TurnIterator first,
                                              TurnIterator beyond)
    {
        typedef typename std::iterator_traits
            <
                TurnIterator
            >::value_type turn_type;
        typedef complement_graph<typename turn_type::point_type> graph;

        graph g(geometry::num_interior_rings(polygon) + 1);
        for (TurnIterator tit = first; tit != beyond; ++tit)
        {
            typename graph::vertex_handle v1 = g.add_vertex
                ( tit->operations[0].seg_id.ring_index + 1 );
            typename graph::vertex_handle v2 = g.add_vertex
                ( tit->operations[0].other_id.ring_index + 1 );
            typename graph::vertex_handle vip = g.add_vertex(tit->point);

            g.add_edge(v1, vip);
            g.add_edge(v2, vip);
        }

        debug_print_complement_graph(std::cout, g);

        return !g.has_cycles();
    }

public:
    static inline bool apply(Polygon const& polygon)
    {
        if ( !has_valid_rings(polygon) )
        {
            return false;
        }

        if ( CheckRingValidityOnly )
        {
            return true;
        }

        // compute turns and check if all are acceptable
        debug_phase::apply(3);

        typedef has_valid_self_turns<Polygon> has_valid_turns;
        typedef typename has_valid_turns::turn_type turn_type;

        std::deque<turn_type> turns;
        bool has_invalid_turns = !has_valid_turns::apply(polygon, turns);
        debug_print_turns(turns.begin(), turns.end());

        if ( has_invalid_turns )
        {
            return false;
        }

        // check if all interior rings are inside the exterior ring
        debug_phase::apply(4);

        if ( !has_holes_inside::apply(polygon) )
        {
            return false;
        }

        // check whether the interior of the polygon is a connected set
        debug_phase::apply(5);

        return has_connected_interior(polygon, turns.begin(), turns.end());
    }
};


}} // namespace detail::is_valid
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


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_IS_VALID_POLYGON_HPP
