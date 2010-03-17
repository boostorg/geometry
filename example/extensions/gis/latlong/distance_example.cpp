// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// Distance Example

// This sample demonstrates the use of latlong-points, xy-points,
// calculate distances between latlong points using different formulas,
// calculate distance between points using pythagoras

#include <iostream>

#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/cartesian2d.hpp>

#include <boost/geometry/extensions/gis/latlong/latlong.hpp>

#ifndef NO_PROJECTION
#include <boost/geometry/extensions/gis/projections/proj/sterea.hpp>
#include <boost/geometry/extensions/gis/projections/parameters.hpp>
#endif

int main()
{
    using namespace boost::geometry;

    point_ll_deg a;
    // Amsterdam 52 22'23"N 4 53'32"E
    a.lat(dms<north>(52, 22, 23));
    a.lon(dms<east>(4, 53, 32));

    // Rotterdam 51 55'51"N 4 28'45"E
    point_ll_deg r(latitude<>(dms<north>(51, 55, 51)), longitude<>(dms<east>(4, 28, 45)));

    // The hague: 52 4' 48" N, 4 18' 0" E
    point_ll_deg h(longitude<>(dms<east>(4, 18, 0)), latitude<>(dms<north>(52, 4, 48)));

    point_ll_rad a_rad, r_rad, h_rad;
    transform(a, a_rad);
    transform(r, r_rad);
    transform(h, h_rad);

#ifndef NO_PROJECTION
    projection::sterea_ellipsoid<point_ll_rad, point_2d> proj(projection::init("+lat_0=52.15616055555555 +lon_0=5.38763888888889 +k=0.9999079 +x_0=155000 +y_0=463000 +ellps=bessel +units=m"));

    point_2d a_rd, r_rd, h_rd;
    proj.forward(a_rad, a_rd);
    proj.forward(h_rad, h_rd);
    proj.forward(r_rad, r_rd);
#else
    point_2d a_rd(121267, 487245);
    point_2d r_rd(92526.2, 438324);
    point_2d h_rd(80454.2, 455086);
#endif

    // ------------------------------------------------------------------------------------------
    // Distances
    // ------------------------------------------------------------------------------------------

    std::cout << "Distance Amsterdam-Rotterdam: " << std::endl;
    std::cout << "haversine:              " << 0.001 * distance(a, r) << " km" << std::endl;
    std::cout << "haversine rad:          " << 0.001 * distance(a_rad, r_rad) << " km" << std::endl;
    std::cout << "haversine other radius: " << distance(a, r, strategy::distance::haversine<point_ll_deg>(6371.0) ) << " km" << std::endl;
    std::cout << "andoyer:                " << 0.001 * distance(a, r, strategy::distance::andoyer<point_ll_deg>() ) << " km" << std::endl;
    std::cout << "vincenty:               " << 0.001 * distance(a, r, strategy::distance::vincenty<point_ll_deg>() ) << " km" << std::endl;
    std::cout << "vincenty rad:           " << 0.001 * distance(a_rad, r_rad, strategy::distance::vincenty<point_ll_rad>() ) << " km" << std::endl;
    std::cout << "RD, pythagoras:         " << 0.001 * distance(a_rd, r_rd) << " km" << std::endl;

    std::cout << std::endl;
    std::cout << "Distance Amsterdam-Den Haag: " << std::endl;
    std::cout << "haversine:              " << 0.001 * distance(a, h) << " km" << std::endl;

    // ------------------------------------------------------------------------------------------
    // Distances to segments
    // ------------------------------------------------------------------------------------------
    std::cout << std::endl << "The Hague - line Amsterdam,Rotterdam" << std::endl;

    segment_2d ar_xy(a_rd, r_rd);

    double dr = distance(h_rd, ar_xy);
    std::cout << "in RD: " << 0.001 * dr << std::endl;

    // Temporary fix: multiply with Earth Radius. This will be refactored.
    double const er = 6378137.0;

    dr = er * distance(h, segment_ll_deg(a, r));
    std::cout << "in LL: " << 0.001 * dr << std::endl;

    std::cout << std::endl << "Rotterdam - line Amsterdam,the Hague" << std::endl;
    dr = distance(r_rd, segment_2d(a_rd, h_rd));
    std::cout << "in RD: " << 0.001 * dr << std::endl;
    dr = er * distance(r, segment_ll_deg(a, h));
    std::cout << "in LL: " << 0.001 * dr << std::endl;
    std::cout << std::endl;

    // ------------------------------------------------------------------------------------------
    // Compilation
    // ------------------------------------------------------------------------------------------
    // Next line does not compile because Vincenty cannot work on xy-points
    //std::cout << "vincenty on xy:         " << 0.001 * distance(a_rd, r_rd, formulae::distance::vincenty<>() ) << " km" << std::endl;

    // Next line does not compile because you cannot (yet) assign degree to radian directly
    //point_ll_rad a_rad2 = a;

    // Next line does not compile because you cannot assign latlong to xy
    // point_2d axy = a;

    // ------------------------------------------------------------------------------------------
    // Length
    // ------------------------------------------------------------------------------------------
    // Length calculations use distances internally. The lines below take automatically the default
    // formulae for distance. However, you can also specify a formula explicitly.

    linestring_ll_deg line1;
    append(line1, a);
    append(line1, r);
    std::cout << "length: " << length(line1) << std::endl;
    std::cout << "length using Vincenty: " << length(line1, strategy::distance::vincenty<point_ll_deg>()) << std::endl;

    linestring_2d line2;
    append(line2, a_rd);
    append(line2, r_rd);
    std::cout << "length: " << length(line2) << std::endl;

    return 0;
}
