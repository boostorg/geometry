// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXT_GIS_IO_SHAPELIB_DBF_WRITE_ATTRIBUTE_HPP
#define BOOST_GEOMETRY_EXT_GIS_IO_SHAPELIB_DBF_WRITE_ATTRIBUTE_HPP




// Should be somewhere in your include path
#include "shapefil.h"



namespace boost { namespace geometry
{


template <typename T>
struct DBFFieldType
{
    // IS integer etc.
};

template <> struct DBFFieldType<int> { static ::DBFFieldType const value = FTInteger; };
template <> struct DBFFieldType<float> { static ::DBFFieldType const value = FTDouble; };
template <> struct DBFFieldType<double> { static ::DBFFieldType const value = FTDouble; };
template <> struct DBFFieldType<std::string> { static ::DBFFieldType const value = FTString; };


template <typename T> struct DBFWriteAttribute
{
};

template <> struct DBFWriteAttribute<int>
{
    template <typename T>
    inline static void apply(DBFHandle dbf, int row_index, int field_index, T const& value)
    {
        DBFWriteIntegerAttribute(dbf, row_index, field_index, value);
    }
};

template <> struct DBFWriteAttribute<double>
{
    template <typename T>
    inline static void apply(DBFHandle dbf, int row_index, int field_index, T const& value)
    {
        DBFWriteDoubleAttribute(dbf, row_index, field_index, value);
    }
};

template <> struct DBFWriteAttribute<std::string>
{
    template <typename T>
    inline static void apply(DBFHandle dbf, int row_index, int field_index, T const& value)
    {
        DBFWriteStringAttribute(dbf, row_index, field_index, value);
    }
};

template <int N> 
struct DBFWriteAttribute<char[N]> : DBFWriteAttribute<std::string> {};

template <>
struct DBFWriteAttribute<char*> : DBFWriteAttribute<std::string> {};


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_EXT_GIS_IO_SHAPELIB_DBF_WRITE_ATTRIBUTE_HPP
