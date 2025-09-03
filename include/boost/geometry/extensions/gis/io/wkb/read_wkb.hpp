// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2009-2012 Mateusz Loskot, London, UK.

// This file was modified by Oracle on 2020.
// Modifications copyright (c) 2020, Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_IO_WKB_READ_WKB_HPP
#define BOOST_GEOMETRY_IO_WKB_READ_WKB_HPP

#include <cstdint>
#include <iterator>
#include <type_traits>

#include <boost/geometry/core/tags.hpp>
#include <boost/geometry/extensions/gis/io/wkb/detail/parser.hpp>

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template <typename Tag, typename G>
struct read_wkb {};

template <typename Geometry>
struct read_wkb<point_tag, Geometry>
{
    template <typename Iterator>
    static inline bool parse(Iterator& it, Iterator end, Geometry& geometry,
        detail::wkb::byte_order_type::enum_t order)
    {
        return detail::wkb::point_parser<Geometry>::parse(it, end, geometry, order);
    }
};

template <typename Geometry>
struct read_wkb<linestring_tag, Geometry>
{
    template <typename Iterator>
    static inline bool parse(Iterator& it, Iterator end, Geometry& geometry,
        detail::wkb::byte_order_type::enum_t order)
    {
        geometry::clear(geometry);
        return detail::wkb::linestring_parser<Geometry>::parse(it, end, geometry, order);
    }
};

template <typename Geometry>
struct read_wkb<polygon_tag, Geometry>
{
    template <typename Iterator>
    static inline bool parse(Iterator& it, Iterator end, Geometry& geometry,
        detail::wkb::byte_order_type::enum_t order)
    {
        geometry::clear(geometry);
        return detail::wkb::polygon_parser<Geometry>::parse(it, end, geometry, order);
    }
};

} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


template <typename Iterator, typename Geometry>
inline bool read_wkb(Iterator begin, Iterator end, Geometry& geometry)
{
    // Stream of bytes can only be parsed using random access iterator.
    static_assert(
        std::is_convertible
            <
                typename std::iterator_traits<Iterator>::iterator_category,
                const std::random_access_iterator_tag&
            >::value,
        "Iterator must be random access");

    detail::wkb::byte_order_type::enum_t byte_order;
    if (detail::wkb::byte_order_parser::parse(begin, end, byte_order))
    {
        return dispatch::read_wkb
            <
            typename tag<Geometry>::type,
            Geometry
            >::parse(begin, end, geometry, byte_order);
    }

    return false;
}

template <typename ByteType, typename Geometry>
inline bool read_wkb(ByteType const* bytes, std::size_t length, Geometry& geometry)
{
    static_assert(std::is_integral<ByteType>::value, "ByteType must be integral");
    static_assert(sizeof(std::uint8_t) == sizeof(ByteType),
                  "Size of ByteType must match size of std::uint8_t");

    ByteType const* begin = bytes;
    ByteType const* const end = bytes + length;

    return read_wkb(begin, end, geometry);
}


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_IO_WKB_READ_WKB_HPP
