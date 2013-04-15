// Boost.Geometry Index
//
// R-tree spatial query visitor implementation
//
// Copyright (c) 2011-2013 Adam Wulkiewicz, Lodz, Poland.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_INDEX_DETAIL_RTREE_VISITORS_SPATIAL_QUERY_HPP
#define BOOST_GEOMETRY_INDEX_DETAIL_RTREE_VISITORS_SPATIAL_QUERY_HPP

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree { namespace visitors {

template <typename Value, typename Options, typename Translator, typename Box, typename Allocators, typename Predicates, typename OutIter>
struct spatial_query
    : public rtree::visitor<Value, typename Options::parameters_type, Box, Allocators, typename Options::node_tag, true>::type
{
    typedef typename rtree::node<Value, typename Options::parameters_type, Box, Allocators, typename Options::node_tag>::type node;
    typedef typename rtree::internal_node<Value, typename Options::parameters_type, Box, Allocators, typename Options::node_tag>::type internal_node;
    typedef typename rtree::leaf<Value, typename Options::parameters_type, Box, Allocators, typename Options::node_tag>::type leaf;

    typedef typename Allocators::size_type size_type;

    static const unsigned predicates_len = index::detail::predicates_length<Predicates>::value;

    inline spatial_query(Translator const& t, Predicates const& p, OutIter out_it)
        : tr(t), pred(p), out_iter(out_it), found_count(0)
    {}

    inline void operator()(internal_node const& n)
    {
        typedef typename rtree::elements_type<internal_node>::type elements_type;
        elements_type const& elements = rtree::elements(n);

        // traverse nodes meeting predicates
        for (typename elements_type::const_iterator it = elements.begin();
            it != elements.end(); ++it)
        {
            // if node meets predicates
            // 0 - dummy value
            if ( index::detail::predicates_check<index::detail::bounds_tag, 0, predicates_len>(pred, 0, it->first) )
                rtree::apply_visitor(*this, *it->second);
        }
    }

    inline void operator()(leaf const& n)
    {
        typedef typename rtree::elements_type<leaf>::type elements_type;
        elements_type const& elements = rtree::elements(n);

        // get all values meeting predicates
        for (typename elements_type::const_iterator it = elements.begin();
            it != elements.end(); ++it)
        {
            // if value meets predicates
            if ( index::detail::predicates_check<index::detail::value_tag, 0, predicates_len>(pred, *it, tr(*it)) )
            {
                out_iter = *it;
                ++out_iter;

                ++found_count;
            }
        }
    }

    Translator const& tr;

    Predicates pred;

    OutIter out_iter;
    size_type found_count;
};

//template <typename Value, typename Options, typename Translator, typename Box, typename Allocators, typename Predicates, typename OutIter>
//struct spatial_query_incremental
//    : public rtree::visitor<Value, typename Options::parameters_type, Box, Allocators, typename Options::node_tag, true>::type
//{
//    typedef typename rtree::node<Value, typename Options::parameters_type, Box, Allocators, typename Options::node_tag>::type node;
//    typedef typename rtree::internal_node<Value, typename Options::parameters_type, Box, Allocators, typename Options::node_tag>::type internal_node;
//    typedef typename rtree::leaf<Value, typename Options::parameters_type, Box, Allocators, typename Options::node_tag>::type leaf;
//
//    typedef typename Allocators::size_type size_type;
//    typedef typename Allocators::node_pointer node_pointer;
//
//    static const unsigned predicates_len = index::detail::predicates_length<Predicates>::value;
//
//    inline spatial_query_incremental(Translator const& t, Predicates const& p)
//        : tr(t), pred(p)
//    {}
//
//    inline void operator()(internal_node const& n)
//    {
//        typedef typename rtree::elements_type<internal_node>::type elements_type;
//        elements_type const& elements = rtree::elements(n);
//
//        internal_stack.push_back(std::make_pair(elements.begin(), elements.end()));
//
//        //// if node meets predicates
//        //// 0 - dummy value
//        //if ( index::detail::predicates_check<index::detail::bounds_tag, 0, predicates_len>(pred, 0, internal_stack.back().first->first) )
//        //{
//        //    nodes.push_back(it->second);
//        //    rtree::apply_visitor(*this, *it->second);
//        //}
//    }
//
//    inline void operator()(leaf const& n)
//    {
//        typedef typename rtree::elements_type<leaf>::type elements_type;
//        elements_type const& elements = rtree::elements(n);
//
//        leaf_range.push_back(std::make_pair(elements.begin(), elements.end()));
//
//        //// if value meets predicates
//        //if ( index::detail::predicates_check<index::detail::value_tag, 0, predicates_len>(pred, *it, tr(*it)) )
//        //{
//        //    out_iter = *it;
//        //    ++out_iter;
//
//        //    ++found_count;
//        //}
//    }
//
//    Translator const& tr;
//    Predicates pred;
//
//    typedef typename rtree::elements_type<internal_node>::type::const_iterator internal_iterator;
//    typedef typename rtree::elements_type<leaf>::type::const_iterator leaf_iterator;
//
//    std::vector< std::pair<internal_iterator, internal_iterator> > internal_stack;
//    std::pair<leaf_iterator, leaf_iterator> leaf_range;
//};

}}} // namespace detail::rtree::visitors

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_INDEX_DETAIL_RTREE_VISITORS_SPATIAL_QUERY_HPP
