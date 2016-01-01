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
#include <boost/geometry/algorithms/detail/overlay/sort_by_side.hpp>
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
    operation_type OperationType,
    typename Geometry1,
    typename Geometry2,
    typename Turns,
    typename Clusters,
    typename RobustPolicy,
    typename Visitor
>
struct traversal
{
    typedef typename side_compare<OperationType>::type side_compare_type;
    typedef typename boost::range_value<Turns>::type turn_type;
    typedef typename turn_type::turn_operation_type turn_operation_type;

    typedef typename boost::range_iterator<Turns>::type turn_iterator;
    typedef typename boost::range_iterator
        <
            typename turn_type::container_type
        >::type turn_operation_iterator_type;

    typedef typename geometry::point_type<Geometry1>::type point_type;
    typedef sort_by_side::side_sorter
        <
            Reverse1, Reverse2,
            point_type, side_compare_type
        > sbs_type;

    inline traversal(Geometry1 const& geometry1, Geometry2 const& geometry2,
            Turns& turns, Clusters const& clusters,
            RobustPolicy const& robust_policy, Visitor& visitor)
        : m_geometry1(geometry1)
        , m_geometry2(geometry2)
        , m_turns(turns)
        , m_clusters(clusters)
        , m_robust_policy(robust_policy)
        , m_visitor(visitor)
    {}


    inline bool select_source(signed_size_type source1,
                              signed_size_type source2)
    {
        return (OperationType == operation_intersection && source1 != source2)
            || (OperationType == operation_union && source1 == source2)
            ;
    }

    inline
    signed_size_type get_next_turn_index(turn_operation_type const& op) const
    {
        return op.enriched.next_ip_index == -1
                ? op.enriched.travels_to_ip_index
                : op.enriched.next_ip_index;
    }

    inline bool traverse_possible(signed_size_type turn_index) const
    {
        if (turn_index == -1)
        {
            return false;
        }

        turn_type const& turn = m_turns[turn_index];

        // It is not a dead end if there is an operation to continue, or of
        // there is a cluster (assuming for now we can get out of the cluster)
        return turn.cluster_id >= 0
            || turn.has(OperationType)
            || turn.has(operation_continue);
    }

    inline bool select_operation(turn_type& turn,
                signed_size_type start_turn_index,
                segment_identifier const& seg_id,
                turn_operation_iterator_type& selected)
    {
        if (turn.discarded)
        {
            return false;
        }

        bool result = false;

        typename turn_operation_type::comparable_distance_type
                max_remaining_distance = 0;

        selected = boost::end(turn.operations);
        for (turn_operation_iterator_type it = boost::begin(turn.operations);
            it != boost::end(turn.operations);
            ++it)
        {
            turn_operation_type const& op = *it;
            if (op.visited.started())
            {
                selected = it;
                return true;
            }

            signed_size_type const next_turn_index = get_next_turn_index(op);

            // In some cases there are two alternatives.
            // For "ii", take the other one (alternate)
            //           UNLESS the other one is already visited
            // For "uu", take the same one (see above);
            // For "cc", take either one, but if there is a starting one,
            //           take that one. If next is dead end, skip that one.
            if (   (op.operation == operation_continue
                    && traverse_possible(next_turn_index)
                    && ! result)
                || (op.operation == OperationType
                    && ! op.visited.finished()
                    && (! result
                        || select_source(op.seg_id.source_index, seg_id.source_index)
                        )
                    )
                )
            {
                if (op.operation == operation_continue)
                {
                    max_remaining_distance = op.remaining_distance;
                }
                selected = it;
                debug_traverse(turn, op, " Candidate");
                result = true;
            }

            if (op.operation == operation_continue && result)
            {
                if (next_turn_index == start_turn_index)
                {
                    selected = it;
                    debug_traverse(turn, op, " Candidate override (start)");
                }
                else if (op.remaining_distance > max_remaining_distance)
                {
                    max_remaining_distance = op.remaining_distance;
                    selected = it;
                    debug_traverse(turn, op, " Candidate override (remaining)");
                }
            }
        }

        if (result)
        {
           debug_traverse(turn, *selected, "  Accepted");
        }

        return result;
    }

