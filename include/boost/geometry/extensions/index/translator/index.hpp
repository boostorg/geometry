// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.SpatialIndex - container index value to bounding object translation
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_TRANSLATOR_INDEX_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_TRANSLATOR_INDEX_HPP

#include <boost/geometry/extensions/index/translator/helpers.hpp>

namespace boost { namespace geometry { namespace index { namespace translator {

template <typename Container>
class index
{
public:
    typedef typename detail::extract_indexable
        <typename Container::value_type>::type indexable_type;

    explicit index(Container const& c) : m_c(c) {}

    indexable_type const& operator()(size_t i) const
    {
        return detail::extract_indexable
            <typename Container::value_type>::get(m_c[i]);
    }

    /*bool equals(size_t i1, size_t i2) const
    {
    return i1 == i2;
    }*/

private:
    Container const& m_c;
};

}}}} // namespace boost::geometry::index::translator

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_TRANSLATOR_INDEX_HPP
