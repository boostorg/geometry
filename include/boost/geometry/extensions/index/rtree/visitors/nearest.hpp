// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.Index - R-tree k nearest neighbors query
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_NEAREST_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_NEAREST_HPP

#include <boost/geometry/extensions/index/algorithms/mindist.hpp>
#include <boost/geometry/extensions/index/algorithms/minmaxdist.hpp>
#include <boost/geometry/extensions/index/algorithms/maxdist.hpp>

#include <boost/geometry/extensions/index/distance_calc.hpp>

#include <boost/geometry/extensions/index/rtree/node/node.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree { namespace visitors {

// TODO: awulkiew - maby it's a good idea to check if curr_mindist < comp_mindist and then check predicates
// in store() or break store to 2 functions e.g. should_store() and store()
// - well not with this algorithm of storing k-th neighbor

template <typename Value, typename Translator, typename Point>
struct nearest_one
{
public:
    typedef typename Translator::indexable_type indexable_type;
    typedef typename geometry::default_distance_result<Point, indexable_type>::type distance_type;

    inline nearest_one()
        : comp_dist(std::numeric_limits<distance_type>::max())
    {}

    inline void store(Value const& val, distance_type const& curr_comp_dist)
    {
        if ( curr_comp_dist < comp_dist )
        {
            comp_dist = curr_mindist;
            value = val;
        }
    }

    inline bool is_comparable_distance_valid() const
    {
        return comp_dist < std::numeric_limits<distance_type>::max();
    }

    inline distance_type comparable_distance() const
    {
        return comp_dist;
    }

    inline size_t get(Value & v)
    {
        v = value;
        return is_comparable_distance_valid() ? 1 : 0;
    }

private:
    Value value;
    distance_type comp_dist;
};

template <typename Value, typename Translator, typename Point>
struct nearest_k
{
public:
    typedef typename Translator::indexable_type indexable_type;
    typedef typename geometry::default_distance_result<Point, indexable_type>::type distance_type;

    inline explicit nearest_k(size_t k)
        : m_count(k)
    {
        // TEMP?
        m_neighbors.reserve(m_count + 1);
    }

    inline void store(Value const& val, distance_type const& curr_comp_dist)
    {
        m_neighbors.push_back(std::make_pair(curr_comp_dist, val));
        std::sort(m_neighbors.begin(), m_neighbors.end(), neighbors_less);

        if ( m_count < m_neighbors.size() )
            m_neighbors.pop_back();

        // TODO: awulkiew - test other methods:
        // heap, manual inserting
        // don't sort if size < k ?
        // check the furthest distance at the first place, before push_back()
    }

    inline bool is_comparable_distance_valid() const
    {
        return m_neighbors.size() == m_count;
    }

    inline distance_type comparable_distance() const
    {
        return m_neighbors.size() < m_count ?
            std::numeric_limits<distance_type>::max() :
            m_neighbors.back().first;
    }

    template <typename OutIter>
    inline size_t get(OutIter & out_it)
    {
        typedef typename std::vector< std::pair<distance_type, Value> >::const_iterator neighbors_iterator;
        for ( neighbors_iterator it = m_neighbors.begin() ; it != m_neighbors.end() ; ++it )
            *out_it = it->second;

        return m_neighbors.size();
    }

private:
    inline static bool neighbors_less(
        std::pair<distance_type, Value> const& p1,
        std::pair<distance_type, Value> const& p2)
    {
        return p1.first < p2.first;
    }

    size_t m_count;
    std::vector< std::pair<distance_type, Value> > m_neighbors;
};

template <
    typename Value,
    typename Options,
    typename Translator,
    typename Box,
    typename PointData,
    typename Predicates,
    typename Result
>
class nearest
    : public rtree::visitor<Value, typename Options::parameters_type, Box, typename Options::node_tag, true>::type
    , index::nonassignable
{
public:
    typedef typename rtree::node<Value, typename Options::parameters_type, Box, typename Options::node_tag>::type node;
    typedef typename rtree::internal_node<Value, typename Options::parameters_type, Box, typename Options::node_tag>::type internal_node;
    typedef typename rtree::leaf<Value, typename Options::parameters_type, Box, typename Options::node_tag>::type leaf;

    typedef typename geometry::default_distance_result<PointData, Box>::type node_distance_type;

    inline nearest(Translator const& t, PointData const& point_data, Predicates const& pr, Result & r)
        : m_tr(t), m_point_data(point_data), m_pred(pr)
        , m_result(r)
    {}

    //TODO: awulkiew - check this approach: store one, global vector of active branches, add branches only if mindist is ok

    inline void operator()(internal_node const& n)
    {
        // array of active nodes
        /*index::pushable_array<
            std::pair<node_distance_type, const node *>,
            Options::parameters_type::max_elements
        > active_branch_list;*/
        std::vector< std::pair<node_distance_type, const node *> > active_branch_list;

        typedef typename rtree::elements_type<internal_node>::type elements_type;
        elements_type const& elements = rtree::elements(n);

        // fill array of nodes meeting predicates
        for (typename elements_type::const_iterator it = elements.begin();
            it != elements.end(); ++it)
        {
            if ( index::predicates_check<rtree::node_predicates_tag>(m_pred, it->first) )
            {
                active_branch_list.push_back(
                    std::make_pair(index::mindist(m_point_data, it->first), it->second)
                );
            }
        }

        // if there aren't any nodes in ABL - return
        if ( active_branch_list.empty() )
            return;
        
        // sort array
        std::sort(active_branch_list.begin(), active_branch_list.end(), abl_mindist_less);

        // recursively visit nodes
        for ( size_t i = 0 ;; ++i )
        {
            // prune nodes
            prune_nodes(active_branch_list);

            if ( active_branch_list.size() <= i )
                break;

            rtree::apply_visitor(*this, *active_branch_list[i].second);
        }
    }

    inline void operator()(leaf const& n)
    {
        typedef typename rtree::elements_type<leaf>::type elements_type;
        elements_type const& elements = rtree::elements(n);
        
        // search leaf for closest value meeting predicates
        for (typename elements_type::const_iterator it = elements.begin();
            it != elements.end(); ++it)
        {
            if ( index::predicates_check<rtree::value_predicates_tag>(m_pred, m_tr(*it)) )
            {
                // store value
                m_result.store(*it, index::mindist(m_point_data, m_tr(*it)));
            }
        }
    }

private:
    inline static bool abl_mindist_less(
        std::pair<node_distance_type, const node *> const& p1,
        std::pair<node_distance_type, const node *> const& p2)
    {
        return p1.first < p2.first;
    }

    template <typename ActiveBranchList>
    inline void prune_nodes(ActiveBranchList & abl) const
    {
        // if some value were found
        if ( m_result.is_comparable_distance_valid() )
        {
            // prune if box's mindist is further than value's mindist
            while ( !abl.empty() &&
                    m_result.comparable_distance() < abl.back().first )
            {
                abl.pop_back();
            }
        }
    }

    Translator const& m_tr;
    PointData const& m_point_data;
    Predicates const& m_pred;

    Result & m_result;
};

}}} // namespace detail::rtree::visitors

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_NEAREST_HPP
