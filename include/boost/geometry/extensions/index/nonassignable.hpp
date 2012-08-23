// Boost.Geometry Index
//
// Spatial query predicates
//
// Copyright (c) 2011-2012 Adam Wulkiewicz, Lodz, Poland.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_NONASSIGNABLE_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_NONASSIGNABLE_HPP

namespace boost { namespace geometry { namespace index {

class nonassignable
{
    nonassignable & operator=(nonassignable const&);
};

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_NONASSIGNABLE_HPP
