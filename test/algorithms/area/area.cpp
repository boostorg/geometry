// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2012 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2012 Mateusz Loskot, London, UK.

// This file was modified by Oracle on 2015, 2016.
// Modifications copyright (c) 2015-2016, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <algorithms/area/test_area.hpp>

#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/ring.hpp>
#include <boost/geometry/geometries/polygon.hpp>

#include <test_geometries/all_custom_ring.hpp>
#include <test_geometries/all_custom_polygon.hpp>
//#define BOOST_GEOMETRY_TEST_DEBUG

#include <boost/variant/variant.hpp>

template <typename Polygon>
void test_polygon()
{
    // Rotated square, length=sqrt(2) -> area=2
    test_geometry<Polygon>("POLYGON((1 1,2 2,3 1,2 0,1 1))", 2.0);
    test_geometry<Polygon>("POLYGON((1 1,2 2,3 1,2 0,1 1))", 2.0);
    test_geometry<Polygon>("POLYGON((0 0,0 7,4 2,2 0,0 0))", 16.0);
    test_geometry<Polygon>("POLYGON((1 1,2 1,2 2,1 2,1 1))", -1.0);
    test_geometry<Polygon>("POLYGON((0 0,0 7,4 2,2 0,0 0), (1 1,2 1,2 2,1 2,1 1))", 15.0);
}


template <typename P>
void test_all()
{
    test_geometry<bg::model::box<P> >("POLYGON((0 0,2 2))", 4.0);
    test_geometry<bg::model::box<P> >("POLYGON((2 2,0 0))", 4.0);

    test_polygon<bg::model::polygon<P> >();
    test_polygon<all_custom_polygon<P> >();

    // clockwise rings (second is wrongly ordered)
    test_geometry<bg::model::ring<P> >("POLYGON((0 0,0 7,4 2,2 0,0 0))", 16.0);
    test_geometry<bg::model::ring<P> >("POLYGON((0 0,2 0,4 2,0 7,0 0))", -16.0);

    test_geometry<all_custom_ring<P> >("POLYGON((0 0,0 7,4 2,2 0,0 0))", 16.0);

    // ccw
    test_geometry<bg::model::polygon<P, false> >
        ("POLYGON((0 0,0 7,4 2,2 0,0 0), (1 1,2 1,2 2,1 2,1 1))", -15.0);

    test_geometry<bg::model::polygon<P, false> >
        ("POLYGON((1 0,0 1,-1 0,0 -1,1 0))", 2);

    typedef typename bg::coordinate_type<P>::type coord_type;
    if (BOOST_GEOMETRY_CONDITION((boost::is_same<coord_type, double>::value)))
    {
        test_geometry<bg::model::polygon<P, false, false> >
            ("POLYGON((100000001 100000000, 100000000 100000001, \
                       99999999 100000000, 100000000  99999999))", 2);
    }
    else if (BOOST_GEOMETRY_CONDITION((boost::is_same<coord_type, float>::value)))
    {
        test_geometry<bg::model::polygon<P, false, false> >
            ("POLYGON((100001 100000, 100000 100001, \
                       99999 100000, 100000  99999))", 2);
    }
}

