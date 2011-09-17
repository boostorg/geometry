// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.Index - Abs of difference
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_ALGORITHMS_DETAIL_DIFF_ABS_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_ALGORITHMS_DETAIL_DIFF_ABS_HPP

namespace boost { namespace geometry { namespace index {

namespace detail {

template <typename T>
inline T diff_abs(T const& v1, T const& v2)
{
    return v1 < v2 ? v2 - v1 : v1 - v2;
}

} // namespace detail

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_ALGORITHMS_DETAIL_DIFF_ABS_HPP
