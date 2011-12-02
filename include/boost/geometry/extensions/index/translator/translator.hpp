// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.SpatialIndex - translators
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_TRANSLATOR_TRANSLATOR_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_TRANSLATOR_TRANSLATOR_HPP

#include <boost/geometry/extensions/index/translator/def.hpp>
#include <boost/geometry/extensions/index/translator/index.hpp>

namespace boost { namespace geometry { namespace index { namespace translator {

template <typename Translator>
struct indexable_type
{
    typedef typename boost::remove_const<
        typename boost::remove_reference<
            typename Translator::result_type
        >::type
    >::type type;
};

}}}} // namespace boost::geometry::index::translator

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_TRANSLATOR_TRANSLATOR_HPP
