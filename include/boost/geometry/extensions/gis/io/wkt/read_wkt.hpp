// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2011, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_GIS_IO_WKT_READ_WKT_HPP
#define BOOST_GEOMETRY_EXTENSIONS_GIS_IO_WKT_READ_WKT_HPP


#if defined(BOOST_MSVC_FULL_VER)
#pragma message ("WARNING: wkt is moved to boost/geometry/domains/gis/io/wkt, please update include file")
#else
#warning "wkt is moved to boost/geometry/domains/gis/io/wkt, please update include file"
#endif


#include <boost/geometry/domains/gis/io/wkt/read_wkt.hpp>


namespace boost { namespace geometry
{


/*!
\brief Parses OGC Well-Known Text (\ref WKT) and outputs using an output iterator
\ingroup wkt
\param wkt string containing \ref WKT
\param out output iterator
\note This function is deprecated!
\note Because the output iterator doesn't always have the type value_type, it should be
specified in the function call.
\par Example:
Small example showing how to use read_wkt with an output iterator
\dontinclude doxygen_1.cpp
\skip example_from_wkt_output_iterator
\line {
\until }
*/
template <typename Geometry, typename OutputIterator>
inline void read_wkt(std::string const& wkt, OutputIterator out)
{
    geometry::concept::check<Geometry>();

    typedef typename point_type<Geometry>::type point_type;

    std::string const& tag =
        geometry_id<Geometry>::value == 2 ? "linestring" : "polygon";

    detail::wkt::tokenizer tokens(wkt, boost::char_separator<char>(" ", ",()"));
    detail::wkt::tokenizer::iterator it;
    if (detail::wkt::initialize<point_type>(tokens, tag, wkt, it))
    {
        detail::wkt::container_inserter<point_type>::apply(it, tokens.end(), wkt, out);
    }
}

}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_EXTENSIONS_GIS_IO_WKT_READ_WKT_HPP
