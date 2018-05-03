// Boost.Geometry
// Unit Test

// Copyright (c) 2017 Adam Wulkiewicz, Lodz, Poland.

// Copyright (c) 2017-2018, Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <string>

#include "test_union.hpp"


struct exterior_points_counter
{
    exterior_points_counter() : count(0) {}

    template <typename Polygon>
    void operator()(Polygon const& poly)
    {
        count += boost::size(bg::exterior_ring(poly));
    }

    std::size_t count;
};

struct interiors_counter
    : exterior_points_counter
{
    template <typename Polygon>
    void operator()(Polygon const& poly)
    {
        count += boost::size(bg::interior_rings(poly));
    }
};

void test_spherical_one(std::string const& wkt1, std::string const& wkt2,
                        std::size_t count, std::size_t exterior_points_count, std::size_t interiors_count,
                        double expected_area)
{
    typedef bg::model::point<double, 2, bg::cs::spherical_equatorial<bg::degree> > point;
    typedef bg::model::polygon<point> polygon;
    typedef bg::model::multi_polygon<polygon> multipolygon;

    polygon p1, p2;

    boost::geometry::read_wkt(wkt1, p1);
    boost::geometry::read_wkt(wkt2, p2);

    multipolygon result;

    boost::geometry::union_(p1, p2, result);

    double result_area = bg::area(result);

    std::size_t result_count = boost::size(result);
    std::size_t result_exterior_points = std::for_each(boost::begin(result),
                                                       boost::end(result),
                                                       exterior_points_counter()).count;
    std::size_t result_interiors = std::for_each(boost::begin(result),
                                                 boost::end(result),
                                                 interiors_counter()).count;
    if (expected_area >= 0)
    {
        BOOST_CHECK_EQUAL(result_count, count);
        BOOST_CHECK_EQUAL(result_exterior_points, exterior_points_count);
        BOOST_CHECK_EQUAL(result_interiors, interiors_count);
        BOOST_CHECK_CLOSE(result_area, expected_area, 0.001);
    }
    else
    {
        BOOST_CHECK_EQUAL(result_count, 0u);
        BOOST_CHECK_EQUAL(result_area, 0.0);
    }
}


void test_spherical()
{
    // https://github.com/boostorg/geometry/issues/475
    test_spherical_one("POLYGON((-78.4072265625001 43.06652924482626,-78.4072265625 43.06740063068311,-78.4063141178686 43.06653210403569,-78.4072265625001 43.06652924482626))",
                       "POLYGON((-78.55968743491499 43.06594969590624,-78.55036227331367 43.07380195109801,-78.53503704605811 43.08248347074284,-78.51769210872999 43.08880392487917,-78.49899564953199 43.09251971058174,-78.47966844278045 43.09348761253013,-78.46045580120891 43.09167037120638,-78.44209853911326 43.08713812460473,-78.42530412309867 43.08006566649393,-78.41071917768537 43.07072563376782,-78.40631359930124 43.06653210565861,-78.55968743491499 43.06594969590624))",
                       1, 12, 0, 0.00000064324358632259458);
}


int test_main(int, char* [])
{
    test_spherical();

    return 0;
}