template <typename CT>
void test_spherical_geo()
{
    typedef CT ct;

    //Geographic

    typedef typename bg::model::point<
                                        ct,
                                        2,
                                        bg::cs::geographic<bg::degree>
                                     > pt_geo;

    typedef typename bg::point_type<pt_geo>::type pt_geo_type;

    typedef typename bg::formula::vincenty_inverse
            <
                ct,
                false,
                true,
                true
            > geo_strategy;

    bg::strategy::area::area_geographic
            <
                pt_geo_type,
                geo_strategy,
                5,
                true
            > area_geographic;

    bg::model::polygon<pt_geo> geometry_geo;

    //Spherical

    typedef typename bg::model::point<
                                        ct,
                                        2,
                                        bg::cs::spherical_equatorial<bg::degree>
                                     > pt;
    bg::model::polygon<pt> geometry;

    // unit-sphere has area of 4-PI. Polygon covering 1/8 of it:
    // calculations splitted for ttmath
    std::string poly = "POLYGON((0 0,0 90,90 0,0 0))";

    ct const four = 4.0;
    ct const eight = 8.0;
    ct expected = four * boost::geometry::math::pi<ct>() / eight;
    bg::read_wkt(poly, geometry);
    ct area = bg::area(geometry);
    BOOST_CHECK_CLOSE(area, expected, 0.0001);

    // With strategy, radius 2 -> 4 pi r^2
    bg::strategy::area::area_spherical
        <
            typename bg::point_type<pt>::type
        > strategy(2.0);

    area = bg::area(geometry, strategy);
    ct const two = 2.0;
    BOOST_CHECK_CLOSE(area, two * two * expected, 0.0001);

    // Geographic total area of earth is about 510072000
    // (https://en.wikipedia.org/wiki/Earth)
    // So the 1/8 is 63759000 and here we get something close to it
    bg::read_wkt(poly, geometry_geo);
    area = bg::area(geometry_geo, area_geographic);
    //GeoGraphicLib gives: 63758202715511.055
    BOOST_CHECK_CLOSE(area, 63758202715509.859, 0.0001);


    // Wrangel Island (dateline crossing)
    // With (spherical) Earth strategy
    poly = "POLYGON((-178.7858 70.7852, 177.4758 71.2333, 179.7436 71.5733, -178.7858 70.7852))";

    bg::strategy::area::area_spherical
        <
            typename bg::point_type<pt>::type
        > spherical_earth(6373);
    bg::read_wkt(poly, geometry);
    area = bg::area(geometry, spherical_earth);
    // SQL Server gives: 4537.9654419375
    // PostGIS gives: 4537.9311668307
    // Note: those are Geographic, this test is Spherical
    BOOST_CHECK_CLOSE(area, 4506.6389, 0.001);

    bg::read_wkt(poly, geometry_geo);
    area = bg::area(geometry_geo, area_geographic);
    BOOST_CHECK_CLOSE(area, 4537974350.6843719, 0.0001);

    // Wrangel, more in detail
    poly = "POLYGON((-178.568604 71.564148,-178.017548 71.449692,-177.833313 71.3461,\
            -177.502838 71.277466 ,-177.439453 71.226929,-177.620026 71.116638,\
            -177.9389 71.037491,-178.8186 70.979965,-179.274445 70.907761,\
            -180 70.9972,179.678314 70.895538,179.272766 70.888596,\
             178.791016 70.7964,178.617737 71.035538,178.872192 71.217484,\
             179.530273 71.4383 ,-180 71.535843 ,-179.628601 71.577194,\
            -179.305298 71.551361,-179.03421 71.597748,-178.568604 71.564148))";
    bg::read_wkt(poly, geometry);
    area = bg::area(geometry, spherical_earth);
    // SQL Server gives: 7669.10402181435
    // PostGIS gives: 7669.55565459832
    BOOST_CHECK_CLOSE(area, 7616.523769, 0.001);

    bg::read_wkt(poly, geometry_geo);
    area = bg::area(geometry_geo, area_geographic);
    BOOST_CHECK_CLOSE(area, 7669498457.4130802, 0.0001);

    // Check more at the equator
    /*
    select 1,geography::STGeomFromText('POLYGON((-178.7858 10.7852 , 179.7436 11.5733 , 177.4758 11.2333 , -178.7858 10.7852))',4326) .STArea()/1000000.0
    union select 2,geography::STGeomFromText('POLYGON((-178.7858 20.7852 , 179.7436 21.5733 , 177.4758 21.2333 , -178.7858 20.7852))',4326) .STArea()/1000000.0
    union select 3,geography::STGeomFromText('POLYGON((-178.7858 30.7852 , 179.7436 31.5733 , 177.4758 31.2333 , -178.7858 30.7852))',4326) .STArea()/1000000.0
    union select 0,geography::STGeomFromText('POLYGON((-178.7858 0.7852 , 179.7436 1.5733 , 177.4758 1.2333 , -178.7858 0.7852))',4326) .STArea()/1000000.0
    union select 4,geography::STGeomFromText('POLYGON((-178.7858 40.7852 , 179.7436 41.5733 , 177.4758 41.2333 , -178.7858 40.7852))',4326) .STArea()/1000000.0
    */

    poly = "POLYGON((-178.7858 0.7852, 177.4758 1.2333, 179.7436 1.5733, -178.7858 0.7852))";
    bg::read_wkt(poly, geometry);
    area = bg::area(geometry, spherical_earth);
    BOOST_CHECK_CLOSE(area, 14136.09946, 0.001); // SQL Server gives: 14064.1902284513
    bg::read_wkt(poly, geometry_geo);
    area = bg::area(geometry_geo, area_geographic);
    BOOST_CHECK_CLOSE(area, 14064196647.560314, 0.0001);

    poly = "POLYGON((-178.7858 10.7852, 177.4758 11.2333, 179.7436 11.5733, -178.7858 10.7852))";
    bg::read_wkt(poly, geometry);
    area = bg::area(geometry, spherical_earth);
    BOOST_CHECK_CLOSE(area, 13760.2456, 0.001); // SQL Server gives: 13697.0941155193
    bg::read_wkt(poly, geometry_geo);
    area = bg::area(geometry_geo, area_geographic);
    BOOST_CHECK_CLOSE(area, 13697095227.008451, 0.0001);

    poly = "POLYGON((-178.7858 20.7852, 177.4758 21.2333, 179.7436 21.5733, -178.7858 20.7852))";
    bg::read_wkt(poly, geometry);
    area = bg::area(geometry, spherical_earth);
    BOOST_CHECK_CLOSE(area, 12987.8682, 0.001); // SQL Server gives: 12944.3970990317 -> -39m^2
    bg::read_wkt(poly, geometry_geo);
    area = bg::area(geometry_geo, area_geographic);
    BOOST_CHECK_CLOSE(area, 12944392155.546635, 0.0001);

    poly = "POLYGON((-178.7858 30.7852, 177.4758 31.2333, 179.7436 31.5733, -178.7858 30.7852))";
    bg::read_wkt(poly, geometry);
    area = bg::area(geometry, spherical_earth);
    BOOST_CHECK_CLOSE(area, 11856.3935, 0.001); // SQL Server gives: 11838.5338423574 -> -18m^2
    bg::read_wkt(poly, geometry_geo);
    area = bg::area(geometry_geo, area_geographic);
    BOOST_CHECK_CLOSE(area, 11838524075.970982, 0.0001);

    poly = "POLYGON((-178.7858 40.7852, 177.4758 41.2333, 179.7436 41.5733, -178.7858 40.7852))";
    bg::read_wkt(poly, geometry);
    area = bg::area(geometry, spherical_earth);
    BOOST_CHECK_CLOSE(area, 10404.627685523914, 0.001); // SQL Server gives: 10412.0607137119, -> +8m^2
    bg::read_wkt(poly, geometry_geo);
    area = bg::area(geometry_geo, area_geographic);
    BOOST_CHECK_CLOSE(area, 10412049661.81769, 0.0001);

    // Concave
    poly = "POLYGON((0 40,1 42,0 44,2 43,4 44,3 42,4 40,2 41,0 40))";
    bg::read_wkt(poly, geometry);
    area = bg::area(geometry, spherical_earth);
    BOOST_CHECK_CLOSE(area, 73538.2958, 0.001); // SQL Server gives: 73604.2047689719
    bg::read_wkt(poly, geometry_geo);
    area = bg::area(geometry_geo, area_geographic);
    BOOST_CHECK_CLOSE(area, 73604208172.719223, 0.0001);

    // With hole POLYGON((0 40,4 40,4 44,0 44,0 40),(1 41,2 43,3 42,1 41))
    poly = "POLYGON((0 40,0 44,4 44,4 40,0 40),(1 41,3 42,2 43,1 41))";
    bg::read_wkt(poly, geometry);
    area = bg::area(geometry, spherical_earth);
    BOOST_CHECK_CLOSE(area, 133233.844876, 0.001); // SQL Server gives: 133353.335
    bg::read_wkt(poly, geometry_geo);
    area = bg::area(geometry_geo, area_geographic);
    BOOST_CHECK_CLOSE(area, 133353331647.58241, 0.0001);

    // around 0 meridian
    {
        std::string poly1 = "POLYGON((-10 0,-10 10,0 10,0 0,-10 0))";
        std::string poly2 = "POLYGON((0 0,0 10,10 10,10 0,0 0))";
        std::string poly3 = "POLYGON((-5 0,-5 10,5 10,5 0,-5 0))";
        bg::read_wkt(poly1, geometry);
        ct area1 = bg::area(geometry);
        bg::read_wkt(poly2, geometry);
        ct area2 = bg::area(geometry);
        bg::read_wkt(poly3, geometry);
        ct area3 = bg::area(geometry);
        BOOST_CHECK_CLOSE(area1, area2, 0.001);
        BOOST_CHECK_CLOSE(area2, area3, 0.001);
        BOOST_CHECK_CLOSE(area1, 0.0303822, 0.001);
        //geographic
        bg::read_wkt(poly1, geometry_geo);
        area1 = bg::area(geometry_geo, area_geographic);
        bg::read_wkt(poly2, geometry_geo);
        area2 = bg::area(geometry_geo, area_geographic);
        bg::read_wkt(poly3, geometry_geo);
        area3 = bg::area(geometry_geo, area_geographic);
        BOOST_CHECK_CLOSE(area1, area2, 0.001);
        BOOST_CHECK_CLOSE(area2, area3, 0.001);
        BOOST_CHECK_CLOSE(area1, 1227857282668.3145, 0.001);
    }
    {
        std::string poly1 = "POLYGON((-10 -5,-10 5,0 5,0 -5,-10 -5))";
        std::string poly2 = "POLYGON((0 -5,0 5,10 5,10 -5,0 -5))";
        std::string poly3 = "POLYGON((-5 -5,-5 5,5 5,5 -5,-5 -5))";
        bg::read_wkt(poly1, geometry);
        ct area1 = bg::area(geometry);
        bg::read_wkt(poly2, geometry);
        ct area2 = bg::area(geometry);
        bg::read_wkt(poly3, geometry);
        ct area3 = bg::area(geometry);
        BOOST_CHECK_CLOSE(area1, area2, 0.001);
        BOOST_CHECK_CLOSE(area2, area3, 0.001);
        BOOST_CHECK_CLOSE(area1, 0.0305, 0.001);
        //geographic
        bg::read_wkt(poly1, geometry_geo);
        area1 = bg::area(geometry_geo, area_geographic);
        bg::read_wkt(poly2, geometry_geo);
        area2 = bg::area(geometry_geo, area_geographic);
        bg::read_wkt(poly3, geometry_geo);
        area3 = bg::area(geometry_geo, area_geographic);
        BOOST_CHECK_CLOSE(area1, area2, 0.001);
        BOOST_CHECK_CLOSE(area2, area3, 0.001);
        BOOST_CHECK_CLOSE(area1, 1232493707152.2292, 0.001);
    }
    // around 180 meridian
    {
        std::string poly1 = "POLYGON((-180 0,-180 10,-170 10,-170 0,-180 0))";
        std::string poly2 = "POLYGON((175 0,175 10,-175 10,-175 0,175 0))";
        std::string poly3 = "POLYGON((170 0,170 10,180 10,180 0,170 0))";
        std::string poly4 = "POLYGON((170 0,170 10,-180 10,-180 0,170 0))";
        std::string poly5 = "POLYGON((180 0,180 10,-170 10,-170 0,180 0))";
        bg::read_wkt(poly1, geometry);
        ct area1 = bg::area(geometry);
        bg::read_wkt(poly2, geometry);
        ct area2 = bg::area(geometry);
        bg::read_wkt(poly3, geometry);
        ct area3 = bg::area(geometry);
        bg::read_wkt(poly4, geometry);
        ct area4 = bg::area(geometry);
        bg::read_wkt(poly5, geometry);
        ct area5 = bg::area(geometry);
        BOOST_CHECK_CLOSE(area1, area2, 0.001);
        BOOST_CHECK_CLOSE(area2, area3, 0.001);
        BOOST_CHECK_CLOSE(area3, area4, 0.001);
        BOOST_CHECK_CLOSE(area4, area5, 0.001);
        BOOST_CHECK_CLOSE(area1, 0.0303822, 0.001);
        //geographic
        bg::read_wkt(poly1, geometry_geo);
        area1 = bg::area(geometry_geo, area_geographic);
        bg::read_wkt(poly2, geometry_geo);
        area2 = bg::area(geometry_geo, area_geographic);
        bg::read_wkt(poly3, geometry_geo);
        area3 = bg::area(geometry_geo, area_geographic);
        bg::read_wkt(poly4, geometry_geo);
        area4 = bg::area(geometry_geo, area_geographic);
        bg::read_wkt(poly5, geometry_geo);
        area5 = bg::area(geometry_geo, area_geographic);
        BOOST_CHECK_CLOSE(area1, area2, 0.001);
        BOOST_CHECK_CLOSE(area2, area3, 0.001);
        BOOST_CHECK_CLOSE(area3, area4, 0.001);
        BOOST_CHECK_CLOSE(area4, area5, 0.001);
        BOOST_CHECK_CLOSE(area1, 1227857282668.313, 0.001);
    }
    {
        std::string poly1 = "POLYGON((-180 -5,-180 5,-170 5,-170 -5,-180 -5))";
        std::string poly2 = "POLYGON((175 -5,175 5,-175 5,-175 -5,175 -5))";
        std::string poly3 = "POLYGON((170 -5,170 5,180 5,180 -5,170 -5))";
        std::string poly4 = "POLYGON((170 -5,170 5,180 5,180 -5,170 -5))";
        std::string poly5 = "POLYGON((180 -5,180 5,-170 5,-170 -5,180 -5))";
        bg::read_wkt(poly1, geometry);
        ct area1 = bg::area(geometry);
        bg::read_wkt(poly2, geometry);
        ct area2 = bg::area(geometry);
        bg::read_wkt(poly3, geometry);
        ct area3 = bg::area(geometry);
        bg::read_wkt(poly4, geometry);
        ct area4 = bg::area(geometry);
        bg::read_wkt(poly5, geometry);
        ct area5 = bg::area(geometry);
        BOOST_CHECK_CLOSE(area1, area2, 0.001);
        BOOST_CHECK_CLOSE(area2, area3, 0.001);
        BOOST_CHECK_CLOSE(area3, area4, 0.001);
        BOOST_CHECK_CLOSE(area4, area5, 0.001);
        BOOST_CHECK_CLOSE(area1, 0.0305, 0.001);
        //geographic
        bg::read_wkt(poly1, geometry_geo);
        area1 = bg::area(geometry_geo, area_geographic);
        bg::read_wkt(poly2, geometry_geo);
        area2 = bg::area(geometry_geo, area_geographic);
        bg::read_wkt(poly3, geometry_geo);
        area3 = bg::area(geometry_geo, area_geographic);
        bg::read_wkt(poly4, geometry_geo);
        area4 = bg::area(geometry_geo, area_geographic);
        bg::read_wkt(poly5, geometry_geo);
        area5 = bg::area(geometry_geo, area_geographic);
        BOOST_CHECK_CLOSE(area1, area2, 0.001);
        BOOST_CHECK_CLOSE(area2, area3, 0.001);
        BOOST_CHECK_CLOSE(area3, area4, 0.001);
        BOOST_CHECK_CLOSE(area4, area5, 0.001);
        BOOST_CHECK_CLOSE(area1, 1232493707152.2278, 0.001);
    }
    // around poles
    {
        std::string poly1 = "POLYGON((0 80,-90 80,-180 80,90 80,0 80))";
        std::string poly2 = "POLYGON((0 80,-90 80,180 80,90 80,0 80))";
        std::string poly3 = "POLYGON((0 -80,90 -80,-180 -80,-90 -80,0 -80))";
        std::string poly4 = "POLYGON((0 -80,90 -80,180 -80,-90 -80,0 -80))";
        bg::read_wkt(poly1, geometry);
        ct area1 = bg::area(geometry);
        bg::read_wkt(poly2, geometry);
        ct area2 = bg::area(geometry);
        bg::read_wkt(poly3, geometry);
        ct area3 = bg::area(geometry);
        bg::read_wkt(poly4, geometry);
        ct area4 = bg::area(geometry);
        BOOST_CHECK_CLOSE(area1, area2, 0.001);
        BOOST_CHECK_CLOSE(area2, area3, 0.001);
        BOOST_CHECK_CLOSE(area3, area4, 0.001);
        //geographic
        bg::read_wkt(poly1, geometry_geo);
        area1 = bg::area(geometry_geo, area_geographic);
        bg::read_wkt(poly2, geometry_geo);
        area2 = bg::area(geometry_geo, area_geographic);
        bg::read_wkt(poly3, geometry_geo);
        area3 = bg::area(geometry_geo, area_geographic);
        bg::read_wkt(poly4, geometry_geo);
        area4 = bg::area(geometry_geo, area_geographic);
        BOOST_CHECK_CLOSE(area1, area2, 0.001);
        BOOST_CHECK_CLOSE(area2, area3, 0.001);
        BOOST_CHECK_CLOSE(area3, area4, 0.001);
    }

    {
        bg::model::ring<pt> aurha; // a'dam-utr-rott.-den haag-a'dam
        std::string poly = "POLYGON((4.892 52.373,5.119 52.093,4.479 51.930,4.23 52.08,4.892 52.373))";
        bg::read_wkt(poly, aurha);
        /*if (polar)
        {
            // Create colatitudes (measured from pole)
            BOOST_FOREACH(pt& p, aurha)
            {
                bg::set<1>(p, ct(90) - bg::get<1>(p));
            }
            bg::correct(aurha);
        }*/
        bg::strategy::area::area_spherical
            <
                typename bg::point_type<pt>::type
            > area_spherical(6372.795);
        area = bg::area(aurha, area_spherical);
        BOOST_CHECK_CLOSE(area, 1476.645675, 0.0001);
        //geographic
        bg::read_wkt(poly, geometry_geo);
        area = bg::area(geometry_geo, area_geographic);
        BOOST_CHECK_CLOSE(area, 1481555970.0765088, 0.001);

        // SQL Server gives: 1481.55595960659
        // for select geography::STGeomFromText('POLYGON((4.892 52.373,4.23 52.08,4.479 51.930,5.119 52.093,4.892 52.373))',4326).STArea()/1000000.0
    }

}

