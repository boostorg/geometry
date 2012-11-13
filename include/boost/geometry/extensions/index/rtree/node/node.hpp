// Boost.Geometry Index
//
// R-tree nodes
//
// Copyright (c) 2011-2012 Adam Wulkiewicz, Lodz, Poland.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_NODE_NODE_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_NODE_NODE_HPP

#include <boost/geometry/extensions/index/rtree/node/concept.hpp>

// WARNING!
// The Node elements containing pointers to nodes, i.e. pair<Box, node*> MUST NOT throw an exception
// in the copy constructor. Otherwise copying may be broken in push_back() of internal vectors.
// The result may be two copies of the same pointer in the vector. This may cause the attempt to destroy
// the same object two times.
// This means for example that Value's CoordinateType copy constructor MUST NOT throw an exception
// because Value's CoordinateType is used in Box type.

#include <boost/geometry/extensions/index/rtree/node/node_d_mem_dynamic.hpp>
#include <boost/geometry/extensions/index/rtree/node/node_d_mem_static.hpp>

#include <boost/geometry/extensions/index/rtree/node/node_s_mem_dynamic.hpp>
#include <boost/geometry/extensions/index/rtree/node/node_s_mem_static.hpp>

#include <boost/geometry/extensions/index/rtree/node/node_auto_ptr.hpp>

#include <boost/geometry/algorithms/expand.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree {

// elements box

template <typename Box, typename FwdIter, typename Translator>
inline Box elements_box(FwdIter first, FwdIter last, Translator const& tr)
{
    BOOST_GEOMETRY_INDEX_ASSERT(first != last, "Can't calculate element's box");

    Box result;

    geometry::convert(element_indexable(*first, tr), result);
    ++first;

    for ( ; first != last ; ++first )
        geometry::expand(result, element_indexable(*first, tr));

    return result;
}

// destroys subtree if the element is internal node's element
template <typename Value, typename Options, typename Translator, typename Box, typename Allocators>
struct destroy_element
{
    typedef typename Options::parameters_type parameters_type;

    typedef typename rtree::internal_node<Value, parameters_type, Box, Allocators, typename Options::node_tag>::type internal_node;
    typedef typename rtree::leaf<Value, parameters_type, Box, Allocators, typename Options::node_tag>::type leaf;

    typedef rtree::node_auto_ptr<Value, Options, Translator, Box, Allocators> node_auto_ptr;

    inline static void apply(typename internal_node::elements_type::value_type & element, Allocators & allocators)
    {
         node_auto_ptr dummy(element.second, allocators);
         element.second = 0;
    }

    inline static void apply(typename leaf::elements_type::value_type &, Allocators &) {}
};

// destroys stored subtrees if internal node's elements are passed
template <typename Value, typename Options, typename Translator, typename Box, typename Allocators>
struct destroy_elements
{
    typedef typename Options::parameters_type parameters_type;

    typedef typename rtree::internal_node<Value, parameters_type, Box, Allocators, typename Options::node_tag>::type internal_node;
    typedef typename rtree::leaf<Value, parameters_type, Box, Allocators, typename Options::node_tag>::type leaf;

    typedef rtree::node_auto_ptr<Value, Options, Translator, Box, Allocators> node_auto_ptr;

    inline static void apply(typename internal_node::elements_type & elements, Allocators & allocators)
    {
        for ( size_t i = 0 ; i < elements.size() ; ++i )
        {
            node_auto_ptr dummy(elements[i].second, allocators);
            elements[i].second = 0;
        }
    }

    inline static void apply(typename leaf::elements_type &, Allocators &)
    {}

    inline static void apply(typename internal_node::elements_type::iterator first,
                             typename internal_node::elements_type::iterator last,
                             Allocators & allocators)
    {
        for ( ; first != last ; ++first )
        {
            node_auto_ptr dummy(first->second, allocators);
            first->second = 0;
        }
    }

    inline static void apply(typename leaf::elements_type::iterator first,
                             typename leaf::elements_type::iterator last,
                             Allocators &)
    {}
};

}} // namespace detail::rtree

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_NODE_NODE_HPP
