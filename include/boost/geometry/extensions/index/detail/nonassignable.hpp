// Boost.Geometry Index
//
// Nonassignable base class.
//
// Copyright (c) 2011-2012 Adam Wulkiewicz, Lodz, Poland.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_NONASSIGNABLE_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_NONASSIGNABLE_HPP

namespace boost { namespace geometry { namespace index { namespace detail {

class nonassignable
{
    nonassignable & operator=(nonassignable const&);
};

}}}} // namespace boost::geometry::index::detail

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_NONASSIGNABLE_HPP
