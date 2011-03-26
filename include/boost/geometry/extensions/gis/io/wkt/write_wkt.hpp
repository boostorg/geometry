// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2011 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2011 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2011 Mateusz Loskot, London, UK.

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_GIS_IO_WKT_WRITE_WKT_HPP
#define BOOST_GEOMETRY_EXTENSIONS_GIS_IO_WKT_WRITE_WKT_HPP


#if defined(BOOST_MSVC_FULL_VER)
#pragma message ("WARNING: wkt is moved to boost/geometry/domains/gis/io/wkt, please update include file")
#else
#warning "wkt is moved to boost/geometry/domains/gis/io/wkt, please update include file"
#endif


#include <boost/geometry/domains/gis/io/wkt/write_wkt.hpp>



namespace boost { namespace geometry
{

// Backward compatibility - NOTE THIS IS DEPRECATED
template <typename Geometry>
inline wkt_manipulator<Geometry> make_wkt(Geometry const& geometry)
{
    concept::check<Geometry const>();

    return wkt_manipulator<Geometry>(geometry);
}

}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_EXTENSIONS_GIS_IO_WKT_WRITE_WKT_HPP