    inline bool select_turn_from_cluster(turn_iterator& turn_it,
                turn_operation_iterator_type& op_it,
                turn_operation_type const& op)
    {
        turn_type const& turn = *turn_it;
        BOOST_ASSERT(turn.cluster_id >= 0);

        typename Clusters::const_iterator mit = m_clusters.find(turn.cluster_id);
        BOOST_ASSERT(mit != m_clusters.end());

        std::set<signed_size_type> const& ids = mit->second;

        sbs_type sbs;
        bool has_subject = false;

        for (typename std::set<signed_size_type>::const_iterator sit = ids.begin();
             sit != ids.end(); ++sit)
        {
            signed_size_type turn_index = *sit;
            turn_type const& cturn = m_turns[turn_index];
            for (int i = 0; i < 2; i++)
            {
                turn_operation_type const& cop = cturn.operations[i];

                // TODO: source_index is NOT a good criterium
                bool const is_subject = &cturn == &turn
                               && op.seg_id.source_index == cop.seg_id.source_index;
                if (is_subject)
                {
                    has_subject = true;
                }

                sbs.add(cop, turn_index, i, m_geometry1, m_geometry2, is_subject);
            }
        }
        if (! has_subject)
        {
            std::cout << "WARNING: no subject" << std::endl;
            return false;
        }
        sbs.apply(turn.point);


        bool result = false;
        for (std::size_t i = 0; i < sbs.m_ranked_points.size(); i++)
        {
            const typename sbs_type::rp& ranked_point = sbs.m_ranked_points[i];

            if (ranked_point.main_rank == 0 && ranked_point.index != sort_by_side::index_from)
            {
                // There are outgoing arcs, not a good cluster to start or
                // to continue
                return false;
            }

            if (ranked_point.main_rank == 1
                    && ranked_point.index == sort_by_side::index_to
                    && ranked_point.operation == operation_blocked)
            {
                return false;
            }

            if (ranked_point.main_rank == 1
                    && ranked_point.index == sort_by_side::index_to
                    && (ranked_point.operation == OperationType
                        || ranked_point.operation == operation_continue))
            {
                turn_type const& ranked_turn = m_turns[ranked_point.turn_index];

                if (ranked_turn.discarded)
                {
                    // Might be collocated u/u turn
                    continue;
                }

                // Use this turn (if also part of a cluster, it will point to
                // next turn outside cluster)
                turn_operation_type const& ranked_op = ranked_turn.operations[ranked_point.op_index];

                turn_it = m_turns.begin() + ranked_point.turn_index;

                if (ranked_turn.both(operation_intersection)
                    && ranked_op.visited.finalized())
                {
                    // For a ii turn, even though one operation might be selected,
                    // it should take the other one if the first one is used in a completed ring
                    op_it = turn_it->operations.begin() + (1 - ranked_point.op_index);
                }
                else
                {
                    // Normal behaviour
                    op_it = turn_it->operations.begin() + ranked_point.op_index;
                }

                result = true;
                // Don't return yet, maybe there is a blocked
            }
            if (ranked_point.main_rank > 1)
            {
                return result;
            }
        }
        return result;
    }

    template <typename Ring>
    inline traverse_error_type travel_to_next_turn(signed_size_type start_turn_index,
                turn_iterator& turn_it,
                turn_operation_iterator_type& op_it,
                segment_identifier& seg_id,
                Ring& current_ring,
                bool is_start)
    {
        turn_operation_iterator_type previous_op_it = op_it;
        turn_iterator previous_turn_it = turn_it;
        turn_type& previous_turn = *turn_it;
        turn_operation_type& previous_op = *op_it;

        // If there is no next IP on this segment
        if (previous_op.enriched.next_ip_index < 0)
        {
            if (previous_op.enriched.travels_to_vertex_index < 0
                || previous_op.enriched.travels_to_ip_index < 0)
            {
                return is_start
                        ? traverse_error_no_next_ip_at_start
                        : traverse_error_no_next_ip;
            }

            if (previous_op.seg_id.source_index == 0)
            {
                geometry::copy_segments<Reverse1>(m_geometry1, previous_op.seg_id,
                        previous_op.enriched.travels_to_vertex_index,
                        m_robust_policy,
                        current_ring);
            }
            else
            {
                geometry::copy_segments<Reverse2>(m_geometry2, previous_op.seg_id,
                        previous_op.enriched.travels_to_vertex_index,
                        m_robust_policy,
                        current_ring);
            }
            seg_id = previous_op.seg_id;
            turn_it = boost::begin(m_turns) + previous_op.enriched.travels_to_ip_index;
        }
        else
        {
            turn_it = boost::begin(m_turns) + previous_op.enriched.next_ip_index;
            seg_id = previous_op.seg_id;
        }

        bool const has_cluster = turn_it->cluster_id >= 0;
        if (has_cluster)
        {
            if (! select_turn_from_cluster(turn_it, op_it, previous_op))
            {
                return is_start
                    ? traverse_error_no_next_ip_at_start
                    : traverse_error_no_next_ip;
            }

            if (is_start && turn_it == previous_turn_it)
            {
                op_it = previous_op_it;
            }
        }

        detail::overlay::append_no_dups_or_spikes(current_ring, turn_it->point,
            m_robust_policy);


        if (is_start)
        {
            // Register the start
            previous_op.visited.set_started();
            m_visitor.visit_traverse(m_turns, previous_turn, previous_op, "Start");
        }

        if (! has_cluster)
        {
            if (! select_operation(*turn_it,
                            start_turn_index,
                            seg_id,
                            op_it))
            {
                return is_start
                    ? traverse_error_dead_end_at_start
                    : traverse_error_dead_end;
            }
        }

        turn_operation_type& op = *op_it;
        if (op.visited.visited())
        {
            return traverse_error_visit_again;
        }

        // Register the visit
        set_visited(*turn_it, op);
        m_visitor.visit_traverse(m_turns, *turn_it, op, "Visit");

        return traverse_error_none;
    }

