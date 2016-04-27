// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2015-2016 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_TRAVERSAL_SWITCH_DETECTOR_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_TRAVERSAL_SWITCH_DETECTOR_HPP

#include <cstddef>

#include <boost/range.hpp>

#include <boost/geometry/algorithms/detail/overlay/copy_segments.hpp>
#include <boost/geometry/algorithms/detail/overlay/turn_info.hpp>
#include <boost/geometry/algorithms/detail/overlay/traversal.hpp>
#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/assert.hpp>

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace overlay
{


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
struct traversal_switch_detector
{
    typedef traversal<Reverse1, Reverse2, OperationType,
           Geometry1, Geometry2, Turns, Clusters, RobustPolicy, Visitor>
        traversal_type;

    typedef typename boost::range_value<Turns>::type turn_type;
    typedef typename turn_type::turn_operation_type turn_operation_type;

    inline traversal_switch_detector(Geometry1 const& geometry1, Geometry2 const& geometry2,
            Turns& turns, Clusters const& clusters,
            RobustPolicy const& robust_policy, Visitor& visitor)
        : m_trav(geometry1, geometry2, turns, clusters, robust_policy,visitor)
        , m_geometry1(geometry1)
        , m_geometry2(geometry2)
        , m_turns(turns)
        , m_clusters(clusters)
        , m_robust_policy(robust_policy)
        , m_visitor(visitor)
    {

    }

    inline traverse_error_type travel_to_next_turn(signed_size_type start_turn_index,
                int start_op_index,
                signed_size_type& turn_index,
                int& op_index,
                bool is_start)
    {
        int const previous_op_index = op_index;
        signed_size_type const previous_turn_index = turn_index;
        turn_type& previous_turn = m_turns[turn_index];
        turn_operation_type& previous_op = previous_turn.operations[op_index];
        segment_identifier previous_seg_id;

        signed_size_type to_vertex_index = -1;
        if (! m_trav.select_turn_from_enriched(turn_index, previous_seg_id,
                          to_vertex_index, start_turn_index, start_op_index,
                          previous_turn, previous_op, is_start))
        {
            return is_start
                    ? traverse_error_no_next_ip_at_start
                    : traverse_error_no_next_ip;
        }

        if (m_turns[turn_index].discarded)
        {
            return is_start
                ? traverse_error_dead_end_at_start
                : traverse_error_dead_end;
        }

        if (is_start)
        {
            // Register the start
            previous_op.visited.set_started();
            m_visitor.visit_traverse(m_turns, previous_turn, previous_op, "Start");
        }

        if (! m_trav.select_turn(start_turn_index, turn_index, op_index,
                previous_op_index, previous_turn_index, previous_seg_id,
                is_start))
        {
            return is_start
                ? traverse_error_no_next_ip_at_start
                : traverse_error_no_next_ip;
        }

        {
            // Check operation (TODO: this might be redundant or should be catched before)
            const turn_type& current_turn = m_turns[turn_index];
            const turn_operation_type& op = current_turn.operations[op_index];
            if (op.visited.finalized()
                || m_trav.is_visited(current_turn, op, turn_index, op_index))
            {
                return traverse_error_visit_again;
            }
        }

        // Update registration and append point
        turn_type& current_turn = m_turns[turn_index];
        turn_operation_type& op = current_turn.operations[op_index];

        // Register the visit
        m_trav.set_visited(current_turn, op);
        m_visitor.visit_traverse(m_turns, current_turn, op, "Visit");

        return traverse_error_none;
    }

    inline traverse_error_type traverse(bool& do_switch, signed_size_type start_turn_index, int start_op_index)
    {
        turn_type& start_turn = m_turns[start_turn_index];
        turn_operation_type& start_op = m_turns[start_turn_index].operations[start_op_index];

        signed_size_type current_turn_index = start_turn_index;
        int current_op_index = start_op_index;

        traverse_error_type error = travel_to_next_turn(start_turn_index,
                    start_op_index,
                    current_turn_index, current_op_index,
                    true);

        if (error != traverse_error_none)
        {
            // This is not necessarily a problem, it happens for clustered turns
            // which are "build in" or otherwise point inwards
            return error;
        }

        // SWITCH
        {
            turn_type const& turn = m_turns[current_turn_index];
            if (turn.both(operation_union))
            {
#if defined(BOOST_GEOMETRY_DEBUG_TRAVERSAL_SWITCH_DETECTOR)
                std::cout << "FOUND OTHER UU TURN RIGHT AFTER START "
                          << current_turn_index
                          << "  started at " << start_turn_index
                          << std::endl;
#endif
                return traverse_error_none;
            }
        } // SWITCH

        if (current_turn_index == start_turn_index)
        {
            start_op.visited.set_finished();

            m_visitor.visit_traverse(m_turns, m_turns[current_turn_index], start_op, "Early finish SWITCH");
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
            error = travel_to_next_turn(start_turn_index, start_op_index,
                    current_turn_index, current_op_index,
                    false);

            if (error != traverse_error_none)
            {
                return error;
            }

            // SWITCH
            {
                turn_type const& turn = m_turns[current_turn_index];
                if (turn.both(operation_union))
                {
                    if (current_turn_index == start_turn_index)
                    {
                        // Found the origin again without uu turns in between
                        do_switch = true;
                    }
                    // Return, it does not need to be finished
                    return traverse_error_none;
                }
            } // SWITCH

            if (current_turn_index == start_turn_index
                    && current_op_index == start_op_index)
            {
                start_op.visited.set_finished();
                m_visitor.visit_traverse(m_turns, start_turn, start_op, "Finish SWITCH");
                return traverse_error_none;
            }
        }

        return traverse_error_endless_loop;
    }

    void traverse_with_operation(bool& do_switch, turn_type const& start_turn,
            std::size_t turn_index, int op_index)
    {
        turn_operation_type const& start_op = start_turn.operations[op_index];

        if (! start_op.visited.none()
            || ! start_op.enriched.startable
            || start_op.visited.rejected()
            || ! (start_op.operation == OperationType
                || start_op.operation == detail::overlay::operation_continue))
        {
            return;
        }

        traverse_error_type traverse_error = traverse(do_switch, turn_index, op_index);

        if (traverse_error != traverse_error_none)
        {
            // Reject this as a starting point
            m_turns[turn_index].operations[op_index].visited.set_rejected();

            // And clear all visit info
            clear_visit_info(m_turns);
        }
    }

    void reset_uu_turn(turn_type& turn)
    {
        for (int i = 0; i < 2; i++)
        {
            turn.operations[i].visited.reset();
        }
    }

    void iterate()
    {
#if defined(BOOST_GEOMETRY_DEBUG_TRAVERSAL_SWITCH_DETECTOR)
        std::cout << "SWITCH BEGIN ITERATION" << std::endl;
#endif

        // Iterate through all unvisited points
        for (std::size_t turn_index = 0; turn_index < m_turns.size(); ++turn_index)
        {
            turn_type& start_turn = m_turns[turn_index];

            if (start_turn.discarded || start_turn.blocked())
            {
                // Skip discarded and blocked turns
                continue;
            }
            if (! start_turn.both(operation_union))
            {
                // For switch-iteration, only start with union
                continue;
            }


            bool do_switch[2] = {0};
            for (int op_index = 0; op_index < 2; op_index++)
            {
                reset_uu_turn(start_turn);
#if defined(BOOST_GEOMETRY_DEBUG_TRAVERSAL_SWITCH_DETECTOR)
                std::cout << "SWITCH EXAMINE " << turn_index << "[" << op_index << "]" << std::endl;
#endif
                traverse_with_operation(do_switch[op_index], start_turn,
                                        turn_index, op_index);
            }
            start_turn.switch_source = do_switch[0] || do_switch[1];

            // Because it was unknown before if traversal per operation should switch, visit-info should be cleared
            reset_uu_turn(start_turn);
        }


#if defined(BOOST_GEOMETRY_DEBUG_TRAVERSAL_SWITCH_DETECTOR)
        std::cout << "SWITCH END ITERATION" << std::endl;

        for (std::size_t turn_index = 0; turn_index < m_turns.size(); ++turn_index)
        {
            turn_type const& start_turn = m_turns[turn_index];

            if (start_turn.both(operation_union))
            {
                std::cout << "SWITCH RESULT "
                             << turn_index << " -> "
                          << start_turn.switch_source << std::endl;
            }
        }
#endif

    }

private:
    traversal_type m_trav;

    Geometry1 const& m_geometry1;
    Geometry2 const& m_geometry2;
    Turns& m_turns;
    Clusters const& m_clusters;
    RobustPolicy const& m_robust_policy;
    Visitor& m_visitor;
};

}} // namespace detail::overlay
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_TRAVERSAL_SWITCH_DETECTOR_HPP
