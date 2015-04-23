// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2009-2012 Mateusz Loskot, London, UK.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_IO_WKB_DETAIL_OGC_HPP
#define BOOST_GEOMETRY_IO_WKB_DETAIL_OGC_HPP

#include <boost/cstdint.hpp>

namespace boost { namespace geometry
{

// The well-known binary representation for OGC geometry (WKBGeometry),
// provides a portable representation of a geometry value as a contiguous
// stream of bytes. It permits geometry values to be exchanged between
// a client application and an SQL database in binary form.
//
// Basic Type definitions
// byte : 1 byte
// uint32 : 32 bit unsigned integer (4 bytes)
// double : double precision number (8 bytes)
//
// enum wkbByteOrder
// {
//   wkbXDR = 0, // Big Endian
//   wkbNDR = 1  // Little Endian
// };
//
// enum wkbGeometryType
// {
//   wkbPoint = 1,
//   wkbLineString = 2,
//   wkbPolygon = 3,
//   wkbMultiPoint = 4,
//   wkbMultiLineString = 5,
//   wkbMultiPolygon = 6,
//   wkbGeometryCollection = 7
// };

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace wkb
{

// TODO: Replace 'struct' with scoped enum from <boost/detail/scoped_enum_emulation.hpp>
// For older Boost, copy
// <boost/spirit/home/support/detail/scoped_enum_emulation.hpp>
// to
// <boost/geometry/detail/scoped_enum_emulation.hpp>
// and use it.

struct byte_order_type
{
    enum enum_t
    {
        xdr     = 0, // wkbXDR, bit-endian
        ndr     = 1, // wkbNDR, little-endian
        unknown = 2  // not defined by OGC
    };
};

struct geometry_type_ogc
{
    enum enum_t
    {
        point      = 1,
        linestring = 2,
        polygon    = 3

        // TODO: Not implemented
        //multipoint = 4,
        //multilinestring = 5,
        //multipolygon = 6,
        //collection = 7
    };
};

struct geometry_type_ewkt
{
    enum enum_t
    {
        point      = 1,
        linestring = 2,
        polygon    = 3,

        // TODO: Not implemented
        //multipoint = 4,
        //multilinestring = 5,
        //multipolygon = 6,
        //collection = 7


        pointz      = 1001,
        linestringz = 1002,
        polygonz    = 1003
    };
};

struct ogc_policy
{
};

struct ewkt_policy
{
};

template
<
    typename Geometry, 
    typename CheckPolicy = ogc_policy, 
    typename Tag = typename tag<Geometry>::type
>
struct geometry_type : not_implemented<Tag>
{
};

template <typename Geometry, typename CheckPolicy>
struct geometry_type<Geometry, CheckPolicy, point_tag>
{
    static bool check(boost::uint32_t value) 
    { 
        return value == get_impl<dimension<Geometry>::value>(); 
    }

    static boost::uint32_t get() 
    { 
        return get_impl<dimension<Geometry>::value>(); 
    }

private:

    template <int dimension>
    static boost::uint32_t get_impl() 
    {
        return geometry_type_ogc::point;
    }

    template <>
    static boost::uint32_t get_impl<3>()
    {
        return 1000 + geometry_type_ogc::point;
    }
};

template <typename Geometry, typename CheckPolicy>
struct geometry_type<Geometry, CheckPolicy, linestring_tag>
{
    static bool check(boost::uint32_t value) 
    { 
        return value == get_impl<dimension<Geometry>::value>(); 
    }

    static boost::uint32_t get() 
    { 
        return get_impl<dimension<Geometry>::value>(); 
    }

private:

    template <int dimension>
    static boost::uint32_t get_impl() 
    {
        return geometry_type_ogc::linestring;
    }

    template <>
    static boost::uint32_t get_impl<3>()
    {
        return 1000 + geometry_type_ogc::linestring;
    }
};

template <typename Geometry, typename CheckPolicy>
struct geometry_type<Geometry, CheckPolicy, polygon_tag>
{
    static bool check(boost::uint32_t value) 
    { 
        return value == get_impl<dimension<Geometry>::value>(); 
    }

    static boost::uint32_t get() 
    { 
        return get_impl<dimension<Geometry>::value>(); 
    }

private:

    template <int dimension>
    static boost::uint32_t get_impl() 
    {
        return geometry_type_ogc::polygon;
    }

    template <>
    static boost::uint32_t get_impl<3>()
    {
        return 1000 + geometry_type_ogc::polygon;
    }
};

}} // namespace detail::wkb
#endif // DOXYGEN_NO_IMPL

}} // namespace boost::geometry
#endif // BOOST_GEOMETRY_IO_WKB_DETAIL_OGC_HPP
