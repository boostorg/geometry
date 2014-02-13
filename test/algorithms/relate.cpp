// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2013, 2014.
// Modifications copyright (c) 2013-2014 Oracle and/or its affiliates.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

#include <geometry_test_common.hpp>

#include <boost/geometry/core/ring_type.hpp>
#include <boost/geometry/algorithms/within.hpp>
#include <boost/geometry/strategies/strategies.hpp>
#include <boost/geometry/geometries/ring.hpp>
#include <boost/geometry/geometries/polygon.hpp>

#include <boost/geometry/io/wkt/read.hpp>

#include <boost/geometry/strategies/cartesian/point_in_box.hpp>
#include <boost/geometry/strategies/cartesian/box_in_box.hpp>
#include <boost/geometry/strategies/agnostic/point_in_box_by_side.hpp>

#include <boost/geometry/multi/algorithms/covered_by.hpp>
#include <boost/geometry/multi/geometries/multi_linestring.hpp>
#include <boost/geometry/multi/io/wkt/read.hpp>

#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/geometries/point_xy.hpp>

#include <boost/geometry/algorithms/detail/relate/relate.hpp>

#include <boost/geometry.hpp>
#include <boost/geometry/multi/geometries/multi_linestring.hpp>

//TEST
#include <to_svg.hpp>

namespace bgdr = bg::detail::relate;

template <typename Geometry1, typename Geometry2>
void check_geometry(Geometry1 const& geometry1,
                    Geometry2 const& geometry2,
                    std::string const& wkt1,
                    std::string const& wkt2,
                    std::string const& expected)
{
    bgdr::result res = bgdr::relate(geometry1, geometry2);
    std::string res_str(boost::begin(res.get_code()), boost::end(res.get_code()));
    bool ok = boost::equal(res_str, expected);

    BOOST_CHECK_MESSAGE(ok,
        "relate: " << wkt1
        << " and " << wkt2
        << " -> Expected: " << expected
        << " detected: " << res_str);
}

template <typename Geometry1, typename Geometry2>
void test_geometry(std::string const& wkt1,
                   std::string const& wkt2,
                   std::string const& expected)
{
    Geometry1 geometry1;
    Geometry2 geometry2;
    bg::read_wkt(wkt1, geometry1);
    bg::read_wkt(wkt2, geometry2);
    check_geometry(geometry1, geometry2, wkt1, wkt2, expected);
}

