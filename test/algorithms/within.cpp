// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright (c) 2007-2011 Barend Gehrels, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <algorithms/test_within.hpp>


#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/geometries/point_xy.hpp>


template <typename P>
void test_all()
{
    // trivial case
    test_ring<P>("POINT(1 1)", "POLYGON((0 0,0 2,2 2,2 0,0 0))", true, false);

    // on border/corner
    test_ring<P>("POINT(0 0)", "POLYGON((0 0,0 2,2 2,2 0,0 0))", false, true);
    test_ring<P>("POINT(0 1)", "POLYGON((0 0,0 2,2 2,2 0,0 0))", false, true);

    // aligned to segment/vertex
    test_ring<P>("POINT(1 1)", "POLYGON((0 0,0 3,3 3,3 1,2 1,2 0,0 0))", true, false);
    test_ring<P>("POINT(1 1)", "POLYGON((0 0,0 3,4 3,3 1,2 2,2 0,0 0))", true, false);

    // same polygon, but point on border
    test_ring<P>("POINT(3 3)", "POLYGON((0 0,0 3,3 3,3 1,2 1,2 0,0 0))", false, true);
    test_ring<P>("POINT(3 3)", "POLYGON((0 0,0 3,4 3,3 1,2 2,2 0,0 0))", false, true);

    // holes
    test_geometry<P, bg::model::polygon<P> >("POINT(2 2)",
        "POLYGON((0 0,0 4,4 4,4 0,0 0),(1 1,3 1,3 3,1 3,1 1))", false);

    typedef bg::model::box<P> box_type;

    test_geometry<P, box_type>("POINT(1 1)", "BOX(0 0,2 2)", true);

    test_geometry<box_type, box_type>("BOX(1 1,2 2)", "BOX(0 0,3 3)", true);
    test_geometry<box_type, box_type>("BOX(0 0,3 3)", "BOX(1 1,2 2)", false);


    // Mixed point types
    test_geometry
        <
            bg::model::d2::point_xy<float>,
            bg::model::polygon<P>
        >("POINT(1 1)", "POLYGON((0 0,0 2,2 2,2 0,0 0))", true);


    // Real-life problem (solved now), point is in the middle, 409623 is also a coordinate
    // on the border, has been wrong in the past (2009)
    test_ring<P>("POINT(146383 409623)",
        "POLYGON((146351 410597,146521 410659,147906 410363,148088 410420"
        ",148175 410296,148281 409750,148215 409623,148154 409666,148154 409666"
        ",148130 409625,148035 409626,148035 409626,148008 409544,147963 409510"
        ",147993 409457,147961 409352,147261 408687,147008 408586,145714 408840"
        ",145001 409033,144486 409066,144616 409308,145023 410286,145254 410488"
        ",145618 410612,145618 410612,146015 410565,146190 410545,146351 410597))",
        true, false);
}

template <typename Point>
void test_spherical()
{
    typedef typename bg::coordinate_type<Point>::type ct;
    bg::model::polygon<Point> wrangel;

    // SQL Server check (no geography::STWithin, so check with intersection trick)
    /*

    with q as (
    select geography::STGeomFromText('POLYGON((-178.569 71.5641,-179.034 71.5977,-179.305 71.5514,-179.629 71.5772,-180 71.5358,179.53 71.4383,178.872 71.2175,178.618 71.0355,178.791 70.7964,179.273 70.8886,179.678 70.8955,-180 70.9972,-179.274 70.9078,-178.819 70.98,-177.939 71.0375,-177.62 71.1166,-177.439 71.2269,-177.503 71.2775,-177.833 71.3461,-178.018 71.4497,-178.569 71.5641))',4326) as wrangel
    )

    select wrangel.STArea()/1000000.0
    ,geography::STGeomFromText('POINT(-179.3 71.27)',4326).STIntersection(wrangel).STAsText() as workaround_within_1
    ,geography::STGeomFromText('POINT(-179.9 70.95)',4326).STIntersection(wrangel).STAsText() as workaround_within_2
    ,geography::STGeomFromText('POINT(179.9 70.95)',4326).STIntersection(wrangel).STAsText() as workaround_within_3
    from q

    -> 7669.10402181435 POINT (-179.3 71.27) GEOMETRYCOLLECTION EMPTY GEOMETRYCOLLECTION EMPTY

    PostGIS knows Within for Geography neither, and the intersection trick gives the same result

    */

    bg::read_wkt("POLYGON((-178.568604 71.564148,-178.017548 71.449692,-177.833313 71.3461,-177.502838 71.277466 ,-177.439453 71.226929,-177.620026 71.116638,-177.9389 71.037491,-178.8186 70.979965,-179.274445 70.907761,-180 70.9972,179.678314 70.895538,179.272766 70.888596,178.791016 70.7964,178.617737 71.035538,178.872192 71.217484,179.530273 71.4383 ,-180 71.535843 ,-179.628601 71.577194,-179.305298 71.551361,-179.03421 71.597748,-178.568604 71.564148))", wrangel);

    bool within = bg::within(Point(-179.3, 71.27), wrangel);
    BOOST_CHECK_EQUAL(within, true);

    within = bg::within(Point(-179.9, 70.95), wrangel);
    BOOST_CHECK_EQUAL(within, false);

    within = bg::within(Point(179.9, 70.95), wrangel);
    BOOST_CHECK_EQUAL(within, false);

    // Test using great circle mapper
    // http://www.gcmap.com/mapui?P=5E52N-9E53N-7E50N-5E52N,7E52.5N,8E51.5N,6E51N

    bg::model::polygon<Point> triangle;
    bg::read_wkt("POLYGON((5 52,9 53,7 50,5 52))", triangle);
    BOOST_CHECK_EQUAL(bg::within(Point(7, 52.5), triangle), true);
    BOOST_CHECK_EQUAL(bg::within(Point(8.0, 51.5), triangle), false);
    BOOST_CHECK_EQUAL(bg::within(Point(6.0, 51.0), triangle), false);
}


int test_main( int , char* [] )
{
    test_all<bg::model::d2::point_xy<int> >();
    test_all<bg::model::d2::point_xy<double> >();

    test_spherical<bg::model::point<double, 2, bg::cs::spherical_equatorial<bg::degree> > >();


#if defined(HAVE_TTMATH)
    test_all<bg::model::d2::point_xy<ttmath_big> >();
    test_spherical<bg::model::point<ttmath_big, 2, bg::cs::spherical_equatorial<bg::degree> > >();
#endif

    return 0;
}
