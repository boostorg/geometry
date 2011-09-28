// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.Index - R-tree spatial query
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_QUERY_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_QUERY_HPP

#include <boost/geometry/extensions/index/rtree/node/node.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree { namespace visitors {

template <typename Value, typename Options, typename Translator, typename Box, typename Predicates, typename OutIter>
struct query
    : public rtree::visitor<Value, typename Options::parameters_type, Box, typename Options::node_tag, true>::type
    , index::nonassignable
{
    typedef typename rtree::node<Value, typename Options::parameters_type, Box, typename Options::node_tag>::type node;
    typedef typename rtree::internal_node<Value, typename Options::parameters_type, Box, typename Options::node_tag>::type internal_node;
    typedef typename rtree::leaf<Value, typename Options::parameters_type, Box, typename Options::node_tag>::type leaf;

    inline query(Translator const& t, Predicates const& p, OutIter out_it)
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
            if ( index::predicates_check<rtree::node_tag>(pred, 0, it->first) )
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
            if ( index::predicates_check<rtree::value_tag>(pred, *it, tr(*it)) )
            {
                out_iter = *it;
                ++out_iter;

                ++found_count;
            }
        }
    }

    Translator const& tr;
    Predicates const& pred;
    OutIter out_iter;
    size_t found_count;
};

}}} // namespace detail::rtree::visitors

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_QUERY_HPP
