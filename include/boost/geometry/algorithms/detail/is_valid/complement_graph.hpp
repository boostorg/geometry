// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_IS_VALID_COMPLEMENT_GRAPH_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_IS_VALID_COMPLEMENT_GRAPH_HPP

#include <cstddef>

#include <set>
#include <stack>
#include <utility>
#include <vector>

#include <boost/assert.hpp>

#include <boost/geometry/policies/compare.hpp>


namespace boost { namespace geometry
{

namespace detail { namespace is_valid
{


template <typename TurnPoint>
class complement_graph_vertex
{
public:
    complement_graph_vertex(std::size_t id,
                            TurnPoint const& dummy = TurnPoint())
        : m_is_ip(false)
        , m_id(id)
        , m_turn_point(dummy)
    {}

    complement_graph_vertex(TurnPoint const& turn_point,
                            std::size_t expected_id)
        : m_is_ip(true)
        , m_id(expected_id)
        , m_turn_point(turn_point)
    {}

    inline std::size_t id() const { return m_id; }

    inline bool operator<(complement_graph_vertex const& other) const
    {
        if ( m_is_ip && other.m_is_ip )
        {
            return geometry::less
                <
                    TurnPoint
                >()(m_turn_point, other.m_turn_point);
        }
        if ( !m_is_ip && !other.m_is_ip )
        {
            return m_id < other.m_id;
        }
        return other.m_is_ip;
    }

private:
    // the following bool determines the type of the vertex
    // true : vertex corresponds to an IP
    // false: vertex corresponds to a hole or outer space, and the id
    //        is the ring id of the corresponding ring of the polygon
    bool m_is_ip;
    std::size_t m_id;
    TurnPoint const& m_turn_point;
};




template <typename TurnPoint>
class complement_graph
{
private:
    typedef complement_graph_vertex<TurnPoint> vertex;
    typedef std::set<vertex> vertex_container;

public:
    typedef typename vertex_container::const_iterator vertex_handle;

private:
    struct vertex_handle_less
    {
        inline bool operator()(vertex_handle v1, vertex_handle v2) const
        {
            return v1->id() < v2->id();
        }
    };

    typedef std::set<vertex_handle, vertex_handle_less> neighbor_container;

    class has_cycles_dfs_data
    {
    public:
        has_cycles_dfs_data(std::size_t num_nodes)
            : m_visited(num_nodes)
            , m_parent_id(num_nodes)
        {}

        inline int parent_id(vertex_handle v) const
        {
            return m_parent_id[v->id()];
        }

        inline void set_parent_id(vertex_handle v, int id)
        {
            m_parent_id[v->id()] = id;
        }

        inline bool visited(vertex_handle v) const
        {
            return m_visited[v->id()];
        }

        inline void set_visited(vertex_handle v, bool value)
        {
            m_visited[v->id()] = value;
        }
    private:
        std::vector<bool> m_visited;
        std::vector<int> m_parent_id;
    };


    inline bool has_cycles(vertex_handle start_vertex,
                           has_cycles_dfs_data& data) const
    {
        std::stack<vertex_handle> stack;
        stack.push(start_vertex);

        while ( !stack.empty() )
        {
            vertex_handle v = stack.top();
            stack.pop();
            
            data.set_visited(v, true);
            for (typename neighbor_container::const_iterator nit
                     = m_neighbors[v->id()].begin();
                 nit != m_neighbors[v->id()].end(); ++nit)
            {
                if ( static_cast<int>((*nit)->id()) != data.parent_id(v) )
                {
                    if ( data.visited(*nit) )
                    {
                        return true;
                    }
                    else
                    {
                        data.set_parent_id(*nit, static_cast<int>(v->id()));
                        stack.push(*nit);
                    }
                }
            }
        }
        return false;
    }

public:
    // num_rings: total number of rings, including the exterior ring
    complement_graph(std::size_t num_rings)
        : m_num_rings(num_rings)
        , m_num_turns(0)
        , m_vertices()
        , m_neighbors(num_rings)
    {}

    // inserts a ring vertex in the graph and returns its handle
    // ring id's are zero-based (so the first interior ring has id 1)
    inline vertex_handle add_vertex(int id)
    {
        return m_vertices.insert(vertex(static_cast<std::size_t>(id))).first;
    }

    // inserts an IP in the graph and returns its id
    inline vertex_handle add_vertex(TurnPoint const& turn_point)
    {
        std::pair<vertex_handle, bool> res
            = m_vertices.insert(vertex(turn_point, m_num_rings + m_num_turns));

        if ( res.second )
        {
            // a new element is inserted
            m_neighbors.push_back(neighbor_container());
            ++m_num_turns;
        }
        return res.first;
    }

    inline void add_edge(vertex_handle v1, vertex_handle v2)
    {
        BOOST_ASSERT( v1 != m_vertices.end() );
        BOOST_ASSERT( v2 != m_vertices.end() );
        m_neighbors[v1->id()].insert(v2);
        m_neighbors[v2->id()].insert(v1);
    }

    inline bool has_cycles() const
    {
        has_cycles_dfs_data data(m_vertices.size());

        // initialize all vertices as non-visited and with no parent set
        for (vertex_handle it = m_vertices.begin();
             it != m_vertices.end(); ++it)
        {
            data.set_visited(it, false);
            data.set_parent_id(it, -1);
        }

        // for each non-visited vertex, start a DFS from that vertex
        for (vertex_handle it = m_vertices.begin();
             it != m_vertices.end(); ++it)
        {
            if ( !data.visited(it) && has_cycles(it, data) )
            {
                return true;
            }
        }
        return false;
    }

    template <typename OStream, typename TP>
    friend inline
    void debug_print_complement_graph(OStream&, complement_graph<TP> const&);

private:
    std::size_t m_num_rings, m_num_turns;
    vertex_container m_vertices;
    std::vector<neighbor_container> m_neighbors;
};


}} // namespace detail::is_valid

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_IS_VALID_COMPLEMENT_GRAPH_HPP
