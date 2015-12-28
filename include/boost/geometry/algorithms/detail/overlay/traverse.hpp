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
                //std::cout << " RETURN";
                return true;
            }

            // In some cases there are two alternatives.
            // For "ii", take the other one (alternate)
            //           UNLESS the other one is already visited
            // For "uu", take the same one (see above);
            // For "cc", take either one, but if there is a starting one,
            //           take that one.
            if (   (op.operation == operation_continue
                    && (! result || op.visited.started()
                        )
                    )
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
                if (op.enriched.next_ip_index == start_turn_index
                    || op.enriched.travels_to_ip_index == start_turn_index)
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

    inline bool select_turn_from_cluster(
                typename boost::range_iterator<Turns>::type& turn_it,
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
                signed_size_type const turn_index = ranked_point.turn_index;
                if (turn_index != -1)
                {
                    turn_it = m_turns.begin() + turn_index;
                    return true;
                }
            }
            if (ranked_point.main_rank > 1)
            {
                // Nothing found, don't change
                return false;
            }
        }
        return false;
    }

    template <typename Ring>
    inline bool travel_to_next_turn(turn_iterator& it,
                Ring& current_ring,
                turn_operation_type& op,
                segment_identifier& seg_id)
    {
        // If there is no next IP on this segment
        if (op.enriched.next_ip_index < 0)
        {
            if (op.enriched.travels_to_vertex_index < 0
                || op.enriched.travels_to_ip_index < 0)
            {
                return false;
            }

            BOOST_GEOMETRY_ASSERT(op.enriched.travels_to_vertex_index >= 0);
            BOOST_GEOMETRY_ASSERT(op.enriched.travels_to_ip_index >= 0);

            if (op.seg_id.source_index == 0)
            {
                geometry::copy_segments<Reverse1>(m_geometry1, op.seg_id,
                        op.enriched.travels_to_vertex_index,
                        m_robust_policy,
                        current_ring);
            }
            else
            {
                geometry::copy_segments<Reverse2>(m_geometry2, op.seg_id,
                        op.enriched.travels_to_vertex_index,
                        m_robust_policy,
                        current_ring);
            }
            seg_id = op.seg_id;
            it = boost::begin(m_turns) + op.enriched.travels_to_ip_index;
        }
        else
        {
            it = boost::begin(m_turns) + op.enriched.next_ip_index;
            seg_id = op.seg_id;
        }

        if (it->cluster_id >= 0)
        {
            if (! select_turn_from_cluster(it, op))
            {
                return false;
            }
        }

        detail::overlay::append_no_dups_or_spikes(current_ring, it->point,
            m_robust_policy);

        return true;
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

        if (! travel_to_next_turn(current_it, ring,
                    start_op, current_seg_id))
        {
            // This is not necessarily a problem, it happens for clustered turns
            // which are "build in" or otherwise point inwards
            return traverse_error_no_next_ip_at_start;
        }

        // Register the start
        start_op.visited.set_started();
        m_visitor.visit_traverse(m_turns, start_turn, start_op, "Start");

        if (! select_operation(*current_it,
                        start_turn_index,
                        current_seg_id,
                        current_op_it))
        {
            return traverse_error_dead_end_at_start;
        }

        // Register the first visit
        set_visited(*current_it, *current_op_it);
        m_visitor.visit_traverse(m_turns, *current_it, *current_op_it, "Visit");

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
            if (! travel_to_next_turn(current_it, ring,
                *current_op_it, current_seg_id))
            {
                return traverse_error_no_next_ip;
            }

            if (! select_operation(*current_it,
                        start_turn_index,
                        current_seg_id,
                        current_op_it))
            {
                return traverse_error_dead_end;
            }

            if (current_op_it->visited.visited())
            {
                return traverse_error_visit_again;
            }

            set_visited(*current_it, *current_op_it);
            m_visitor.visit_traverse(m_turns, *current_it, *current_op_it, "Visit");

            if (current_op_it == start_op_it)
            {
                start_op.visited.set_finished();
                m_visitor.visit_traverse(m_turns, *current_it, start_op, "Finish");
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
        do
        {
            state.reset();

            signed_size_type start_turn_index = 0;

            // Iterate through all unvisited points
            for (turn_iterator turn_it = boost::begin(turns);
                state.good() && turn_it != boost::end(turns);
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
                    state.good() && op_it != boost::end(start_turn.operations);
                    ++op_it)
                {
                    op_type& start_op = *op_it;

                    if (!start_op.visited.none()
                        || start_op.visited.rejected()
                        || !(start_op.operation == OpType
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
        } while (! state.good());
    }
};

}} // namespace detail::overlay
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_TRAVERSE_HPP
