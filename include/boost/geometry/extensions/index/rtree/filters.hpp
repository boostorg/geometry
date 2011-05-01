// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.SpatialIndex - rtree queries filters implementation
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_FILTERS_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_FILTERS_HPP

#include <deque>
#include <boost/static_assert.hpp>

#include <boost/geometry/extensions/index/filters/spacial_filter.hpp>

// TODO: awulkiew - implement nearest filter
//#include <boost/geometry/extensions/index/filters/nearest_filter.hpp>

namespace boost { namespace geometry { namespace index {

template <typename Value, typename Translator, typename Tag>
class rtree;

namespace filters {

template <typename Value, typename Translator, typename Tag>
class spatial_filter< index::rtree<Value, Translator, Tag> >
{
public:
    typedef typename std::deque<Value>::iterator iterator;
    typedef typename std::deque<Value>::const_iterator const_iterator;

    template <typename Geometry>
    spatial_filter(index::rtree<Value, Translator, Tag> const& rtree, Geometry const& geom)
    {
        rtree.find(geom, std::back_inserter(m_result));
    }

    iterator begin() { return m_result.begin(); }
    iterator end() { return m_result.end(); }
    const_iterator begin() const { return m_result.begin(); }
    const_iterator end() const { return m_result.end(); }

private:
    std::deque<Value> m_result;
};

} // namespace filters

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_FILTERS_HPP
