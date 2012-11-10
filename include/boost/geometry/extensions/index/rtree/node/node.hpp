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

}} // namespace detail::rtree

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_NODE_NODE_HPP
