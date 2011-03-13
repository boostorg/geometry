// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2008-2009, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_GIS_IO_WKT_STREAM_WKT_HPP
#define BOOST_GEOMETRY_EXTENSIONS_GIS_IO_WKT_STREAM_WKT_HPP

#include <boost/geometry/domains/gis/io/wkt/write_wkt.hpp>

// This short file contains only one manipulator, streaming as WKT
// Don't move contents to as_wkt, developers must be able to choose how to stream

// Don't use namespace boost::geometry, to enable the library to stream custom geometries which
// are living outside the namespace boost { namespace geometry

//namespace boost { namespace geometry
//{


/*!
\brief Streams a geometry as Well-Known Text
\ingroup wkt
*/
template<typename Char, typename Traits, typename Geometry>
inline std::basic_ostream<Char,Traits>& operator<<
    (
        std::basic_ostream<Char,Traits> &os,
        Geometry const& geom
    )
{
    os << boost::geometry::wkt(geom);
    return os;
}

//}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_EXTENSIONS_GIS_IO_WKT_STREAM_WKT_HPP