template <typename P>
void test_ccw()
{
    typedef typename bg::coordinate_type<P>::type ct;
    bg::model::polygon<P, false> ccw_polygon;
    // counterclockwise rings (second is wrongly ordered)
    std::string poly1 = "POLYGON((1 1,2 2,3 1,2 0,1 1))";
    std::string poly2 = "POLYGON((1 1,2 0,3 1,2 2,1 1))";
    std::string poly3 = "POLYGON((0 0,0 7,4 2,2 0,0 0))";
    std::string poly4 = "POLYGON((0 0,2 0,4 2,0 7,0 0))";

    bg::read_wkt(poly1, ccw_polygon);
    ct area1 = bg::area(ccw_polygon);
    bg::read_wkt(poly2, ccw_polygon);
    ct area2 = bg::area(ccw_polygon);
    bg::read_wkt(poly3, ccw_polygon);
    ct area3 = bg::area(ccw_polygon);
    bg::read_wkt(poly4, ccw_polygon);
    ct area4 = bg::area(ccw_polygon);
    BOOST_CHECK_CLOSE(area1, -1 * area2, 0.001);
    BOOST_CHECK_CLOSE(area3, -1 * area4, 0.001);
}

template <typename P, typename CT>
void test_open(CT expected_area)
{
    typedef bg::model::polygon<P, true, false> open_polygon;
    test_geometry<open_polygon>("POLYGON((1 1,2 2,3 1,2 0))", expected_area);
    // Note the triangular testcase used in CCW is not sensible for open/close
}

