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

#include <boost/geometry/policies/compare.hpp>


namespace boost { namespace geometry
{

namespace detail { namespace is_valid
{


template <typename TurnPoint>
class graph_vertex
{
public:
    typedef typename std::set
        <
            graph_vertex<TurnPoint>
        >::const_iterator vertex_handle;

    struct vertex_handle_less
    {
        bool operator()(vertex_handle v1, vertex_handle v2) const
        {
            return v1->id() < v2->id();
        }
    };

    typedef std::set<vertex_handle, vertex_handle_less> neighbor_container;
    typedef typename neighbor_container::const_iterator neighbor_iterator;

    graph_vertex(int id, TurnPoint const& dummy = TurnPoint())
        : m_is_ip(false)
        , m_id(id)
        , m_turn_point(dummy)
        , m_neighbors()
        , m_parent()
        , m_visited(false)
    {}

    graph_vertex(TurnPoint const& turn_point)
        : m_is_ip(true)
        , m_id(-1)
        , m_turn_point(turn_point)
        , m_neighbors()
        , m_parent()
        , m_visited(false)
    {}

    int id() const { return m_id; }
    void id(int id) const { m_id = id; }
    int parent_id() const
    {
        if ( m_parent.second )
        {
            return m_parent.first->id();
        }
        return -1;
    }

    // for DFS -- start
    bool visited() const { return m_visited; }
    void visited(bool value) const { m_visited = value; }

    vertex_handle parent() const { return m_parent.first; }
    void parent(vertex_handle p) const { m_parent = std::make_pair(p, true); }
    void initialize_parent() const
    {
        m_parent = std::make_pair(vertex_handle(), false);
    }
    // for DFS -- end

    void add_neighbor(vertex_handle v) const
    {
        m_neighbors.insert(v);
    }

    neighbor_iterator neighbors_begin() const
    {
        return m_neighbors.begin();
    }

    neighbor_iterator neighbors_end() const
    {
        return m_neighbors.end();
    }

    bool operator<(graph_vertex const& other) const
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
    mutable int m_id;
    TurnPoint const& m_turn_point;
    mutable neighbor_container m_neighbors;
    mutable std::pair<vertex_handle, bool> m_parent; // for DFS
    mutable bool m_visited; // for DFS
};


template <typename Vertex>
class complement_graph
{
private:
    typedef std::set<Vertex> vertex_container;

public:
    typedef typename vertex_container::const_iterator vertex_handle;

private:
    bool has_cycles(vertex_handle start_vertex) const
    {
        std::stack<vertex_handle> stack;
        stack.push(start_vertex);

        while ( !stack.empty() )
        {
            vertex_handle v = stack.top();
            stack.pop();
            
            v->visited(true);
            for (typename Vertex::neighbor_iterator nit = v->neighbors_begin();
                 nit != v->neighbors_end(); ++nit)
            {
                if ( (*nit)->id() != v->parent_id() )
                {
                    if ( !(*nit)->visited() )
                    {
                        (*nit)->parent(v);
                        stack.push(*nit);
                    }
                    else
                    {
                        return true;
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
    {}

    // inserts a ring vertex in the graph and returns its id
    vertex_handle add_vertex(int ring_id)
    {
        Vertex v(static_cast<std::size_t>(ring_id + 1));
        return m_vertices.insert(v).first;
    }

    // inserts an IP in the graph and returns its id
    template <typename TurnPoint>
    vertex_handle add_vertex(TurnPoint const& turn_point)
    {
        Vertex v(turn_point);
        std::pair<vertex_handle, bool> res = m_vertices.insert(v);
        if ( res.second )
        {
            // a new element is inserted
            res.first->id( m_num_rings + m_num_turns );
            ++m_num_turns;
        }
        return res.first;
    }

    void add_edge(vertex_handle v1, vertex_handle v2)
    {
        BOOST_ASSERT( v1 != m_vertices.end() );
        BOOST_ASSERT( v2 != m_vertices.end() );
        v1->add_neighbor(v2);
        v2->add_neighbor(v1);
    }

    bool has_cycles() const
    {
        for (vertex_handle it = m_vertices.begin();
             it != m_vertices.end(); ++it)
        {
            it->visited(false);
            it->initialize_parent();
        }

        for (vertex_handle it = m_vertices.begin();
             it != m_vertices.end(); ++it)
        {
            if ( !it->visited() && has_cycles(it) )
            {
                return true;
            }
        }
        return false;
    }

    template <typename OStream, typename V>
    friend inline
    void debug_print_complement_graph(OStream&, complement_graph<V> const&);

private:
    std::size_t m_num_rings, m_num_turns;
    vertex_container m_vertices;
};


}} // namespace detail::is_valid

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_IS_VALID_COMPLEMENT_GRAPH_HPP
