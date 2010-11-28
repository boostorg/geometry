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
#include <boost/geometry/extensions/gis/geographic/strategies/andoyer.hpp>
#include <boost/geometry/strategies/spherical/distance_cross_track.hpp>

#ifndef NO_PROJECTION
#  include <boost/geometry/extensions/gis/projections/proj/sterea.hpp>
#  include <boost/geometry/extensions/gis/projections/proj/laea.hpp>
#  include <boost/geometry/extensions/gis/projections/parameters.hpp>
#endif

// BSG 28-10-2010
// TODO: celar up this test
// it is more a test than an example
// the results are sometimes WRONG

int main()
{

    using namespace boost::geometry;

    typedef model::point_ll_deg latlon_point;

    latlon_point city1;
    // Amsterdam 52 22'23"N 4 53'32"E
    std::string const city1_name = "Amsterdam";
    city1.lat(dms<north>(52, 22, 23));
    city1.lon(dms<east>(4, 53, 32));

    // Rotterdam 51 55'51"N 4 28'45"E
    // latlon_point city2(latitude<>(dms<north>(51, 55, 51)), longitude<>(dms<east>(4, 28, 45)));
    // Paris 48 52' 0" N, 2 19' 59" E
    std::string const city2_name = "Paris";
    latlon_point city2(latitude<>(dms<north>(48, 52, 0)), longitude<>(dms<east>(2, 19, 59)));

    // The Hague: 52 4' 48" N, 4 18' 0" E
    //latlon_point city3(longitude<>(dms<east>(4, 18, 0)), latitude<>(dms<north>(52, 4, 48)));
    // Barcelona
    std::string const city3_name = "Barcelona";
    latlon_point city3(longitude<>(dms<east>(2, 11, 0)), latitude<>(dms<north>(41, 23, 0)));


    model::point_ll_rad a_rad, r_rad, h_rad;
    transform(city1, a_rad);
    transform(city2, r_rad);
    transform(city3, h_rad);

#ifndef NO_PROJECTION
    /*
    projection::sterea_ellipsoid<model::point_ll_rad, model::point_2d> proj
        (projection::init(
        "+lat_0=52.15616055555555 +lon_0=5.38763888888889 +k=0.9999079 +x_0=155000 +y_0=463000 +ellps=bessel +units=m"));
    */
    projection::laea_ellipsoid<model::point_ll_rad, model::point_2d> proj
        (projection::init(
        " +lat_0=52 +lon_0=10 +x_0=4321000 +y_0=3210000 +ellps=GRS80 +units=m"));


    model::point_2d a_rd, r_rd, h_rd;
    proj.forward(a_rad, a_rd);
    proj.forward(h_rad, h_rd);
    proj.forward(r_rad, r_rd);
#else
    model::point_2d a_rd(121267, 487245);
    model::point_2d r_rd(92526.2, 438324);
    model::point_2d h_rd(80454.2, 455086);
#endif

    // ------------------------------------------------------------------------------------------
    // Distances
    // ------------------------------------------------------------------------------------------

    std::cout << "Distance " << city1_name << "-" << city2_name << ": " << std::endl;
    std::cout << "haversine:              " << 0.001 * distance(city1, city2) << " km" << std::endl;
    std::cout << "haversine rad:          " << 0.001 * distance(a_rad, r_rad) << " km" << std::endl;
    std::cout << "haversine other radius: " << distance(city1, city2, strategy::distance::haversine<latlon_point>(6371.0) ) << " km" << std::endl;
    std::cout << "andoyer:                " << 0.001 * distance(city1, city2, strategy::distance::andoyer<latlon_point>() ) << " km" << std::endl;
    std::cout << "vincenty:               " << 0.001 * distance(city1, city2, strategy::distance::vincenty<latlon_point>() ) << " km" << std::endl;
    std::cout << "vincenty rad:           " << 0.001 * distance(a_rad, r_rad, strategy::distance::vincenty<model::point_ll_rad>() ) << " km" << std::endl;
    std::cout << "Projected, pythagoras:  " << 0.001 * distance(a_rd, r_rd) << " km" << std::endl;

    std::cout << std::endl;
    std::cout << "Distance " << city1_name << "-" << city3_name << ": " << std::endl;
    std::cout << "andoyer:                " << 0.001 * distance(city1, city3, strategy::distance::andoyer<latlon_point>()) << " km" << std::endl;
    std::cout << "Distance " << city2_name << "-" << city3_name << ": " << std::endl;
    std::cout << "andoyer:                " << 0.001 * distance(city2, city3, strategy::distance::andoyer<latlon_point>()) << " km" << std::endl;

    // ------------------------------------------------------------------------------------------
    // Distances to segments
    // ------------------------------------------------------------------------------------------
    std::cout << std::endl << city3_name << " - line " << city1_name << "," << city2_name << std::endl;

    model::segment_2d ar_xy(a_rd, r_rd);

    double dr = distance(h_rd, ar_xy);
    std::cout << "in RD: " << 0.001 * dr << std::endl;

    double const radius = 6378137.0;

    dr = distance(city3, model::segment_ll_deg(city1, city2));
    std::cout << "in LL: " << 0.001 * dr << std::endl;

    std::cout << std::endl << city2_name << " - line " << city1_name << "," << city3_name << std::endl;
    dr = distance(r_rd, model::segment_2d(a_rd, h_rd));
    std::cout << "in RD: " << 0.001 * dr << std::endl;
    dr = distance(city2, model::segment_ll_deg(city1, city3));
    std::cout << "in LL: " << 0.001 * dr << std::endl;
    std::cout << std::endl;
    

    // ------------------------------------------------------------------------------------------
    // Compilation
    // ------------------------------------------------------------------------------------------
    // Next line does not compile because Vincenty cannot work on xy-points
    //std::cout << "vincenty on xy:         " << 0.001 * distance(a_rd, r_rd, formulae::distance::vincenty<>() ) << " km" << std::endl;

    // Next line does not compile because you cannot (yet) assign degree to radian directly
    //point_ll_rad a_rad2 = city1;

    // Next line does not compile because you cannot assign latlong to xy
    // point_2d axy = city1;

    // ------------------------------------------------------------------------------------------
    // Length
    // ------------------------------------------------------------------------------------------
    // Length calculations use distances internally. The lines below take automatically the default
    // formulae for distance. However, you can also specify city1 formula explicitly.

    model::linestring_ll_deg line1;
    append(line1, city1);
    append(line1, city2);
    std::cout << "length: " << length(line1) << std::endl;
    std::cout << "length using Vincenty: " << length(line1, strategy::distance::vincenty<latlon_point>()) << std::endl;

    model::linestring_2d line2;
    append(line2, a_rd);
    append(line2, r_rd);
    std::cout << "length: " << length(line2) << std::endl;

    return 0;
}
