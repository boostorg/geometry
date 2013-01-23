// Boost.Geometry Index
//
// Query expr
//
// Copyright (c) 2011-2013 Adam Wulkiewicz, Lodz, Poland.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_INDEX_QUERY_EXPR_HPP
#define BOOST_GEOMETRY_INDEX_QUERY_EXPR_HPP

#include <boost/tuple/tuple.hpp>
#include <boost/mpl/assert.hpp>

namespace boost { namespace geometry { namespace index { namespace qe {

// tags

struct intersects_tag {};
struct within_tag {};

struct nearest_tag {};

// predicates

template <typename T, typename Tag>
struct predicate
{
    typedef Tag tag_type;
    predicate(T const& d) : data(d) {}
    T const& data;
};

template <typename T>
struct predicate<T, nearest_tag>
{
    typedef nearest_tag tag_type;
    predicate(T const& d, size_t k) : data(d), count(k) {}
    T const& data;
    size_t count;
};

// predicates generators

template <typename Geometry>
predicate<Geometry, intersects_tag> intersects(Geometry const& g)
{
    return predicate<Geometry, intersects_tag>(g);
}

template <typename DistancePredicates>
predicate<DistancePredicates, nearest_tag> nearest(DistancePredicates const& p, size_t k)
{
    return predicate<DistancePredicates, nearest_tag>(p, k);
}

}}}} // namespace boost::geometry::index::qe

#endif // BOOST_GEOMETRY_INDEX_QUERY_EXPR_HPP
