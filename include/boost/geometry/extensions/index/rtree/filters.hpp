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

#include <boost/geometry/extensions/index/filters/query_filter.hpp>
#include <boost/geometry/extensions/index/filters/nearest_filter.hpp>

namespace boost { namespace geometry { namespace index {

template <typename Value, typename Options, typename Translator>
class rtree;

template <typename Value, typename Options, typename Translator>
class query_filter< index::rtree<Value, Options, Translator> >
{
public:
    typedef std::vector<Value> result_type;
    typedef typename result_type::iterator iterator;
    typedef typename result_type::const_iterator const_iterator;
    
    template <typename Predicates>
    inline query_filter(
        index::rtree<Value, Options, Translator> const& rtree,
        Predicates const& pred
    )
    {
        rtree.query(pred, std::back_inserter(m_result));
    }

    inline iterator begin() { return m_result.begin(); }
    inline iterator end() { return m_result.end(); }
    inline const_iterator begin() const { return m_result.begin(); }
    inline const_iterator end() const { return m_result.end(); }

private:
    result_type m_result;
};

template <typename Value, typename Options, typename Translator>
class nearest_filter< index::rtree<Value, Options, Translator> >
{
public:
    typedef std::vector<Value> result_type;
    typedef typename result_type::iterator iterator;
    typedef typename result_type::const_iterator const_iterator;

    template <typename DistancesPredicates, typename Predicates>
    inline nearest_filter(
        index::rtree<Value, Options, Translator> const& rtree,
        DistancesPredicates const& dpred,
        size_t k,
        Predicates const& pred
    )
    {
        rtree.nearest(dpred, k, pred, std::back_inserter(m_result));
    }

    inline iterator begin() { return m_result.begin(); }
    inline iterator end() { return m_result.end(); }
    inline const_iterator begin() const { return m_result.begin(); }
    inline const_iterator end() const { return m_result.end(); }

private:
    result_type m_result;
};

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_FILTERS_HPP
