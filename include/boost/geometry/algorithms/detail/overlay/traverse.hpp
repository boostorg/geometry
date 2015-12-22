// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_TRAVERSE_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_TRAVERSE_HPP

#include <cstddef>

#include <boost/range.hpp>

#include <boost/geometry/algorithms/detail/overlay/backtrack_check_si.hpp>
#include <boost/geometry/algorithms/detail/overlay/copy_segments.hpp>
#include <boost/geometry/algorithms/detail/overlay/turn_info.hpp>
#include <boost/geometry/algorithms/num_points.hpp>
#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/assert.hpp>
#include <boost/geometry/core/closure.hpp>
#include <boost/geometry/core/coordinate_dimension.hpp>
#include <boost/geometry/geometries/concepts/check.hpp>

#if defined(BOOST_GEOMETRY_DEBUG_INTERSECTION) \
    || defined(BOOST_GEOMETRY_OVERLAY_REPORT_WKT) \
    || defined(BOOST_GEOMETRY_DEBUG_TRAVERSE)
#  include <string>
#  include <boost/geometry/algorithms/detail/overlay/debug_turn_info.hpp>
#  include <boost/geometry/io/wkt/wkt.hpp>
#endif

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace overlay
{

template <typename Turn, typename Operation>
#ifdef BOOST_GEOMETRY_DEBUG_TRAVERSE
inline void debug_traverse(Turn const& turn, Operation op,
                std::string const& header)
{
    std::cout << header
        << " at " << op.seg_id
        << " meth: " << method_char(turn.method)
        << " op: " << operation_char(op.operation)
        << " vis: " << visited_char(op.visited)
        << " of:  " << operation_char(turn.operations[0].operation)
        << operation_char(turn.operations[1].operation)
        << " " << geometry::wkt(turn.point)
        << std::endl;

    if (boost::contains(header, "Finished"))
    {
        std::cout << std::endl;
    }
}
#else
inline void debug_traverse(Turn const& , Operation, const char*)
{
}
#endif


template <typename Info, typename Turn>
inline void set_visited_for_continue(Info& info, Turn const& turn)
{
    // On "continue", set "visited" for ALL directions
    if (turn.operation == detail::overlay::operation_continue)
    {
        for (typename boost::range_iterator
            <
                typename Info::container_type
            >::type it = boost::begin(info.operations);
            it != boost::end(info.operations);
            ++it)
        {
            if (it->visited.none())
            {
                it->visited.set_visited();
            }
        }
    }
}

//! Metafunction to define side_order (clockwise, ccw) by operation_type
template <operation_type OpType>
struct side_compare {};

template <>
struct side_compare<operation_union>
{
    typedef std::greater<int> type;
};

template <>
struct side_compare<operation_intersection>
{
    typedef std::less<int> type;
};


template
<
    bool Reverse1,
    bool Reverse2,
    typename SideCompare
>
struct traversal
{

    template <typename Turns, typename SortBySide>
    static inline signed_size_type traverse_cluster(Turns& turns,
        SortBySide const& sbs, std::size_t index)
    {
        typedef typename boost::range_value<Turns>::type turn_type;
        typename SortBySide::rp const& ranked_point = sbs.m_ranked_points[index];
        std::size_t result = ranked_point.turn_index;


        turn_type& ranked_turn = turns[ranked_point.turn_index];
        typename turn_type::turn_operation_type& ranked_op
                = ranked_turn.operations[ranked_point.op_index];

        signed_size_type next_turn_index = ranked_op.enriched.next_ip_index;
        if (next_turn_index == -1)
        {
            next_turn_index = ranked_op.enriched.travels_to_ip_index;
        }
        if (next_turn_index == -1)
        {
            // Turn found but dead end, do not use this turn. Can happen if,
            // for example, in intersection ux is not filtered out
            return -1;
        }

        for (std::size_t i = index + 1;
             i < sbs.m_ranked_points.size();
             i++)
        {
            const typename SortBySide::rp& next = sbs.m_ranked_points[i];
            if (next.main_rank != ranked_point.main_rank)
            {
                return result;
            }
            if (next.turn_index == next_turn_index)
            {
                ranked_op.visited.set_visited();
                result = next.turn_index;

                // If there are more consecutively in same cluster, move
                // to next one
                return traverse_cluster(turns, sbs, i);
            }
        }
        return result;
    }


    template
    <
        typename Geometry1,
        typename Geometry2,
        typename Clusters,
        typename Turns,
        typename Operation
    >
    static inline void select_turn_from_cluster(Geometry1 const& geometry1, Geometry2 const& geometry2,
                Clusters const& clusters, Turns& turns,
                typename boost::range_iterator<Turns>::type& turn_it,
                Operation const& op)
    {
        typedef typename boost::range_value<Turns>::type turn_type;
        turn_type const& turn = *turn_it;
        if (turn.cluster_id < 0)
        {
            return;
        }
        typename Clusters::const_iterator mit = clusters.find(turn.cluster_id);
        if (mit == clusters.end())
        {
            return;
        }

        std::set<signed_size_type> const& ids = mit->second;

        typedef typename geometry::point_type<Geometry1>::type point_type;
        typedef sort_by_side::side_sorter<Reverse1, Reverse2, point_type, SideCompare> sbs_type;
        sbs_type sbs;

        for (typename std::set<signed_size_type>::const_iterator sit = ids.begin();
             sit != ids.end(); ++sit)
        {
            signed_size_type turn_index = *sit;
            turn_type const& cturn = turns[turn_index];
            for (int i = 0; i < 2; i++)
            {
                Operation const& cop = cturn.operations[i];

                // TODO: source_index is NOT a good criterium
                bool is_subject = &cturn == &turn
                               && op.seg_id.source_index == cop.seg_id.source_index;

                sbs.add(cop, turn_index, i, geometry1, geometry2, is_subject);
            }
        }
        sbs.apply(turn.point);

        for (std::size_t i = 0; i < sbs.m_ranked_points.size(); i++)
        {
            const typename sbs_type::rp& ranked_point = sbs.m_ranked_points[i];
            turn_type& ranked_turn = turns[ranked_point.turn_index];

            if (ranked_point.main_rank == 0 && ranked_point.index != sort_by_side::index_from)
            {
                // There are outgoing arcs, quit
                return;
            }

            if (ranked_point.main_rank == 1
                    && ranked_point.index == sort_by_side::index_to)
            {
                if (ranked_turn.discarded)
                {
                    // Might be collocated u/u turn
                    return;
                }

                // Use this turn, or, in a cluster, traverse through it
                signed_size_type turn_index = traverse_cluster(turns, sbs, i);
                if (turn_index != -1)
                {
                    turn_it = turns.begin() + turn_index;
                    return;
                }
            }
            if (ranked_point.main_rank >= 1)
            {
                // Nothing found, don't change
                return;
            }
        }
    }

    template
    <
        typename GeometryOut,
        typename G1,
        typename G2,
        typename Clusters,
        typename Turns,
        typename IntersectionInfo,
        typename RobustPolicy
    >
    static inline bool assign_next_ip(G1 const& g1, G2 const& g2,
                Clusters const& clusters, Turns& turns,
                typename boost::range_iterator<Turns>::type& ip,
                GeometryOut& current_output,
                IntersectionInfo& info,
                segment_identifier& seg_id,
                RobustPolicy const& robust_policy)
    {
        info.visited.set_visited();
        set_visited_for_continue(*ip, info);

        // If there is no next IP on this segment
        if (info.enriched.next_ip_index < 0)
        {
            if (info.enriched.travels_to_vertex_index < 0
                || info.enriched.travels_to_ip_index < 0)
            {
                return false;
            }

            BOOST_GEOMETRY_ASSERT(info.enriched.travels_to_vertex_index >= 0);
            BOOST_GEOMETRY_ASSERT(info.enriched.travels_to_ip_index >= 0);

            if (info.seg_id.source_index == 0)
            {
                geometry::copy_segments<Reverse1>(g1, info.seg_id,
                        info.enriched.travels_to_vertex_index,
                        robust_policy,
                        current_output);
            }
            else
            {
                geometry::copy_segments<Reverse2>(g2, info.seg_id,
                        info.enriched.travels_to_vertex_index,
                        robust_policy,
                        current_output);
            }
            seg_id = info.seg_id;
            ip = boost::begin(turns) + info.enriched.travels_to_ip_index;
        }
        else
        {
            ip = boost::begin(turns) + info.enriched.next_ip_index;
            seg_id = info.seg_id;
        }

        detail::overlay::append_no_dups_or_spikes(current_output, ip->point,
            robust_policy);

        if (ip->cluster_id >= 0)
        {
            select_turn_from_cluster(g1, g2, clusters, turns, ip, info);
        }

        return true;
    }

};


inline bool select_source(operation_type operation,
                          signed_size_type source1,
                          signed_size_type source2)
{
    return (operation == operation_intersection && source1 != source2)
        || (operation == operation_union && source1 == source2)
        ;
}


template
<
    typename Turn,
    typename Iterator
>
inline bool select_next_ip(operation_type operation,
            Turn& turn,
            std::size_t start_turn_index,
            segment_identifier const& seg_id,
            Iterator& selected)
{
    if (turn.discarded)
    {
        return false;
    }

    bool has_tp = false;

    typedef typename std::iterator_traits
    <
        Iterator
    >::value_type operation_type;

    typename operation_type::comparable_distance_type
            max_remaining_distance = 0;

    selected = boost::end(turn.operations);
    for (Iterator it = boost::begin(turn.operations);
        it != boost::end(turn.operations);
        ++it)
    {
        if (it->visited.started())
        {
            selected = it;
            //std::cout << " RETURN";
            return true;
        }

        // In some cases there are two alternatives.
        // For "ii", take the other one (alternate)
        //           UNLESS the other one is already visited
        // For "uu", take the same one (see above);
        // For "cc", take either one, but if there is a starting one,
        //           take that one.
        if (   (it->operation == operation_continue
                && (! has_tp || it->visited.started()
                    )
                )
            || (it->operation == operation
                && ! it->visited.finished()
                && (! has_tp
                    || select_source(operation,
                            it->seg_id.source_index, seg_id.source_index)
                    )
                )
            )
        {
            if (it->operation == operation_continue)
            {
                max_remaining_distance = it->remaining_distance;
            }
            selected = it;
            debug_traverse(turn, *it, " Candidate");
            has_tp = true;
        }

        if (it->operation == operation_continue && has_tp)
        {
            if (it->enriched.next_ip_index == start_turn_index
                || it->enriched.travels_to_ip_index == start_turn_index)
            {
                selected = it;
                debug_traverse(turn, *it, " Candidate override (start)");
            }
            else if (it->remaining_distance > max_remaining_distance)
            {
                max_remaining_distance = it->remaining_distance;
                selected = it;
                debug_traverse(turn, *it, " Candidate override (remaining)");
            }
        }
    }

    if (has_tp)
    {
       debug_traverse(turn, *selected, "  Accepted");
    }


    return has_tp;
}


/*!
    \brief Traverses through intersection points / geometries
    \ingroup overlay
 */
template
<
    bool Reverse1, bool Reverse2,
    typename Geometry1,
    typename Geometry2,
    operation_type OpType,
    typename Backtrack = backtrack_check_self_intersections<Geometry1, Geometry2>
>
class traverse
{
public :
    template
    <
        typename RobustPolicy,
        typename Turns,
        typename Rings,
        typename Visitor,
        typename Clusters
    >
    static inline void apply(Geometry1 const& geometry1,
                Geometry2 const& geometry2,
                RobustPolicy const& robust_policy,
                Turns& turns, Rings& rings,
                Clusters const& clusters,
                Visitor& visitor)
    {
        typedef typename boost::range_value<Rings>::type ring_type;
        typedef typename boost::range_iterator<Turns>::type turn_iterator;
        typedef typename boost::range_value<Turns>::type turn_type;
        typedef typename boost::range_iterator
            <
                typename turn_type::container_type
            >::type turn_operation_iterator_type;
        typedef typename turn_type::turn_operation_type op_type;

        typedef traversal
            <
                Reverse1,
                Reverse2,
                typename side_compare<OpType>::type
            > trav;

        std::size_t const min_num_points
                = core_detail::closure::minimum_ring_size
                        <
                            geometry::closure<ring_type>::value
                        >::value;

        std::size_t size_at_start = boost::size(rings);

        typename Backtrack::state_type state;
        do
        {
            state.reset();

            std::size_t start_turn_index = 0;

            // Iterate through all unvisited points
            for (turn_iterator it = boost::begin(turns);
                state.good() && it != boost::end(turns);
                ++it, ++start_turn_index)
            {
                turn_type& the_turn = *it;
                // Skip discarded ones
                if (! (the_turn.discarded || ! the_turn.selectable_start || the_turn.blocked()))
                {
                    for (turn_operation_iterator_type iit = boost::begin(the_turn.operations);
                        state.good() && iit != boost::end(the_turn.operations);
                        ++iit)
                    {
                        op_type& the_op = *iit;

                        if (the_op.visited.none()
                            && ! the_op.visited.rejected()
                            && (the_op.operation == OpType
                                || the_op.operation == detail::overlay::operation_continue)
                            )
                        {
                            set_visited_for_continue(the_turn, the_op);

                            ring_type current_output;
                            detail::overlay::append_no_dups_or_spikes(current_output,
                                the_turn.point, robust_policy);

                            // Copy iterators, can be reassigned below
                            turn_iterator current_it = it;
                            turn_operation_iterator_type current_iit = iit;
                            segment_identifier current_seg_id;

                            if (! trav::assign_next_ip(
                                        geometry1, geometry2,
                                        clusters, turns,
                                        current_it, current_output,
                                        the_op, current_seg_id,
                                        robust_policy))
                            {
                                Backtrack::apply(
                                    size_at_start,
                                    rings, current_output, turns, *current_it, *current_iit,
                                    "No next IP",
                                    geometry1, geometry2, robust_policy, state, visitor);
                            }

                            if (current_it == it)
                            {
                                the_op.visited.set_finished();
                                detail::overlay::debug_traverse(*current_it, the_op, "->Finished early");
                                visitor.visit_traverse(turns, *current_it, the_op, "E");
                                if (geometry::num_points(current_output) >= min_num_points)
                                {
                                    clean_closing_dups_and_spikes(current_output, robust_policy);
                                    rings.push_back(current_output);
                                    continue;
                                }
                            }

                            if (! detail::overlay::select_next_ip(
                                            OpType,
                                            *current_it,
                                            start_turn_index,
                                            current_seg_id,
                                            current_iit))
                            {
                                Backtrack::apply(
                                    size_at_start,
                                    rings, current_output, turns, the_turn, the_op,
                                    "Dead end at start",
                                    geometry1, geometry2, robust_policy, state, visitor);
                            }
                            else
                            {

                                the_op.visited.set_started();
                                detail::overlay::debug_traverse(the_turn, the_op, "-> Started");
                                detail::overlay::debug_traverse(*current_it, *current_iit, "Selected  ");
                                visitor.visit_traverse(turns, the_turn, the_op, "B");
                                visitor.visit_traverse(turns, *current_it, *current_iit, "V");

                                typename boost::range_size<Turns>::type i = 0;

                                while (current_iit != iit && state.good())
                                {
                                    if (current_iit->visited.visited())
                                    {
                                        // It visits a visited node again, without passing the start node.
                                        // This makes it suspicious for endless loops
                                        Backtrack::apply(
                                            size_at_start,
                                            rings,  current_output, turns, the_turn, the_op,
                                            "Visit again",
                                            geometry1, geometry2, robust_policy, state, visitor);
                                    }
                                    else
                                    {


                                        // We assume clockwise polygons only, non self-intersecting, closed.
                                        // However, the input might be different, and checking validity
                                        // is up to the library user.

                                        // Therefore we make here some sanity checks. If the input
                                        // violates the assumptions, the output polygon will not be correct
                                        // but the routine will stop and output the current polygon, and
                                        // will continue with the next one.

                                        // Below three reasons to stop.
                                        trav::assign_next_ip(
                                            geometry1, geometry2,
                                            clusters, turns, current_it, current_output,
                                            *current_iit, current_seg_id,
                                            robust_policy);

                                        if (! detail::overlay::select_next_ip(
                                                    OpType,
                                                    *current_it,
                                                    start_turn_index,
                                                    current_seg_id,
                                                    current_iit))
                                        {
                                            // Should not occur in valid (non-self-intersecting) polygons
                                            // Should not occur in self-intersecting polygons without spikes
                                            // Might occur in polygons with spikes
                                            Backtrack::apply(
                                                size_at_start,
                                                rings,  current_output, turns, the_turn, the_op,
                                                "Dead end",
                                                geometry1, geometry2, robust_policy, state, visitor);
                                        }
                                        else
                                        {
                                            detail::overlay::debug_traverse(*current_it, *current_iit, "Selected  ");
                                            visitor.visit_traverse(turns, *current_it, *current_iit, "V");
                                        }

                                        if (i++ > 2 + 2 * turns.size())
                                        {
                                            // Sanity check: there may be never more loops
                                            // than turn points.
                                            // Turn points marked as "ii" can be visited twice.
                                            Backtrack::apply(
                                                size_at_start,
                                                rings,  current_output, turns, the_turn, the_op,
                                                "Endless loop",
                                                geometry1, geometry2, robust_policy, state, visitor);
                                        }
                                    }
                                }

                                if (state.good())
                                {
                                    the_op.visited.set_finished();
                                    detail::overlay::debug_traverse(*current_it, the_op, "->Finished");
                                    visitor.visit_traverse(turns, *current_it, the_op, "F");
                                    if (geometry::num_points(current_output) >= min_num_points)
                                    {
                                        clean_closing_dups_and_spikes(current_output, robust_policy);
                                        rings.push_back(current_output);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        } while (! state.good());
    }
};

}} // namespace detail::overlay
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_TRAVERSE_HPP