    inline void finalize_visit_info()
    {
        for (typename boost::range_iterator<Turns>::type
            it = boost::begin(m_turns);
            it != boost::end(m_turns);
            ++it)
        {
            for (typename boost::range_iterator
                <
                    typename turn_type::container_type
                >::type op_it = boost::begin(it->operations);
                op_it != boost::end(it->operations);
                ++op_it)
            {
                op_it->visited.finalize();
            }
        }
    }

    inline void set_visited(turn_type& turn, turn_operation_type& op)
    {
        op.visited.set_visited();

        // On "continue", set "visited" for ALL directions in this turn
        if (op.operation == detail::overlay::operation_continue)
        {
            for (turn_operation_iterator_type it = boost::begin(turn.operations);
                it != boost::end(turn.operations);
                ++it)
            {
                if (it->visited.none())
                {
                    it->visited.set_visited();
                }
            }
        }
    }


    template <typename Ring>
    inline traverse_error_type traverse(Ring& ring, turn_type const& start_turn,
            signed_size_type start_turn_index,
            turn_operation_type& start_op,
            turn_iterator start_it, turn_operation_iterator_type start_op_it)
    {
        detail::overlay::append_no_dups_or_spikes(ring,
            start_turn.point, m_robust_policy);

        // Copy iterators, can be reassigned below
        turn_iterator current_it = start_it;
        turn_operation_iterator_type current_op_it = start_op_it;
        segment_identifier current_seg_id;

        traverse_error_type error = travel_to_next_turn(start_turn_index,
                    current_it, current_op_it, current_seg_id,
                    ring, true);

        if (error != traverse_error_none)
        {
            // This is not necessarily a problem, it happens for clustered turns
            // which are "build in" or otherwise point inwards
            return error;
        }

        if (current_it == start_it)
        {
            start_op.visited.set_finished();
            m_visitor.visit_traverse(m_turns, *current_it, start_op, "Early finish");
            return traverse_error_none;
        }

        std::size_t const max_iterations = 2 + 2 * m_turns.size();
        for (std::size_t i = 0; i <= max_iterations; i++)
        {
            // We assume clockwise polygons only, non self-intersecting, closed.
            // However, the input might be different, and checking validity
            // is up to the library user.

            // Therefore we make here some sanity checks. If the input
            // violates the assumptions, the output polygon will not be correct
            // but the routine will stop and output the current polygon, and
            // will continue with the next one.

            // Below three reasons to stop.
            error = travel_to_next_turn(start_turn_index,
                    current_it, current_op_it, current_seg_id,
                    ring, false);

            if (error != traverse_error_none)
            {
                return error;
            }

            if (current_op_it == start_op_it)
            {
                start_op.visited.set_finished();
                m_visitor.visit_traverse(m_turns, start_turn, start_op, "Finish");
                return traverse_error_none;
            }
        }

        return traverse_error_endless_loop;
    }

private :
    Geometry1 const& m_geometry1;
    Geometry2 const& m_geometry2;
    Turns& m_turns;
    Clusters const& m_clusters;
    RobustPolicy const& m_robust_policy;
    Visitor& m_visitor;
};



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

        traversal
            <
                Reverse1, Reverse2, OpType,
                Geometry1, Geometry2,
                Turns, Clusters,
                RobustPolicy, Visitor
            > trav(geometry1, geometry2, turns, clusters,
                   robust_policy, visitor);

        std::size_t const min_num_points
                = core_detail::closure::minimum_ring_size
                        <
                            geometry::closure<ring_type>::value
                        >::value;

        std::size_t finalized_ring_size = boost::size(rings);

        typename Backtrack::state_type state;

        signed_size_type start_turn_index = 0;

        // Iterate through all unvisited points
        for (turn_iterator turn_it = boost::begin(turns);
            turn_it != boost::end(turns);
            ++turn_it, ++start_turn_index)
        {
            turn_type& start_turn = *turn_it;

            // Skip discarded ones
            if (start_turn.discarded
                || ! start_turn.selectable_start
                || start_turn.blocked())
            {
                continue;
            }

            for (turn_operation_iterator_type op_it = boost::begin(start_turn.operations);
                op_it != boost::end(start_turn.operations);
                ++op_it)
            {
                op_type& start_op = *op_it;

                if (! start_op.visited.none()
                    || start_op.visited.rejected()
                    || ! (start_op.operation == OpType
                        || start_op.operation == detail::overlay::operation_continue))
                {
                    continue;
                }

                ring_type ring;
                traverse_error_type traverse_error = trav.traverse(ring,
                                start_turn, start_turn_index,
                                start_op, turn_it, op_it);

                if (traverse_error == traverse_error_none)
                {
                    if (geometry::num_points(ring) >= min_num_points)
                    {
                        clean_closing_dups_and_spikes(ring, robust_policy);
                        rings.push_back(ring);

                        trav.finalize_visit_info();
                        finalized_ring_size++;
                    }
                }
                else
                {
                    Backtrack::apply(
                        finalized_ring_size,
                        rings, ring, turns, start_turn, start_op,
                        traverse_error,
                        geometry1, geometry2, robust_policy, state, visitor);
                }
            }
        }
    }
};

}} // namespace detail::overlay
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_TRAVERSE_HPP