template <typename P, typename CT>
void test_open_ccw(CT expected_area)
{
    typedef bg::model::polygon<P, false, false> open_polygon;
    test_geometry<open_polygon>("POLYGON((1 1,2 0,3 1,2 2))", expected_area);
    // Note the triangular testcase used in CCW is not sensible for open/close
}

template <typename P>
void test_empty_input()
{
    bg::model::polygon<P> poly_empty;
    bg::model::ring<P> ring_empty;

    test_empty_input(poly_empty);
    test_empty_input(ring_empty);
}

void test_large_integers()
{
    typedef bg::model::point<int, 2, bg::cs::cartesian> int_point_type;
    typedef bg::model::point<double, 2, bg::cs::cartesian> double_point_type;

    bg::model::polygon<int_point_type> int_poly;
    bg::model::polygon<double_point_type> double_poly;

    std::string const polygon_li = "POLYGON((1872000 528000,1872000 192000,1536119 192000,1536000 528000,1200000 528000,1200000 863880,1536000 863880,1872000 863880,1872000 528000))";
    bg::read_wkt(polygon_li, int_poly);
    bg::read_wkt(polygon_li, double_poly);

    double int_area = bg::area(int_poly);
    double double_area = bg::area(double_poly);

    BOOST_CHECK_CLOSE(int_area, double_area, 0.0001);
}