template <typename P>
void test_linestring_linestring()
{
    typedef bg::model::linestring<P> ls;

    test_geometry<ls, ls>("LINESTRING(0 0, 2 2, 3 2)", "LINESTRING(0 0, 2 2, 3 2)", "1FFF0FFF2");
    test_geometry<ls, ls>("LINESTRING(0 0,3 2)", "LINESTRING(0 0, 2 2, 3 2)", "FF1F0F1F2");
    
    test_geometry<ls, ls>("LINESTRING(0 0, 1 1, 2 2, 3 2)", "LINESTRING(0 0, 2 2, 3 2)", "1FFF0FFF2");
    test_geometry<ls, ls>("LINESTRING(3 2, 2 2, 1 1, 0 0)", "LINESTRING(0 0, 2 2, 3 2)", "1FFF0FFF2");
    test_geometry<ls, ls>("LINESTRING(0 0, 1 1, 2 2, 3 2)", "LINESTRING(3 2, 2 2, 0 0)", "1FFF0FFF2");
    test_geometry<ls, ls>("LINESTRING(3 2, 2 2, 1 1, 0 0)", "LINESTRING(3 2, 2 2, 0 0)", "1FFF0FFF2");

    test_geometry<ls, ls>("LINESTRING(3 1, 2 2, 1 1, 0 0)", "LINESTRING(0 0, 2 2, 3 2)", "1F1F00102");
    test_geometry<ls, ls>("LINESTRING(3 3, 2 2, 1 1, 0 0)", "LINESTRING(0 0, 2 2, 3 2)", "1F1F00102");

    test_geometry<ls, ls>("LINESTRING(0 0, 1 1, 2 2, 2 3)", "LINESTRING(0 0, 2 2, 2 3)", "1FFF0FFF2");
    test_geometry<ls, ls>("LINESTRING(2 3, 2 2, 1 1, 0 0)", "LINESTRING(0 0, 2 2, 2 3)", "1FFF0FFF2");
    test_geometry<ls, ls>("LINESTRING(0 0, 1 1, 2 2, 2 3)", "LINESTRING(2 3, 2 2, 0 0)", "1FFF0FFF2");
    test_geometry<ls, ls>("LINESTRING(2 3, 2 2, 1 1, 0 0)", "LINESTRING(2 3, 2 2, 0 0)", "1FFF0FFF2");

    test_geometry<ls, ls>("LINESTRING(1 1, 2 2, 3 2)", "LINESTRING(0 0, 2 2, 4 2)", "1FF0FF102");

    test_geometry<ls, ls>("LINESTRING(3 2, 2 2, 1 1)", "LINESTRING(0 0, 2 2, 4 2)", "1FF0FF102");
    test_geometry<ls, ls>("LINESTRING(1 1, 2 2, 3 2)", "LINESTRING(4 2, 2 2, 0 0)", "1FF0FF102");
    test_geometry<ls, ls>("LINESTRING(3 2, 2 2, 1 1)", "LINESTRING(4 2, 2 2, 0 0)", "1FF0FF102");

//    test_geometry<ls, ls>("LINESTRING(1 1, 2 2, 2 2)", "LINESTRING(0 0, 2 2, 4 2)", true);

//    test_geometry<ls, ls>("LINESTRING(1 1, 2 2, 3 3)", "LINESTRING(0 0, 2 2, 4 2)", false);
//    test_geometry<ls, ls>("LINESTRING(1 1, 2 2, 3 2, 3 3)", "LINESTRING(0 0, 2 2, 4 2)", false);
//    test_geometry<ls, ls>("LINESTRING(1 1, 2 2, 3 1)", "LINESTRING(0 0, 2 2, 4 2)", false);
//    test_geometry<ls, ls>("LINESTRING(1 1, 2 2, 3 2, 3 1)", "LINESTRING(0 0, 2 2, 4 2)", false);

//    test_geometry<ls, ls>("LINESTRING(0 1, 1 1, 2 2, 3 2)", "LINESTRING(0 0, 2 2, 4 2)", false);
//    test_geometry<ls, ls>("LINESTRING(0 1, 0 0, 2 2, 3 2)", "LINESTRING(0 0, 2 2, 4 2)", false);
//    test_geometry<ls, ls>("LINESTRING(1 0, 1 1, 2 2, 3 2)", "LINESTRING(0 0, 2 2, 4 2)", false);
//    test_geometry<ls, ls>("LINESTRING(1 0, 0 0, 2 2, 3 2)", "LINESTRING(0 0, 2 2, 4 2)", false);

//    test_geometry<ls, ls>("LINESTRING(0 0)", "LINESTRING(0 0)", false);
//    test_geometry<ls, ls>("LINESTRING(1 1)", "LINESTRING(0 0, 2 2)", true);
//    test_geometry<ls, ls>("LINESTRING(0 0)", "LINESTRING(0 0, 2 2)", false);
//    test_geometry<ls, ls>("LINESTRING(0 0, 1 1)", "LINESTRING(0 0)", false);

//    test_geometry<ls, ls>("LINESTRING(0 0,5 0,3 0,6 0)", "LINESTRING(0 0,6 0)", true);
//    test_geometry<ls, ls>("LINESTRING(0 0,2 2,3 3,1 1)", "LINESTRING(0 0,3 3,6 3)", true);

    test_geometry<ls, ls>("LINESTRING(0 0,2 2,3 3,1 1,5 3)", "LINESTRING(0 0,3 3,6 3)", "1F100F102");
    test_geometry<ls, ls>("LINESTRING(5 3,1 1,3 3,2 2,0 0)", "LINESTRING(0 0,3 3,6 3)", "1F100F102");
    test_geometry<ls, ls>("LINESTRING(0 0,2 2,3 3,1 1,5 3)", "LINESTRING(6 3,3 3,0 0)", "1F100F102");
    test_geometry<ls, ls>("LINESTRING(5 3,1 1,3 3,2 2,0 0)", "LINESTRING(6 3,3 3,0 0)", "1F100F102");

    // spike
    test_geometry<ls, ls>("LINESTRING(0 0,10 0)",
                          "LINESTRING(1 0,9 0,2 0)", "101FF0FF2");

    // loop i/i i/i u/u u/u
    test_geometry<ls, ls>("LINESTRING(0 0,10 0)",
                          "LINESTRING(1 1,1 0,6 0,6 1,4 1,4 0,9 0,9 1)", "1F1FF0102");

    // self-intersecting and self-touching equal
    test_geometry<ls, ls>("LINESTRING(0 5,5 5,10 5,10 10,5 10,5 5,5 0)",
                          "LINESTRING(0 5,5 5,5 10,10 10,10 5,5 5,5 0)", "1FFF0FFF2");
    // self-intersecting loop and self-touching equal
    test_geometry<ls, ls>("LINESTRING(0 5,5 5,10 5,10 10,5 10,5 5,10 5,10 10,5 10,5 5,5 0)",
                          "LINESTRING(0 5,5 5,5 10,10 10,10 5,5 5,5 0)", "1FFF0FFF2");

    test_geometry<ls, ls>("LINESTRING(0 0,1 1)", "LINESTRING(0 1,1 0)", "0F1FF0102");
    test_geometry<ls, ls>("LINESTRING(0 0,1 1)", "LINESTRING(1 1,2 0)", "FF1F00102");
    test_geometry<ls, ls>("LINESTRING(0 0,1 1)", "LINESTRING(2 0,1 1)", "FF1F00102");

    test_geometry<ls, ls>("LINESTRING(0 0,1 0,2 1,3 5,4 0)", "LINESTRING(1 0,2 1,3 5)", "101FF0FF2");
    test_geometry<ls, ls>("LINESTRING(0 0,1 0,2 1,3 5,4 0)", "LINESTRING(3 5,2 1,1 0)", "101FF0FF2");
    test_geometry<ls, ls>("LINESTRING(1 0,2 1,3 5)", "LINESTRING(4 0,3 5,2 1,1 0,0 0)", "1FF0FF102");
    test_geometry<ls, ls>("LINESTRING(3 5,2 1,1 0)", "LINESTRING(4 0,3 5,2 1,1 0,0 0)", "1FF0FF102");
    
    test_geometry<ls, ls>("LINESTRING(0 0,10 0)", "LINESTRING(-1 -1,1 0,10 0,20 -1)", "1F10F0102");
    test_geometry<ls, ls>("LINESTRING(0 0,10 0)", "LINESTRING(20 -1,10 0,1 0,-1 -1)", "1F10F0102");

    test_geometry<ls, ls>("LINESTRING(-1 1,0 0,1 0,5 0,5 5,10 5,15 0,31 0)",
                          "LINESTRING(-1 -1,0 0,1 0,2 0,3 1,4 0,30 0)",
                          "101FF0102");
    test_geometry<ls, ls>("LINESTRING(-1 1,0 0,1 0,5 0,5 5,10 5,15 0,31 0)",
                          "LINESTRING(30 0,4 0,3 1,2 0,1 0,0 0,-1 -1)",
                          "101FF0102");
    test_geometry<ls, ls>("LINESTRING(31 0,15 0,10 5,5 5,5 0,1 0,0 0,-1 1)",
                          "LINESTRING(-1 -1,0 0,1 0,2 0,3 1,4 0,30 0)",
                          "101FF0102");
    test_geometry<ls, ls>("LINESTRING(31 0,15 0,10 5,5 5,5 0,1 0,0 0,-1 1)",
                          "LINESTRING(30 0,4 0,3 1,2 0,1 0,0 0,-1 -1)",
                          "101FF0102");

    // linear ring
    test_geometry<ls, ls>("LINESTRING(0 0,10 0)", "LINESTRING(5 0,9 0,5 5,1 0,5 0)", "1F1FF01F2");
    test_geometry<ls, ls>("LINESTRING(0 0,5 0,10 0)", "LINESTRING(5 0,9 0,5 5,1 0,5 0)", "1F1FF01F2");
    test_geometry<ls, ls>("LINESTRING(0 0,5 0,10 0)", "LINESTRING(5 0,10 0,5 5,1 0,5 0)", "1F10F01F2");

    // ERROR: x/x and x/u are generated for one point, after handling of x the algorithm thinks that G1 is outside
    // and therefore thinks that it touches G2 at the next x
    test_geometry<ls, ls>("LINESTRING(0 0,5 0)", "LINESTRING(5 0,10 0,5 5,0 0,5 0)", "1FF0FF1F2");
    // AS ABOVE
    test_geometry<ls, ls>("LINESTRING(0 0,5 0)", "LINESTRING(5 0,10 0,5 5,5 0)", "FF00F01F2");

    //to_svg<ls, ls>("LINESTRING(0 0,5 0)", "LINESTRING(5 0,10 0,5 5,0 0,5 0)", "test_relate_00.svg");
}

template <typename P>
void test_all()
{
    test_linestring_linestring<P>();
}

int test_main( int , char* [] )
{
    test_all<bg::model::d2::point_xy<int> >();
    test_all<bg::model::d2::point_xy<double> >();

#if defined(HAVE_TTMATH)
    test_all<bg::model::d2::point_xy<ttmath_big> >();
#endif

    return 0;
}
