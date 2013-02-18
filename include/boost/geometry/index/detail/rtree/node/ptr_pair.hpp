// Boost.Geometry Index
//
// Movable-only pair intended to be used internally in nodes.
// It may be used to provide exclusive ownership of node pointers
//
// Copyright (c) 2011-2013 Adam Wulkiewicz, Lodz, Poland.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_INDEX_DETAIL_RTREE_NODE_PTR_PAIR_HPP
#define BOOST_GEOMETRY_INDEX_DETAIL_RTREE_NODE_PTR_PAIR_HPP

#include <boost/move/move.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree {

template <typename First, typename Pointer>
class ptr_pair
{
    BOOST_MOVABLE_BUT_NOT_COPYABLE(ptr_pair)
public:
    typedef First first_type;
    typedef Pointer second_type;
    ptr_pair(First const& f, Pointer s) : first(f), second(s) {}

// INFO - members aren't really moved!
    ptr_pair(BOOST_RV_REF(ptr_pair) p) : first(p.first), second(p.second) { p.second = 0; }
    ptr_pair & operator=(BOOST_RV_REF(ptr_pair) p) { first = p.first; second = p.second; p.second = 0; return *this; }

    first_type first;
    second_type second;
};

template <typename First, typename Pointer> inline
ptr_pair<First, Pointer>
make_ptr_pair(First const& f, Pointer s)
{
    return ptr_pair<First, Pointer>(f, s);
}

}} // namespace detail::rtree

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_INDEX_DETAIL_RTREE_NODE_PTR_PAIR_HPP
