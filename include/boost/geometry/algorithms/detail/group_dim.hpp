// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014 Oracle and/or its affiliates.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_GROUP_DIM_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_GROUP_DIM_HPP

#include <boost/type_traits/is_base_of.hpp>

#include <boost/geometry/core/tag.hpp>

namespace boost { namespace geometry {

#ifndef DOXYGEN_NO_DETAIL
namespace detail {

template <typename Geometry,
          typename Tag = typename geometry::tag<Geometry>::type,
          bool IsPoint = boost::is_base_of<pointlike_tag, Tag>::value,
          bool IsLinear = boost::is_base_of<linear_tag, Tag>::value,
          bool IsAreal = boost::is_base_of<areal_tag, Tag>::value>
struct group_tag
    : not_implemented<Tag>
{};

template <typename Geometry, typename Tag>
struct group_tag<Geometry, Tag, true, false, false>
{
    typedef pointlike_tag type;
};

template <typename Geometry, typename Tag>
struct group_tag<Geometry, Tag, false, true, false>
{
    typedef linear_tag type;
};

template <typename Geometry, typename Tag>
struct group_tag<Geometry, Tag, false, false, true>
{
    typedef areal_tag type;
};

template <typename Geometry,
          typename GroupTag = typename group_tag<Geometry>::type>
struct group_dim
    : not_implemented<GroupTag>
{};

template <typename Geometry>
struct group_dim<Geometry, pointlike_tag>
{
    static const int value = 0;
};

template <typename Geometry>
struct group_dim<Geometry, linear_tag>
{
    static const int value = 1;
};

template <typename Geometry>
struct group_dim<Geometry, areal_tag>
{
    static const int value = 2;
};

} // namespace detail
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_GROUP_DIM_HPP
