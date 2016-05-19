// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2016 Oracle and/or its affiliates.
// Contributed and/or modified by Vissarion Fisikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <algorithms/test_length.hpp>

#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/adapted/std_pair_as_segment.hpp>

#include <test_geometries/all_custom_linestring.hpp>
#include <test_geometries/wrapped_boost_array.hpp>

std::vector<std::string > Ls_data_gen()
{
   std::string arr[] = {"LINESTRING(0 0,180 0,180 180)",
                        "LINESTRING(0 0,180 0,180 0,180 0,180 180,180 180)",
                        "LINESTRING(0 0,180 0,180 10,180 20,180 30,180 40,180 50,180 60,180 70,180 80,180 90,180 100,\
                        180 110,180 120,180 130,180 140,180 150,180 160,180 170,180 180)"};
   std::vector<std::string> Ls_data (arr, arr + sizeof(arr) / sizeof(arr[0]));
   return Ls_data;
}

template <typename P>
void test_all_default() //test the default strategy
{
    double const pi = boost::math::constants::pi<double>();
    std::vector<std::string > Ls_data = Ls_data_gen();

    for(size_t i=0; i<2; ++i)
        test_geometry<bg::model::linestring<P> >(Ls_data[i], 2 * pi);

    // Geometries with length zero
    test_geometry<P>("POINT(0 0)", 0);
    test_geometry<bg::model::polygon<P> >("POLYGON((0 0,0 1,1 1,1 0,0 0))", 0);
}

template <typename P>
void test_all_haversine(double const mean_radius)
{
    double const pi = boost::math::constants::pi<double>();
    bg::strategy::distance::haversine<float> haversine_strategy(mean_radius);
    std::vector<std::string > Ls_data = Ls_data_gen();

    for(size_t i=0; i<2; ++i)
        test_geometry<bg::model::linestring<P> >(Ls_data[i], 2 * pi * mean_radius, haversine_strategy);

    // Geometries with length zero
    test_geometry<P>("POINT(0 0)", 0, haversine_strategy);
    test_geometry<bg::model::polygon<P> >("POLYGON((0 0,0 1,1 1,1 0,0 0))", 0, haversine_strategy);
    //TODO why the following is not zero but 2.44929359829470641435e-16 ?
    //test_geometry<std::pair<P, P> >("LINESTRING(0 0,360 0)", 0);
    //TODO why the following is not zero but 4.8985871965894128287e-16 ?
    //test_geometry<std::pair<P, P> >("LINESTRING(0 0,720 0)", 0);
}

template <typename P>
void test_empty_input()
{
    test_empty_input(bg::model::linestring<P>());
}

int test_main(int, char* [])
{
    //Earth radius estimation (see https://en.wikipedia.org/wiki/Earth_radius)
    double const mean_radius = 6371.0;

    test_all_haversine<bg::model::d2::point_xy<int, bg::cs::spherical_equatorial<bg::degree> > >(mean_radius);
    test_all_haversine<bg::model::d2::point_xy<float, bg::cs::spherical_equatorial<bg::degree> > >(mean_radius);
    test_all_haversine<bg::model::d2::point_xy<double , bg::cs::spherical_equatorial<bg::degree> > >(mean_radius);

#if defined(HAVE_TTMATH)
    test_all<bg::model::d2::point_xy<ttmath_big> >();
#endif

    //test_empty_input<bg::model::d2::point_xy<int> >();

    return 0;
}