void test_variant()
{
    typedef bg::model::point<double, 2, bg::cs::cartesian> double_point_type;
    typedef bg::model::polygon<double_point_type> polygon_type;
    typedef bg::model::box<double_point_type> box_type;

    polygon_type poly;
    std::string const polygon_li = "POLYGON((18 5,18 1,15 1,15 5,12 5,12 8,15 8,18 8,18 5))";
    bg::read_wkt(polygon_li, poly);

    box_type box;
    std::string const box_li = "BOX(0 0,2 2)";
    bg::read_wkt(box_li, box);

    boost::variant<polygon_type, box_type> v;

    v = poly;
    BOOST_CHECK_CLOSE(bg::area(v), bg::area(poly), 0.0001);
    v = box;
    BOOST_CHECK_CLOSE(bg::area(v), bg::area(box), 0.0001);
}

int test_main(int, char* [])
{

    test_all<bg::model::point<int, 2, bg::cs::cartesian> >();
    test_all<bg::model::point<float, 2, bg::cs::cartesian> >();
    test_all<bg::model::point<double, 2, bg::cs::cartesian> >();

    test_spherical_geo<double>();

    typedef bg::model::point<double, 2, bg::cs::cartesian> pt_crt;
    typedef bg::model::point<double, 2, bg::cs::spherical_equatorial<bg::degree> > pt_sph;
    typedef bg::model::point<double, 2, bg::cs::geographic<bg::degree> > pt_geo;

    test_ccw<pt_crt>();
    test_ccw<pt_sph>();
    test_ccw<pt_geo>();

    test_open<pt_crt>(2.0);
    test_open<pt_sph>(0.00060917296206278821);
    test_open<pt_geo >(24615760871.487991);

    test_open_ccw<pt_crt>(2.0);
    test_open_ccw<pt_sph>(0.00060917296206278821);
    test_open_ccw<pt_geo >(24615760871.487991);

#ifdef HAVE_TTMATH
    test_all<bg::model::d2::point_xy<ttmath_big> >();
    test_spherical_geo<ttmath_big>();
#endif

    test_large_integers();

    test_variant();

    // test_empty_input<bg::model::d2::point_xy<int> >();

    return 0;
}
