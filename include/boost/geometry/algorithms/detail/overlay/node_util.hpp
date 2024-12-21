// Boost.Geometry

// Copyright (c) 2024 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_NODE_UTIL_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_NODE_UTIL_HPP

#include <set>

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace overlay
{
template <typename Turns, typename Clusters>
std::set<std::size_t> get_turn_indices_by_cluster_id(Turns const& turns, Clusters const& clusters, signed_size_type cluster_id)
{
    std::set<std::size_t> result;
    auto it = clusters.find(cluster_id);
    if (it != clusters.end())
    {
        result.insert(it->second.turn_indices.begin(), it->second.turn_indices.end());
    }
    return result;
}

// Returns the node id of the turn:
// - if it is clustered, the negative cluster_id
// - if it is not clustered, the turn index
template <typename Turns>
signed_size_type get_node_id(Turns const& turns, std::size_t turn_index)
{
    auto const& turn = turns[turn_index];
    return turn.is_clustered() ? -turn.cluster_id : turn_index;
}

template <typename Turns, typename Clusters>
std::set<std::size_t> get_turn_indices_by_node_id(Turns const& turns, Clusters const& clusters, signed_size_type node_id)
{
    if (node_id < 0)
    {
        return get_turn_indices_by_cluster_id(turns, clusters, -node_id);
    }
    if (node_id >= turns.size())
    {
        // It is 'allowed' to have node_ids larger than the largest turn index (for example extra
        // nodes in a graph). But they are not related to turns.
        return {};
    }

    auto const turn_index = static_cast<std::size_t>(node_id);
    auto const& turn = turns[turn_index];
    if (turn.is_clustered())
    {
        return get_turn_indices_by_cluster_id(turns, clusters, turn.cluster_id);
    }
    return {turn_index};
}

// TODO: later union too
template <operation_type TargetOperation, typename Operation>
bool is_target_operation(Operation const& op, std::size_t turn_count, int op_index)
{
    return
        op.enriched.travels_to_ip_index >= 0
        && op.enriched.travels_to_ip_index < static_cast<int>(turn_count)
        && (op.operation == TargetOperation || (op_index == 0 && op.operation == operation_continue));
}

template <operation_type TargetOperation, typename Turns, typename Clusters>
auto get_blocked_nodes(Turns const& turns, Clusters const& clusters,
                      std::set<std::size_t> const& turn_indices)
{
    std::set<signed_size_type> blocked_nodes;
    for (std::size_t turn_index : turn_indices)
    {
        auto const& turn = turns[turn_index];

        if (turn.discarded)
        {
            continue;
        }
        if (! turn.combination(TargetOperation, operation_blocked))
        {
            continue;
        }

        for (std::size_t j = 0; j < 2; j++)
        {
            auto const& op = turn.operations[j];
            if (op.operation == operation_blocked)
            {
                blocked_nodes.insert(get_node_id(turns, op.enriched.travels_to_ip_index));
            }
        }
    }
    return blocked_nodes;
}

template <operation_type TargetOperation, typename Turns, typename Clusters>
auto get_target_nodes(Turns const& turns, Clusters const& clusters, 
                      std::set<std::size_t> const& turn_indices)
{
    auto const blocked_nodes = get_blocked_nodes<TargetOperation>(turns, clusters, turn_indices);

    std::set<signed_size_type> target_nodes;
    for (std::size_t turn_index : turn_indices)
    {
        auto const& turn = turns[turn_index];
        if (turn.discarded)
        {
            continue;
        }

        bool const is_clustered_self = turn.is_self() && turn.is_clustered();
        for (std::size_t j = 0; j < 2; j++)
        {
            auto const& op = turn.operations[j];

            if (is_target_operation<TargetOperation>(op, turns.size(), j))
            {
                auto const target_node_id = get_node_id(turns, op.enriched.travels_to_ip_index);
                if (is_clustered_self && blocked_nodes.count(target_node_id) > 0)
                {
                    continue;
                }
                target_nodes.insert(target_node_id);
            }
        }
    }

    return target_nodes;
}


}} // namespace detail::overlay
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_NODE_UTIL_HPP
