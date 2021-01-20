// Boost.Geometry

// Copyright (c) 2020 Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_UTIL_IS_GEOMETRY_TYPE_HPP
#define BOOST_GEOMETRY_UTIL_IS_GEOMETRY_TYPE_HPP

namespace boost { namespace geometry
{

template <typename Geometry>
struct is_polygon
{
    static const bool value = boost::is_same
        <
            typename tag<Geometry>::type,
            polygon_tag
        >::type::value;
};


template <typename Geometry>
struct is_multi_linestring
{
    static const bool value = boost::is_same
        <
            typename tag<Geometry>::type,
            multi_linestring_tag
        >::type::value;
};


template <typename Geometry>
struct is_multi_polygon
{
    static const bool value = boost::is_same
        <
            typename tag<Geometry>::type,
            multi_polygon_tag
        >::type::value;
};

template <typename Geometry>
struct is_ring
{
    static const bool value = boost::is_same
        <
            typename tag<Geometry>::type,
            ring_tag
        >::type::value;
};

template <typename Geometry>
struct is_box
{
    static const bool value = boost::is_same
        <
            typename tag<Geometry>::type,
            box_tag
        >::type::value;
};


} } // namespace boost::geometry

#endif //BOOST_GEOMETRY_UTIL_IS_GEOMETRY_TYPE_HPP
