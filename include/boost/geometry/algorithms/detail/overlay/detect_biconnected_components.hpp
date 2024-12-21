// Boost.Geometry

// Copyright (c) 2024 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_DETECT_ARTICULATION_POINTS_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_DETECT_ARTICULATION_POINTS_HPP

#include <map>
#include <set>

#include <boost/geometry/algorithms/detail/overlay/segment_identifier.hpp>
#include <boost/geometry/algorithms/detail/overlay/node_util.hpp>
#include <boost/geometry/algorithms/detail/overlay/graph_util.hpp>

#include <boost/graph/biconnected_components.hpp>
#include <boost/graph/adjacency_list.hpp>

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace overlay
{


struct vertex_info
{
    signed_size_type node_id{0};
    std::set<std::size_t> target_vertex_indices;

    bool is_extra{false};

    // For extra nodes, also store the original node
    signed_size_type original_node_id{0};
};

struct state_type
{
    // Maps from vertex to vertex info
    std::map<std::size_t, vertex_info> vertex_map;

    // Reverse mapping. Every node (turn or cluster) has only ONE vertex,
    // but there might be additional vertices, not associated with a node.
    std::map<signed_size_type, std::size_t> node_to_vertex_index;

    // Keeps track of vertex index, which must, for Boost.Graph,
    // be consecutive. The turn index is not (because of discarded and clusters).
    std::size_t vertex_index{0};

    // For some cases (returning to itself) extra nodes are inserted.
    // They are numbered from turn.size() and up.
    std::size_t extra_node_id{0};
};

void add_target_node(signed_size_type source_node_id, signed_size_type target_node_id,
                     state_type& state, bool allow_returning)
{
    // Insert the source and target node (turn or cluster)
    auto it_source = state.node_to_vertex_index.find(source_node_id);
    if (it_source == state.node_to_vertex_index.end())
    {
        it_source = state.node_to_vertex_index.insert({source_node_id, state.vertex_index++}).first;
    }

    auto it_target = state.node_to_vertex_index.find(target_node_id);
    if (it_target == state.node_to_vertex_index.end())
    {
        it_target = state.node_to_vertex_index.insert({target_node_id, state.vertex_index++}).first;
    }

    // Get the accompanying vertex into (might be a new record)
    auto& vertex_info = state.vertex_map[it_source->second];
    vertex_info.node_id = source_node_id;

    if (target_node_id == source_node_id && allow_returning)
    {
        std::size_t const extra_node_id = state.extra_node_id++;

        // Add a brand new vertex, and add the target to it
        // To keep the index right, first this brand new vertex as the target
        vertex_info.target_vertex_indices.insert(state.vertex_index);
        state.node_to_vertex_index.insert({extra_node_id, state.vertex_index});
        auto& extra_vertex_info = state.vertex_map[state.vertex_index++];
        extra_vertex_info.node_id = extra_node_id;
        extra_vertex_info.is_extra = true;
        extra_vertex_info.original_node_id = source_node_id;
        extra_vertex_info.target_vertex_indices.insert(it_target->second);
    }
    else
    {
        vertex_info.target_vertex_indices.insert(it_target->second);
    }
}


template <operation_type TargetOperation, typename Turns, typename Clusters>
void fill_vertex_map(Turns const& turns, Clusters const& clusters, state_type& state)
{
    std::set<std::size_t> visited_turns;
    for (std::size_t i = 0; i < turns.size(); i++)
    {
        auto const & turn = turns[i];
        if (turn.discarded)
        {
            continue;
        }
        if (visited_turns.count(i) > 0)
        {
            continue;
        }

        auto const source_node_id = get_node_id(turns, i);
        auto const turn_indices = get_turn_indices_by_node_id(turns, clusters, source_node_id);

        visited_turns.insert(turn_indices.begin(), turn_indices.end());

        auto const target_nodes = get_target_nodes<TargetOperation>(turns, clusters, turn_indices);

        for (auto const& target_node_id : target_nodes)
        {
            add_target_node(source_node_id, target_node_id, state, true);
        }
    }
}

// Assigns biconnected components to turns
template <typename Turns, typename Clusters, typename Graph, typename Components>
void assign_biconnected_component_ids(Turns& turns, Clusters const& clusters,
    Graph const& graph, Components const& component, state_type const& state)
{
    auto node_id_from_it = [&state](auto const& it)
    {
        return it->second.is_extra
            ? it->second.original_node_id
            : it->second.node_id;
    };    

    typename graph_traits<Graph>::edge_iterator ei, ei_end;
    for (boost::tie(ei, ei_end) = edges(graph); ei != ei_end; ++ei)
    {
        auto it_source = state.vertex_map.find(source(*ei, graph));
        auto it_target = state.vertex_map.find(target(*ei, graph));
        if (it_source == state.vertex_map.end() || it_target == state.vertex_map.end())
        {
            continue;
        }

        auto const source_node_id = node_id_from_it(it_source);
        auto const target_node_id = node_id_from_it(it_target);

        auto const source_turn_indices = get_turn_indices_by_node_id(turns, clusters, source_node_id);

        // Assign the component to all the operations
        // going from the source node to the target node.
        for (auto const& turn_index : source_turn_indices)
        {
            auto& source_turn = turns[turn_index];
            for (std::size_t j = 0; j < 2; j++)
            {
                auto& op = source_turn.operations[j];
                if (op.enriched.travels_to_ip_index < 0)
                {
                    continue;
                }
                auto const travels_to_node_id = get_node_id(turns, op.enriched.travels_to_ip_index);
                if (travels_to_node_id == target_node_id)
                {
                    op.enriched.component_id = static_cast<int>(component[*ei]);
                }
            }
        }
    }
}

template <operation_type TargetOperation, typename Turns, typename Clusters>
void detect_biconnected_components(Turns& turns, Clusters const& clusters)
{
    using graph_t = boost::adjacency_list
        <
            boost::vecS,
            boost::vecS,
            boost::undirectedS,
            boost::no_property,
            boost::property<edge_component, std::size_t>
        >;
    using vertex_t = boost::graph_traits <graph_t>::vertex_descriptor;

    // Mapping to add turns to vertices, count them, and then build the graph.
    // (It is convenient if the vertex index is the same as the turn index.
    // Therefore the default mapping is made like that, extra vertices
    // are added later)

    state_type state;
    state.extra_node_id = static_cast<std::size_t>(turns.size());

    fill_vertex_map<TargetOperation>(turns, clusters, state);

    // Build the graph from the vertices
    graph_t graph(state.vertex_map.size());
    for (auto const& key_value : state.vertex_map)
    {
        auto const vertex_index = key_value.first;
        for (auto const& target_vertex_index : key_value.second.target_vertex_indices)
        {
            add_edge(vertex_index, target_vertex_index, graph);
        }
    }

    edge_component ec;
    auto component = get(ec, graph);
    biconnected_components(graph, component);
    fix_components(component, graph);

    assign_biconnected_component_ids(turns, clusters, graph, component, state);
}


}} // namespace detail::overlay
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_DETECT_ARTICULATION_POINTS_HPP
