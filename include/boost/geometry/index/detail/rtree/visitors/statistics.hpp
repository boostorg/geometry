// Boost.Geometry Index
//
// R-tree visitor collecting basic statistics
//
// Copyright (c) 2011-2013 Adam Wulkiewicz, Lodz, Poland.
// Copyright (c) 2013 Mateusz Loskot, London, UK.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_INDEX_DETAIL_RTREE_VISITORS_STATISTICS_HPP
#define BOOST_GEOMETRY_INDEX_DETAIL_RTREE_VISITORS_STATISTICS_HPP

#include <boost/geometry/index/detail/indexable.hpp>
#include <algorithm>
#include <tuple>

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree { namespace visitors {

template <typename Value, typename Options, typename Translator, typename Box, typename Allocators>
struct statistics : public rtree::visitor<Value, typename Options::parameters_type, Box, Allocators, typename Options::node_tag, true>::type
{
    typedef typename rtree::internal_node<Value, typename Options::parameters_type, Box, Allocators, typename Options::node_tag>::type internal_node;
    typedef typename rtree::leaf<Value, typename Options::parameters_type, Box, Allocators, typename Options::node_tag>::type leaf;

    inline statistics()
        : level(0)
        , levels(1) // count root
        , nodes(0)
        , leaves(0)
        , values(0)
        , values_min(0)
        , values_max(0)
    {}

    inline void operator()(internal_node const& n)
    {
        typedef typename rtree::elements_type<internal_node>::type elements_type;
        elements_type const& elements = rtree::elements(n);
        
        ++nodes; // count node

        size_t const level_backup = level;
        ++level;

        levels += level++ > levels ? 1 : 0; // count level (root already counted)
                
        for (typename elements_type::const_iterator it = elements.begin();
            it != elements.end(); ++it)
        {
            rtree::apply_visitor(*this, *it->second);
        }
        
        level = level_backup;
    }

    inline void operator()(leaf const& n)
    {   
        typedef typename rtree::elements_type<leaf>::type elements_type;
        elements_type const& elements = rtree::elements(n);

        ++leaves; // count leaves
        
        std::size_t const v = elements.size();
        // count values spread per node and total
        values_min = (std::min)(values_min == 0 ? v : values_min, v);
        values_max = (std::max)(values_max, v);
        values += v;
    }
    
    std::size_t level;
    std::size_t levels;
    std::size_t nodes;
    std::size_t leaves;
    std::size_t values;
    std::size_t values_min;
    std::size_t values_max;
};

}}} // namespace detail::rtree::visitors

template <typename Value, typename Options, typename IndexableGetter, typename EqualTo, typename Allocator>
std::tuple<std::size_t, std::size_t, std::size_t, std::size_t, std::size_t, std::size_t>
    statistics(rtree<Value, Options, IndexableGetter, EqualTo, Allocator> const& tree)
{
    typedef rtree<Value, Options, IndexableGetter, EqualTo, Allocator> rtree_type;

    typedef typename rtree_type::value_type value_type;
    typedef typename rtree_type::options_type options_type;
    typedef typename rtree_type::translator_type translator_type;
    typedef typename rtree_type::box_type box_type;
    typedef typename rtree_type::allocators_type allocators_type;

    detail::rtree::visitors::statistics
        <
        value_type, options_type, translator_type, box_type, allocators_type
        > stats_v;

    tree.apply_visitor(stats_v);
    
    return std::make_tuple(stats_v.levels, stats_v.nodes, stats_v.leaves, stats_v.values, stats_v.values_min, stats_v.values_max);
}

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_INDEX_DETAIL_RTREE_VISITORS_STATISTICS_HPP
