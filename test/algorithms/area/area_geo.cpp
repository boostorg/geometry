// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// This file was modified by Oracle on 2015, 2016.
// Modifications copyright (c) 2015-2016, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <boost/geometry.hpp>
#include <geometry_test_common.hpp>

namespace bg = boost::geometry;

//Testing geographic strategies
template <typename CT>
void test_geo_strategies()
{
    std::string poly = "POLYGON((52 0, 41 -74, -23 -43, -26 28, 52 0))";

    typedef bg::model::point<CT, 2,
            bg::cs::geographic<bg::degree> > pt_geo;

    typedef typename bg::point_type<pt_geo>::type pt_geo_type;

    bg::strategy::area::area_geographic
            <pt_geo_type> geographic_default;

    bg::strategy::area::area_geographic
            <
                pt_geo_type,
                bg::formula::andoyer_inverse,
                1,
                true
            > geographic_andoyer1;
    bg::strategy::area::area_geographic
            <
                pt_geo_type,
                bg::formula::andoyer_inverse,
                2,
                true
            > geographic_andoyer2;
    bg::strategy::area::area_geographic
            <
                pt_geo_type,
                bg::formula::andoyer_inverse,
                3,
                true
            > geographic_andoyer3;
    bg::strategy::area::area_geographic
            <
                pt_geo_type,
                bg::formula::andoyer_inverse,
                4,
                true
            > geographic_andoyer4;
    bg::strategy::area::area_geographic
            <
                pt_geo_type,
                bg::formula::andoyer_inverse,
                5,
                true
            > geographic_andoyer5;

    bg::strategy::area::area_geographic
            <
                pt_geo_type,
                bg::formula::thomas_inverse,
                1,
                true
            > geographic_thomas1;
    bg::strategy::area::area_geographic
            <
                pt_geo_type,
                bg::formula::thomas_inverse,
                2,
                true
            > geographic_thomas2;
    bg::strategy::area::area_geographic
            <
                pt_geo_type,
                bg::formula::thomas_inverse,
                3,
                true
            > geographic_thomas3;
    bg::strategy::area::area_geographic
            <
                pt_geo_type,
                bg::formula::thomas_inverse,
                4,
                true
            > geographic_thomas4;
    bg::strategy::area::area_geographic
            <
                pt_geo_type,
                bg::formula::thomas_inverse,
                5,
                true
            > geographic_thomas5;

    bg::strategy::area::area_geographic
            <
                pt_geo_type,
                bg::formula::vincenty_inverse,
                1,
                true
            > geographic_vincenty1;
    bg::strategy::area::area_geographic
            <
                pt_geo_type,
                bg::formula::vincenty_inverse,
                2,
                true
            > geographic_vincenty2;
    bg::strategy::area::area_geographic
            <
                pt_geo_type,
                bg::formula::vincenty_inverse,
                3,
                true
            > geographic_vincenty3;
    bg::strategy::area::area_geographic
            <
                pt_geo_type,
                bg::formula::vincenty_inverse,
                4,
                true
            > geographic_vincenty4;
    bg::strategy::area::area_geographic
            <
                pt_geo_type,
                bg::formula::vincenty_inverse,
                5,
                true
            > geographic_vincenty5;

    bg::strategy::area::area_geographic
            <
                pt_geo_type,
                bg::formula::vincenty_inverse,
                5
            > geographic_vincenty5_default;

    bg::strategy::area::area_geographic
            <
                pt_geo_type,
                bg::formula::vincenty_inverse
            > geographic_vincenty_default;

    bg::model::polygon<pt_geo> geometry_geo;

    //GeographicLib         63316536351834.289
    //PostGIS (v2.2.2)      6.33946+13
    //MS SQL SERVER         632930207487035

    bg::read_wkt(poly, geometry_geo);
    CT area;
    CT err = 0.0000001;

    CT area_default = bg::area(geometry_geo);
    BOOST_CHECK_CLOSE(area_default, 63316536092341.266, err);
    area = bg::area(geometry_geo, geographic_default);
    BOOST_CHECK_CLOSE(area, 63316536092341.266, err);

    CT area_less_accurate = bg::area(geometry_geo, geographic_andoyer1);
    BOOST_CHECK_CLOSE(area_less_accurate, 63316309346280.18, err);
    area = bg::area(geometry_geo, geographic_andoyer2);
    BOOST_CHECK_CLOSE(area, 63316309224306.5, err);
    area = bg::area(geometry_geo, geographic_andoyer3);
    BOOST_CHECK_CLOSE(area, 63316309224411.195, err);
    area = bg::area(geometry_geo, geographic_andoyer4);
    BOOST_CHECK_CLOSE(area, 63316309224411.094, err);
    area = bg::area(geometry_geo, geographic_andoyer5);
    BOOST_CHECK_CLOSE(area, 63316309224411.094, err);

    area = bg::area(geometry_geo, geographic_thomas1);
    BOOST_CHECK_CLOSE(area, 63316536214315.32, err);
    area = bg::area(geometry_geo, geographic_thomas2);
    BOOST_CHECK_CLOSE(area, 63316536092341.266, err);
    area = bg::area(geometry_geo, geographic_thomas3);
    BOOST_CHECK_CLOSE(area, 63316536092445.961, err);
    area = bg::area(geometry_geo, geographic_thomas4);
    BOOST_CHECK_CLOSE(area, 63316536092445.859, err);
    area = bg::area(geometry_geo, geographic_thomas5);
    BOOST_CHECK_CLOSE(area, 63316536092445.859, err);

    area = bg::area(geometry_geo, geographic_vincenty1);
    BOOST_CHECK_CLOSE(area, 63316536473798.984, err);
    area = bg::area(geometry_geo, geographic_vincenty2);
    BOOST_CHECK_CLOSE(area, 63316536351824.93, err);
    area = bg::area(geometry_geo, geographic_vincenty3);
    BOOST_CHECK_CLOSE(area, 63316536351929.625, err);
    area = bg::area(geometry_geo, geographic_vincenty4);
    BOOST_CHECK_CLOSE(area, 63316536351929.523, err);
    CT area_most_accurate = bg::area(geometry_geo, geographic_vincenty5);
    BOOST_CHECK_CLOSE(area, 63316536351929.523, err);

    area = bg::area(geometry_geo, geographic_vincenty5_default);
    BOOST_CHECK_CLOSE(area, 63316536351929.523, err);
    area = bg::area(geometry_geo, geographic_vincenty_default);
    BOOST_CHECK_CLOSE(area, 63316536351824.93, err);

    BOOST_CHECK_CLOSE(area_most_accurate, area_less_accurate, .001);
    BOOST_CHECK_CLOSE(area_most_accurate, area_default, .000001);
/*
    // timings and accuracy
    std::cout.precision(25);
    std::size_t exp_times = 100000;
    {   clock_t startTime = clock();
        for (int j=0; j < exp_times; j++) area = bg::area(geometry_geo, geographic_andoyer1);
        std::cout << double( clock() - startTime ) / (double)CLOCKS_PER_SEC<< " ";
        std::cout  << area << std::endl;}
    {   clock_t startTime = clock();
        for (int j=0; j < exp_times; j++) area = bg::area(geometry_geo, geographic_andoyer2);
        std::cout << double( clock() - startTime ) / (double)CLOCKS_PER_SEC<< " ";
        std::cout  << area << std::endl;}
    {   clock_t startTime = clock();
        for (int j=0; j < exp_times; j++) area = bg::area(geometry_geo, geographic_andoyer3);
        std::cout << double( clock() - startTime ) / (double)CLOCKS_PER_SEC<< " ";
        std::cout  << area << std::endl;}
    {   clock_t startTime = clock();
        for (int j=0; j < exp_times; j++) area = bg::area(geometry_geo, geographic_andoyer4);
        std::cout << double( clock() - startTime ) / (double)CLOCKS_PER_SEC<< " ";
        std::cout  << area << std::endl;}
    {   clock_t startTime = clock();
        for (int j=0; j < exp_times; j++) area = bg::area(geometry_geo, geographic_andoyer5);
        std::cout << double( clock() - startTime ) / (double)CLOCKS_PER_SEC<< " ";
        std::cout  << area << std::endl;}
    {   clock_t startTime = clock();
        for (int j=0; j < exp_times; j++) area = bg::area(geometry_geo, geographic_thomas1);
        std::cout << double( clock() - startTime ) / (double)CLOCKS_PER_SEC<< " ";
        std::cout  << area << std::endl;}
    {   clock_t startTime = clock();
        for (int j=0; j < exp_times; j++) area = bg::area(geometry_geo, geographic_thomas2);
        std::cout << double( clock() - startTime ) / (double)CLOCKS_PER_SEC<< " ";
        std::cout  << area << std::endl;}
    {   clock_t startTime = clock();
        for (int j=0; j < exp_times; j++) area = bg::area(geometry_geo, geographic_thomas3);
        std::cout << double( clock() - startTime ) / (double)CLOCKS_PER_SEC<< " ";
        std::cout  << area << std::endl;}
    {   clock_t startTime = clock();
        for (int j=0; j < exp_times; j++) area = bg::area(geometry_geo, geographic_thomas4);
        std::cout << double( clock() - startTime ) / (double)CLOCKS_PER_SEC<< " ";
        std::cout  << area << std::endl;}
    {   clock_t startTime = clock();
        for (int j=0; j < exp_times; j++) area = bg::area(geometry_geo, geographic_thomas5);
        std::cout << double( clock() - startTime ) / (double)CLOCKS_PER_SEC<< " ";
        std::cout  << area << std::endl;}
    {   clock_t startTime = clock();
        for (int j=0; j < exp_times; j++) area = bg::area(geometry_geo, geographic_vincenty1);
        std::cout << double( clock() - startTime ) / (double)CLOCKS_PER_SEC<< " ";
        std::cout  << area << std::endl;}
    {   clock_t startTime = clock();
        for (int j=0; j < exp_times; j++) area = bg::area(geometry_geo, geographic_vincenty2);
        std::cout << double( clock() - startTime ) / (double)CLOCKS_PER_SEC<< " ";
        std::cout  << area << std::endl;}
    {   clock_t startTime = clock();
        for (int j=0; j < exp_times; j++) area = bg::area(geometry_geo, geographic_vincenty3);
        std::cout << double( clock() - startTime ) / (double)CLOCKS_PER_SEC<< " ";
        std::cout  << area << std::endl;}
    {   clock_t startTime = clock();
        for (int j=0; j < exp_times; j++) area = bg::area(geometry_geo, geographic_vincenty4);
        std::cout << double( clock() - startTime ) / (double)CLOCKS_PER_SEC<< " ";
        std::cout  << area << std::endl;}
    {   clock_t startTime = clock();
        for (int j=0; j < exp_times; j++) area = bg::area(geometry_geo, geographic_vincenty5);
        std::cout << double( clock() - startTime ) / (double)CLOCKS_PER_SEC<< " ";
        std::cout  << area << std::endl;}
*/
}

int test_main(int, char* [])
{

    test_geo_strategies<double>();

    return 0;
}
