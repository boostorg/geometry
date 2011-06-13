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

template <typename Value, typename Options, typename Translator>
class rtree;

namespace filters {

template <typename Value, typename Options, typename Translator>
class spatial_filter< index::rtree<Value, Options, Translator> >
{
public:
    typedef typename std::deque<Value>::iterator iterator;
    typedef typename std::deque<Value>::const_iterator const_iterator;
    
    template <typename Geometry>
    inline spatial_filter(index::rtree<Value, Options, Translator> const& rtree, Geometry const& geom)
    {
        rtree.find(geom, std::back_inserter(m_result));
    }

    inline iterator begin() { return m_result.begin(); }
    inline iterator end() { return m_result.end(); }
    inline const_iterator begin() const { return m_result.begin(); }
    inline const_iterator end() const { return m_result.end(); }

private:
    std::deque<Value> m_result;
};

} // namespace filters

// TODO: awulkiew - filter may be implemented in operator|

//template<typename Value, typename Translator, typename Tag, typename Geometry>
//std::deque<Value>
//operator|(
//    index::rtree<Value, Translator, Tag> const& si,
//    index::filters::detail::spatially_filtered<Geometry> const& f)
//{
//    std::deque<Value> result;
//    si.find(f.geometry(), std::back_inserter(result));
//    return result;
//}

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_FILTERS_HPP
