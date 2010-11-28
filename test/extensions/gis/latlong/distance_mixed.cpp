// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <geometry_test_common.hpp>
#include <boost/test/unit_test.hpp>

#include <boost/geometry/algorithms/assign.hpp>

#include <boost/geometry/geometry.hpp>
#include <boost/geometry/extensions/gis/latlong/latlong.hpp>

#include <test_common/test_point.hpp>



int test_main(int, char* [])
{
    using namespace bg::strategy::distance;

    bg::model::point_ll_deg paris;
    paris.lat(bg::dms<bg::north>(48, 52, 0));
    paris.lon(bg::dms<bg::east>(2, 19, 59));

    bg::model::point_ll_deg amsterdam;
    amsterdam.lat(bg::dms<bg::north>(52, 22, 23));
    amsterdam.lon(bg::dms<bg::east>(4, 53, 32));

    bg::model::point_ll_rad paris_rad, amsterdam_rad;
    transform(amsterdam, amsterdam_rad);
    transform(paris, paris_rad);

    double d1 = 0.001 * distance(paris, amsterdam);
    double d2 = 0.001 * distance(paris_rad, amsterdam_rad);

    double d3 = 0.001 * distance(paris, amsterdam_rad);
    double d4 = 0.001 * distance(paris_rad, amsterdam);
    std::cout << "Distances: " << d1 << " == " << d2 << " == " << d3 << " == " << d4 << std::endl;

    double d5 = 0.001 * distance(paris, amsterdam, vincenty<bg::model::point_ll_deg, bg::model::point_ll_deg>());
    double d6 = 0.001 * distance(paris_rad, amsterdam_rad, vincenty<bg::model::point_ll_rad, bg::model::point_ll_rad>());
    double d7 = 0.001 * distance(paris, amsterdam_rad, vincenty<bg::model::point_ll_deg, bg::model::point_ll_rad>());
    double d8 = 0.001 * bg::distance(paris_rad, amsterdam, vincenty<bg::model::point_ll_rad, bg::model::point_ll_deg>());
    std::cout << "Distances: " << d5 << " == " << d6 << " == " << d7 << " == " << d8 << std::endl;

    bg::model::point_ll_deg barcelona(
        bg::latitude<>(bg::dms<bg::north>(41, 23)),
        bg::longitude<>(bg::dms<bg::east>(2, 11))
        );

    // Now declare a line in latlong and calculate the distance, this MUST reverse...
    bg::model::linestring_ll_deg ab;
    ab.push_back(amsterdam);
    ab.push_back(barcelona);

    double d9 = 0.001 * distance(ab, paris);
    double d10 = 0.001 * distance(paris, ab);
    double d11 = 0.001 * distance(paris, ab, vincenty<bg::model::point_ll_deg, bg::model::point_ll_deg>());
    double d12 = 0.001 * distance(ab, paris, vincenty<bg::model::point_ll_deg, bg::model::point_ll_deg>());
    std::cout << "Distances: " << d9 << " == " << d10 << " == " << d11 << " == " << d12 << std::endl;

    // TODO: solve this case, it is reversed -> strategy should be reversed as well
    // For this, first implement that linestring can have other coor.sys then point type...
    //double d13 = 0.001 * distance(ab, paris_rad, vincenty<point_ll_deg, point_ll_rad>());

    return 0;
}
