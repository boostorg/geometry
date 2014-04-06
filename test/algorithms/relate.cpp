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
#include <boost/geometry/multi/geometries/multi_point.hpp>
#include <boost/geometry/multi/geometries/multi_polygon.hpp>

//TEST
#include <to_svg.hpp>

namespace bgdr = bg::detail::relate;

std::string transposed(std::string matrix)
{
    std::swap(matrix[1], matrix[3]);
    std::swap(matrix[2], matrix[6]);
    std::swap(matrix[5], matrix[7]);
    return matrix;
}

template <typename Geometry1, typename Geometry2>
void check_geometry(Geometry1 const& geometry1,
                    Geometry2 const& geometry2,
                    std::string const& wkt1,
                    std::string const& wkt2,
                    std::string const& expected)
{
    {
        std::string res_str = bgdr::relate<bgdr::matrix9>(geometry1, geometry2);
        bool ok = boost::equal(res_str, expected);
        BOOST_CHECK_MESSAGE(ok,
            "relate: " << wkt1
            << " and " << wkt2
            << " -> Expected: " << expected
            << " detected: " << res_str);
    }

    // changed sequence of geometries - transposed result
    {
        std::string res_str = bgdr::relate(geometry2, geometry1, bgdr::matrix9());
        std::string expected_tr = transposed(expected);
        bool ok = boost::equal(res_str, expected_tr);
        BOOST_CHECK_MESSAGE(ok,
            "relate: " << wkt2
            << " and " << wkt1
            << " -> Expected: " << expected_tr
            << " detected: " << res_str);
    }

    {
        bool result = bgdr::relate(geometry1, geometry2, bgdr::mask9(expected));
        // TODO: SHOULD BE !interrupted - CHECK THIS!
        BOOST_CHECK_MESSAGE(result, 
            "relate: " << wkt1
            << " and " << wkt2
            << " -> Expected: " << expected);
    }

    if ( bg::detail::relate::interruption_enabled<Geometry1, Geometry2>::value )
    {
        // brake the expected output
        std::string expected_interrupt = expected;
        bool changed = false;
        BOOST_FOREACH(char & c, expected_interrupt)
        {
            if ( c >= '0' && c <= '9' )
            {
                if ( c == '0' )
                    c = 'F';
                else
                    --c;

                changed = true;
            }
        }

        if ( changed )
        {
            bool result = bgdr::relate(geometry1, geometry2, bgdr::mask9(expected_interrupt));
            // TODO: SHOULD BE interrupted - CHECK THIS!
            BOOST_CHECK_MESSAGE(!result,
                "relate: " << wkt1
                << " and " << wkt2
                << " -> Expected interrupt for:" << expected_interrupt);
        }
    }
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
void test_point_point()
{
    test_geometry<P, P>("POINT(0 0)", "POINT(0 0)", "0FFFFFFF2");
    test_geometry<P, P>("POINT(1 0)", "POINT(0 0)", "FF0FFF0F2");
}

template <typename P>
void test_point_multipoint()
{
    typedef bg::model::multi_point<P> mpt;

    test_geometry<P, mpt>("POINT(0 0)", "MULTIPOINT(0 0)", "0FFFFFFF2");
    test_geometry<P, mpt>("POINT(1 0)", "MULTIPOINT(0 0)", "FF0FFF0F2");
    test_geometry<P, mpt>("POINT(0 0)", "MULTIPOINT(0 0, 1 0)", "0FFFFF0F2");
}

template <typename P>
void test_multipoint_multipoint()
{
    typedef bg::model::multi_point<P> mpt;

    test_geometry<mpt, mpt>("MULTIPOINT(0 0)", "MULTIPOINT(0 0)", "0FFFFFFF2");
    test_geometry<mpt, mpt>("MULTIPOINT(1 0)", "MULTIPOINT(0 0)", "FF0FFF0F2");
    test_geometry<mpt, mpt>("MULTIPOINT(0 0)", "MULTIPOINT(0 0, 1 0)", "0FFFFF0F2");
    test_geometry<mpt, mpt>("MULTIPOINT(0 0, 1 0)", "MULTIPOINT(0 0)", "0F0FFFFF2");
    test_geometry<mpt, mpt>("MULTIPOINT(0 0, 1 1)", "MULTIPOINT(0 0, 1 0)", "0F0FFF0F2");

    //typedef bg::model::d2::point_xy<float> ptf;
    //typedef bg::model::multi_point<ptf> mptf;
    //test_geometry<mptf, mpt>("MULTIPOINT(0 0)", "MULTIPOINT(0 0)", "0FFFFFFF2");
}

template <typename P>
void test_point_linestring()
{
    typedef bg::model::linestring<P> ls;
    
    test_geometry<P, ls>("POINT(0 0)", "LINESTRING(0 0, 2 2, 3 2)", "F0FFFF102");
    test_geometry<P, ls>("POINT(1 1)", "LINESTRING(0 0, 2 2, 3 2)", "0FFFFF102");
    test_geometry<P, ls>("POINT(3 2)", "LINESTRING(0 0, 2 2, 3 2)", "F0FFFF102");
    test_geometry<P, ls>("POINT(1 0)", "LINESTRING(0 0, 2 2, 3 2)", "FF0FFF102");

    test_geometry<P, ls>("POINT(0 0)", "LINESTRING(0 0, 2 2, 3 2, 0 0)", "0FFFFF1F2");
    test_geometry<P, ls>("POINT(1 1)", "LINESTRING(0 0, 2 2, 3 2, 0 0)", "0FFFFF1F2");
    test_geometry<P, ls>("POINT(3 2)", "LINESTRING(0 0, 2 2, 3 2, 0 0)", "0FFFFF1F2");
    test_geometry<P, ls>("POINT(1 0)", "LINESTRING(0 0, 2 2, 3 2, 0 0)", "FF0FFF1F2");
}

template <typename P>
void test_point_multilinestring()
{
    typedef bg::model::linestring<P> ls;
    typedef bg::model::multi_linestring<ls> mls;

    test_geometry<P, mls>("POINT(0 0)", "MULTILINESTRING((0 0, 2 0, 2 2),(0 0, 0 2))", "0FFFFF102");
    test_geometry<P, mls>("POINT(0 0)", "MULTILINESTRING((0 0, 2 0, 2 2),(0 0, 0 2, 2 2))", "0FFFFF1F2");
    test_geometry<P, mls>("POINT(0 0)", "MULTILINESTRING((0 0, 2 0, 2 2),(0 0, 0 2, 2 2),(0 0, 1 1))", "F0FFFF102");

    test_geometry<P, mls>("POINT(0 0)", "MULTILINESTRING((0 0,5 0),(0 0,0 5,5 0),(0 0,-5 0),(0 0,0 -5,-5 0))", "0FFFFF1F2");
    test_geometry<P, mls>("POINT(5 0)", "MULTILINESTRING((0 0,5 0),(0 0,0 5,5 0),(0 0,-5 0),(0 0,0 -5,-5 0))", "0FFFFF1F2");
    test_geometry<P, mls>("POINT(1 0)", "MULTILINESTRING((0 0,5 0),(0 0,0 5,5 0),(0 0,-5 0),(0 0,0 -5,-5 0))", "0FFFFF1F2");
}

template <typename P>
void test_linestring_linestring()
{
    typedef bg::model::linestring<P> ls;

    test_geometry<ls, ls>("LINESTRING(0 0, 2 2, 3 2)", "LINESTRING(0 0, 2 2, 3 2)", "1FFF0FFF2");
    test_geometry<ls, ls>("LINESTRING(0 0,3 2)", "LINESTRING(0 0, 2 2, 3 2)", "FF1F0F1F2");
    test_geometry<ls, ls>("LINESTRING(1 0,2 2,2 3)", "LINESTRING(0 0, 2 2, 3 2)", "0F1FF0102");

    test_geometry<ls, ls>("LINESTRING(0 0,5 0)", "LINESTRING(0 0,1 1,2 0,2 -1)", "0F1F00102");
    
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

    // spikes
    // FOR NOW DISABLED
    /*test_geometry<ls, ls>("LINESTRING(0 0,10 0)",
                          "LINESTRING(1 0,9 0,2 0)", "101FF0FF2");
    test_geometry<ls, ls>("LINESTRING(0 0,2 2,3 3,1 1)", "LINESTRING(0 0,3 3,6 3)", "1FF00F102");
    test_geometry<ls, ls>("LINESTRING(0 0,2 2,3 3,1 1)", "LINESTRING(0 0,4 4,6 3)", "1FF00F102");*/

    test_geometry<ls, ls>("LINESTRING(0 0,2 2,3 3,4 4)", "LINESTRING(0 0,1 1,4 4)", "1FFF0FFF2");

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

    // self-IP
    test_geometry<ls, ls>("LINESTRING(1 0,9 0)",
                          "LINESTRING(0 0,10 0,10 10,5 0,0 10)",
                          "1FF0FF102");
    test_geometry<ls, ls>("LINESTRING(1 0,5 0,9 0)",
                          "LINESTRING(0 0,10 0,10 10,5 0,0 10)",
                          "1FF0FF102");
    test_geometry<ls, ls>("LINESTRING(1 0,9 0)",
                          "LINESTRING(0 0,10 0,10 10,5 10,5 -1)",
                          "1FF0FF102");
    test_geometry<ls, ls>("LINESTRING(1 0,9 0)",
                          "LINESTRING(0 0,10 0,5 0,5 5)",
                          "1FF0FF102");
    test_geometry<ls, ls>("LINESTRING(1 0,7 0)", "LINESTRING(0 0,10 0,10 10,4 -1)",
                          "1FF0FF102");
    test_geometry<ls, ls>("LINESTRING(1 0,5 0,7 0)", "LINESTRING(0 0,10 0,10 10,4 -1)",
                          "1FF0FF102");
    test_geometry<ls, ls>("LINESTRING(1 0,7 0,8 1)", "LINESTRING(0 0,10 0,10 10,4 -1)",
                          "1F10F0102");
    test_geometry<ls, ls>("LINESTRING(1 0,5 0,7 0,8 1)", "LINESTRING(0 0,10 0,10 10,4 -1)",
                          "1F10F0102");

    // self-IP going out and in on the same point
    test_geometry<ls, ls>("LINESTRING(2 0,5 0,5 5,6 5,5 0,8 0)", "LINESTRING(1 0,9 0)",
                          "1F10FF102");

    // duplicated points
    test_geometry<ls, ls>("LINESTRING(1 1, 2 2, 2 2)", "LINESTRING(0 0, 2 2, 4 2)", "1FF0FF102");
    test_geometry<ls, ls>("LINESTRING(1 1, 1 1, 2 2)", "LINESTRING(0 0, 2 2, 4 2)", "1FF0FF102");

    // linear ring
    test_geometry<ls, ls>("LINESTRING(0 0,10 0)", "LINESTRING(5 0,9 0,5 5,1 0,5 0)", "1F1FF01F2");
    test_geometry<ls, ls>("LINESTRING(0 0,5 0,10 0)", "LINESTRING(5 0,9 0,5 5,1 0,5 0)", "1F1FF01F2");
    test_geometry<ls, ls>("LINESTRING(0 0,5 0,10 0)", "LINESTRING(5 0,10 0,5 5,1 0,5 0)", "1F10F01F2");

    test_geometry<ls, ls>("LINESTRING(0 0,5 0)", "LINESTRING(5 0,10 0,5 5,0 0,5 0)", "1FF0FF1F2");
    test_geometry<ls, ls>("LINESTRING(0 0,5 0)", "LINESTRING(5 0,10 0,5 5,5 0)", "FF10F01F2");

    //to_svg<ls, ls>("LINESTRING(0 0,5 0)", "LINESTRING(5 0,10 0,5 5,5 0)", "test_relate_00.svg");

    // INVALID LINESTRINGS
    // 1-point LS (a Point) NOT disjoint
    //test_geometry<ls, ls>("LINESTRING(1 0)", "LINESTRING(0 0,5 0)", "0FFFFF102");
    //test_geometry<ls, ls>("LINESTRING(0 0,5 0)", "LINESTRING(1 0)", "0F1FF0FF2");
    //test_geometry<ls, ls>("LINESTRING(0 0,5 0)", "LINESTRING(1 0,1 0)", "0F1FF0FF2");
    //test_geometry<ls, ls>("LINESTRING(0 0,5 0)", "LINESTRING(1 0,1 0,1 0)", "0F1FF0FF2");
    // Point/Point
    //test_geometry<ls, ls>("LINESTRING(0 0)", "LINESTRING(0 0)", "0FFFFFFF2");

    // OTHER MASKS
    {
        namespace bgdr = bg::detail::relate;
        ls ls1, ls2, ls3;
        bg::read_wkt("LINESTRING(0 0,2 0)", ls1);
        bg::read_wkt("LINESTRING(2 0,4 0)", ls2);
        bg::read_wkt("LINESTRING(1 0,1 1)", ls3);
        BOOST_CHECK(bgdr::relate(ls1, ls2, bgdr::mask9("FT*******")
                                        || bgdr::mask9("F**T*****")
                                        || bgdr::mask9("F***T****")));
        BOOST_CHECK(bgdr::relate(ls1, ls3, bgdr::mask9("FT*******")
                                        || bgdr::mask9("F**T*****")
                                        || bgdr::mask9("F***T****")));
        BOOST_CHECK(bgdr::relate(ls3, ls1, bgdr::mask9("FT*******")
                                        || bgdr::mask9("F**T*****")
                                        || bgdr::mask9("F***T****")));
    }
}

template <typename P>
void test_linestring_multi_linestring()
{
    typedef bg::model::linestring<P> ls;
    typedef bg::model::multi_linestring<ls> mls;

    // LS disjoint
    test_geometry<ls, mls>("LINESTRING(0 0,10 0)", "MULTILINESTRING((1 0,2 0),(1 1,2 1))", "101FF0102");
    // linear ring disjoint
    test_geometry<ls, mls>("LINESTRING(0 0,10 0)", "MULTILINESTRING((1 0,2 0),(1 1,2 1,2 2,1 1))", "101FF01F2");
    // 2xLS forming non-simple linear ring disjoint
    test_geometry<ls, mls>("LINESTRING(0 0,10 0)", "MULTILINESTRING((1 0,2 0),(1 1,2 1,2 2),(1 1,2 2))", "101FF01F2");

    // INVALID LINESTRINGS
    // 1-point LS (a Point) disjoint
    //test_geometry<ls, mls>("LINESTRING(0 0,10 0)", "MULTILINESTRING((1 0,2 0),(1 1))", "101FF00F2");
    //test_geometry<ls, mls>("LINESTRING(0 0,10 0)", "MULTILINESTRING((1 0,2 0),(1 1,1 1))", "101FF00F2");
    //test_geometry<ls, mls>("LINESTRING(0 0,10 0)", "MULTILINESTRING((1 0,2 0),(1 1,1 1,1 1))", "101FF00F2");
    // 1-point LS (a Point) NOT disjoint
    //test_geometry<ls, mls>("LINESTRING(0 0,10 0)", "MULTILINESTRING((1 0,9 0),(2 0))", "101FF0FF2");
    //test_geometry<ls, mls>("LINESTRING(0 0,10 0)", "MULTILINESTRING((1 0,9 0),(2 0,2 0))", "101FF0FF2");
    //test_geometry<ls, mls>("LINESTRING(0 0,10 0)", "MULTILINESTRING((1 0,9 0),(2 0,2 0,2 0))", "101FF0FF2");
}

template <typename P>
void test_multi_linestring_multi_linestring()
{
    typedef bg::model::linestring<P> ls;
    typedef bg::model::multi_linestring<ls> mls;

    to_svg<mls, mls>("MULTILINESTRING((0 0,0 0,18 0,18 0,19 0,19 0,19 0,30 0,30 0))",
                     "MULTILINESTRING((0 10,5 0,20 0,20 0,30 0))",
                     "mls_mls1.svg");
    to_svg<mls, mls>("MULTILINESTRING((0 0,0 0,18 0,18 0,19 0,19 0,19 0,30 0,30 0))",
                     "MULTILINESTRING((0 10,5 0,20 0,20 0,30 0),(1 10,1 10,1 0,1 0,1 -10),(2 0,2 0),(3 0,3 0,3 0),(0 0,0 0,0 10,0 10),(30 0,30 0,31 0,31 0))",
                     "mls_mls2.svg");
    to_svg<mls, mls>("MULTILINESTRING((0 0,0 0,18 0,18 0,19 0,19 0,19 0,30 0,30 0))",
                     "MULTILINESTRING((0 10,5 0,20 0,20 0,30 0),(0 0,0 0,0 10,0 10))",
                     "mls_mls3.svg");
}

template <typename P>
void test_linestring_polygon()
{
    typedef bg::model::linestring<P> ls;
    typedef bg::model::polygon<P> poly;
    typedef bg::model::ring<P> ring;

    // LS disjoint
    test_geometry<ls, poly>("LINESTRING(11 0,11 10)", "POLYGON((0 0,0 10,10 10,10 0,0 0))", "FF1FF0212");
    test_geometry<ls, ring>("LINESTRING(11 0,11 10)", "POLYGON((0 0,0 10,10 10,10 0,0 0))", "FF1FF0212");

    // II BB
    test_geometry<ls, poly>("LINESTRING(0 0,10 10)", "POLYGON((0 0,0 10,10 10,10 0,0 0))",    "1FFF0F212");
    test_geometry<ls, poly>("LINESTRING(5 0,5 5,10 5)", "POLYGON((0 0,0 10,10 10,10 0,0 0))", "1FFF0F212");
    test_geometry<ls, poly>("LINESTRING(5 1,5 5,9 5)", "POLYGON((0 0,0 10,10 10,10 0,0 0))",  "1FF0FF212");
    
    // IE 
    test_geometry<ls, poly>("LINESTRING(11 1,11 5)", "POLYGON((0 0,0 10,10 10,10 0,0 0))", "FF1FF0212");
    // IE IB0
    test_geometry<ls, poly>("LINESTRING(11 1,10 5)", "POLYGON((0 0,0 10,10 10,10 0,0 0))", "FF1F00212");
    // IE IB1
    test_geometry<ls, poly>("LINESTRING(11 1,10 5,10 10)", "POLYGON((0 0,0 10,10 10,10 0,0 0))", "F11F00212");
    test_geometry<ls, poly>("LINESTRING(11 1,10 10,0 10)", "POLYGON((0 0,0 10,10 10,10 0,0 0))", "F11F00212");
    test_geometry<ls, poly>("LINESTRING(11 1,10 0,0 0)", "POLYGON((0 0,0 10,10 10,10 0,0 0))", "F11F00212");
    test_geometry<ls, poly>("LINESTRING(0 -1,1 0,2 0)", "POLYGON((0 0,0 10,10 10,10 0,0 0))", "F11F00212");
    // IE IB0 II
    test_geometry<ls, poly>("LINESTRING(11 1,10 5,5 5)", "POLYGON((0 0,0 10,10 10,10 0,0 0))", "1010F0212");
    // IE IB0 lring
    test_geometry<ls, poly>("LINESTRING(11 1,10 5,11 5,11 1)", "POLYGON((0 0,0 10,10 10,10 0,0 0))", "F01FFF212");
    // IE IB1 lring
    test_geometry<ls, poly>("LINESTRING(11 1,10 5,10 10,11 5,11 1)", "POLYGON((0 0,0 10,10 10,10 0,0 0))", "F11FFF212");
    
    // IB1 II
    test_geometry<ls, poly>("LINESTRING(0 0,5 0,5 5)", "POLYGON((0 0,0 10,10 10,10 0,0 0))", "11F00F212");
    // BI0 II IB1
    test_geometry<ls, poly>("LINESTRING(5 0,5 5,10 5,10 10)", "POLYGON((0 0,0 10,10 10,10 0,0 0))", "11FF0F212");

    // IB1 II IB1
    test_geometry<ls, poly>("LINESTRING(1 0,2 0,3 1,4 0,5 0)", "POLYGON((0 0,0 10,10 10,10 0,0 0))", "11FF0F212");
    // IB1 IE IB1
    test_geometry<ls, poly>("LINESTRING(1 0,2 0,3 -1,4 0,5 0)", "POLYGON((0 0,0 10,10 10,10 0,0 0))", "F11F0F212");

    // II IB1
    test_geometry<ls, poly>("LINESTRING(5 5,10 5,10 10)", "POLYGON((0 0,0 10,10 10,10 0,0 0))", "11F00F212");
    // IB1 II
    test_geometry<ls, poly>("LINESTRING(10 10,10 5,5 5)", "POLYGON((0 0,0 10,10 10,10 0,0 0))", "11F00F212");
    // IE IB1
    test_geometry<ls, poly>("LINESTRING(15 5,10 5,10 10)", "POLYGON((0 0,0 10,10 10,10 0,0 0))", "F11F00212");
    // IB1 IE
    test_geometry<ls, poly>("LINESTRING(10 10,10 5,15 5)", "POLYGON((0 0,0 10,10 10,10 0,0 0))", "F11F00212");

    // duplicated points
    {
        // II IB0 IE
        test_geometry<ls, poly>("LINESTRING(5 5,10 5,15 10)", "POLYGON((0 0,0 10,10 10,10 0,0 0))", "1010F0212");
        test_geometry<ls, poly>("LINESTRING(5 5,5 5,5 5,10 5,10 5,10 5,15 10,15 10,15 10)",
            "POLYGON((0 0,0 0,0 0,0 10,0 10,0 10,10 10,10 10,10 10,10 0,10 0,10 0,0 0,0 0,0 0))",
            "1010F0212");
        test_geometry<ls, poly>("LINESTRING(5 5,5 5,5 5,10 0,10 0,10 0,15 10,15 10,15 10)",
            "POLYGON((0 0,0 0,0 0,0 10,0 10,0 10,10 10,10 10,10 10,10 0,10 0,10 0,0 0,0 0,0 0))",
            "1010F0212");
        // IE IB0 II
        test_geometry<ls, poly>("LINESTRING(15 10,15 10,15 10,10 5,10 5,10 5,5 5,5 5,5 5)",
            "POLYGON((0 0,0 0,0 0,0 10,0 10,0 10,10 10,10 10,10 10,10 0,10 0,10 0,0 0,0 0,0 0))",
            "1010F0212");
        test_geometry<ls, poly>("LINESTRING(15 10,15 10,15 10,10 0,10 0,10 0,5 5,5 5,5 5)",
            "POLYGON((0 0,0 0,0 0,0 10,0 10,0 10,10 10,10 10,10 10,10 0,10 0,10 0,0 0,0 0,0 0))",
            "1010F0212");

        // TEST
        //test_geometry<ls, poly>("LINESTRING(5 5,5 5,5 5)", "POLYGON((0 0,0 10,10 10,10 0,0 0))", "1010F0212");
        test_geometry<ls, poly>("LINESTRING(5 5,5 5,5 5,15 5,15 5,15 5)", "POLYGON((0 0,0 10,10 10,10 0,0 0))", "1010F0212");
    }

    // non-simple polygon with hole
    test_geometry<ls, poly>("LINESTRING(9 1,10 5,9 9)",
                            "POLYGON((0 0,0 10,10 10,10 0,0 0),(10 5,2 8,2 2,10 5))",
                            "10F0FF212");
    test_geometry<ls, poly>("LINESTRING(10 1,10 5,10 9)",
                            "POLYGON((0 0,0 10,10 10,10 0,0 0),(10 5,2 8,2 2,10 5))",
                            "F1FF0F212");
    test_geometry<ls, poly>("LINESTRING(2 8,10 5,2 2)",
                            "POLYGON((0 0,0 10,10 10,10 0,0 0),(10 5,2 8,2 2,10 5))",
                            "F1FF0F212");
    test_geometry<ls, poly>("LINESTRING(10 1,10 5,2 2)",
                            "POLYGON((0 0,0 10,10 10,10 0,0 0),(10 5,2 8,2 2,10 5))",
                            "F1FF0F212");
    test_geometry<ls, poly>("LINESTRING(10 1,10 5,2 8)",
                            "POLYGON((0 0,0 10,10 10,10 0,0 0),(10 5,2 8,2 2,10 5))",
                            "F1FF0F212");

    // non-simple polygon with hole, linear ring
    test_geometry<ls, poly>("LINESTRING(9 1,10 5,9 9,1 9,1 1,9 1)",
                            "POLYGON((0 0,0 10,10 10,10 0,0 0),(10 5,2 8,2 2,10 5))",
                            "10FFFF212");
    test_geometry<ls, poly>("LINESTRING(10 5,10 9,11 5,10 1,10 5)",
                            "POLYGON((0 0,0 10,10 10,10 0,0 0),(10 5,2 8,2 2,10 5))",
                            "F11FFF212");
    test_geometry<ls, poly>("LINESTRING(11 5,10 1,10 5,10 9,11 5)",
                            "POLYGON((0 0,0 10,10 10,10 0,0 0),(10 5,2 8,2 2,10 5))",
                            "F11FFF212");

    // non-simple polygon with self-touching holes
    test_geometry<ls, poly>("LINESTRING(7 1,8 5,7 9)",
                            "POLYGON((0 0,0 10,10 10,10 0,0 0),(8 1,9 1,9 9,8 9,8 1),(2 2,8 5,2 8,2 2))",
                            "10F0FF212");
    test_geometry<ls, poly>("LINESTRING(8 2,8 5,8 8)",
                            "POLYGON((0 0,0 10,10 10,10 0,0 0),(8 1,9 1,9 9,8 9,8 1),(2 2,8 5,2 8,2 2))",
                            "F1FF0F212");
    test_geometry<ls, poly>("LINESTRING(2 8,8 5,2 2)",
                            "POLYGON((0 0,0 10,10 10,10 0,0 0),(8 1,9 1,9 9,8 9,8 1),(2 2,8 5,2 8,2 2))",
                            "F1FF0F212");

    // non-simple polygon self-touching
    test_geometry<ls, poly>("LINESTRING(9 1,10 5,9 9)",
                            "POLYGON((0 0,0 10,10 10,10 5,2 8,2 2,10 5,10 0,0 0))",
                            "10F0FF212");
    test_geometry<ls, poly>("LINESTRING(10 1,10 5,10 9)",
                            "POLYGON((0 0,0 10,10 10,10 5,2 8,2 2,10 5,10 0,0 0))",
                            "F1FF0F212");
    test_geometry<ls, poly>("LINESTRING(2 8,10 5,2 2)",
                            "POLYGON((0 0,0 10,10 10,10 5,2 8,2 2,10 5,10 0,0 0))",
                            "F1FF0F212");

    // non-simple polygon self-touching, linear ring
    test_geometry<ls, poly>("LINESTRING(9 1,10 5,9 9,1 9,1 1,9 1)",
                            "POLYGON((0 0,0 10,10 10,10 5,2 8,2 2,10 5,10 0,0 0))",
                            "10FFFF212");
    test_geometry<ls, poly>("LINESTRING(10 5,10 9,11 5,10 1,10 5)",
                            "POLYGON((0 0,0 10,10 10,10 5,2 8,2 2,10 5,10 0,0 0))",
                            "F11FFF212");
    test_geometry<ls, poly>("LINESTRING(11 5,10 1,10 5,10 9,11 5)",
                            "POLYGON((0 0,0 10,10 10,10 5,2 8,2 2,10 5,10 0,0 0))",
                            "F11FFF212");

    // polygons with some ring equal to the linestring
    test_geometry<ls, poly>("LINESTRING(0 0,10 0,10 10,0 10,0 0)",
                            "POLYGON((0 0,0 10,10 10,10 0,0 0))",
                            "F1FFFF2F2");
    test_geometry<ls, poly>("LINESTRING(0 0,10 0,10 10,0 10,0 0)",
                            "POLYGON((0 0,0 10,10 10,10 0,0 0),(2 2,5 5,2 8,2 2))",
                            "F1FFFF212");
    test_geometry<ls, poly>("LINESTRING(2 2,5 5,2 8,2 2)",
                            "POLYGON((0 0,0 10,10 10,10 0,0 0),(2 2,5 5,2 8,2 2))",
                            "F1FFFF212");

    // self-IP going on the boundary then into the exterior and to the boundary again
    test_geometry<ls, poly>("LINESTRING(2 10,5 10,5 15,6 15,5 10,8 10)", "POLYGON((0 0,0 10,10 10,10 0,0 0))",
                            "F11F0F212");
    // self-IP going on the boundary then into the interior and to the boundary again
    test_geometry<ls, poly>("LINESTRING(2 10,5 10,5 5,6 5,5 10,8 10)", "POLYGON((0 0,0 10,10 10,10 0,0 0))",
                            "11FF0F212");

    // ccw
    {
        typedef bg::model::polygon<P, false> ccwpoly;

        // IE IB0 II
        test_geometry<ls, ccwpoly>("LINESTRING(11 1,10 5,5 5)", "POLYGON((0 0,10 0,10 10,0 10,0 0))", "1010F0212");
        // IE IB1 II
        test_geometry<ls, ccwpoly>("LINESTRING(11 1,10 1,10 5,5 5)", "POLYGON((0 0,10 0,10 10,0 10,0 0))", "1110F0212");
        test_geometry<ls, ccwpoly>("LINESTRING(11 1,10 5,10 1,5 5)", "POLYGON((0 0,10 0,10 10,0 10,0 0))", "1110F0212");
        // II IB0 IE
        test_geometry<ls, ccwpoly>("LINESTRING(5 1,10 5,11 1)", "POLYGON((0 0,10 0,10 10,0 10,0 0))", "1010F0212");
        // IE IB1 II
        test_geometry<ls, ccwpoly>("LINESTRING(5 5,10 1,10 5,11 5)", "POLYGON((0 0,10 0,10 10,0 10,0 0))", "1110F0212");
        test_geometry<ls, ccwpoly>("LINESTRING(5 5,10 5,10 1,11 5)", "POLYGON((0 0,10 0,10 10,0 10,0 0))", "1110F0212");

    }
    
}

template <typename P>
void test_linestring_multi_polygon()
{
    typedef bg::model::linestring<P> ls;
    typedef bg::model::polygon<P> poly;
    typedef bg::model::multi_polygon<poly> mpoly;

    test_geometry<ls, mpoly>("LINESTRING(10 1,10 5,10 9)",
                            "MULTIPOLYGON(((0 20,0 30,10 30,10 20,0 20)),((0 0,0 10,10 10,10 0,0 0),(10 5,2 8,2 2,10 5)))",
                            "F1FF0F212");
    test_geometry<ls, mpoly>("LINESTRING(10 1,10 5,10 9)",
                            "MULTIPOLYGON(((0 20,0 30,10 30,10 20,0 20)),((0 0,0 10,10 10,10 0,0 0)))",
                            "F1FF0F212");

    test_geometry<ls, mpoly>("LINESTRING(10 1,10 5,2 2)",
                            "MULTIPOLYGON(((0 20,0 30,10 30,10 20,0 20)),((0 0,0 10,10 10,10 0,0 0),(10 5,2 8,2 2,10 5)))",
                            "F1FF0F212");
    test_geometry<ls, mpoly>("LINESTRING(10 1,10 5,2 2)",
                            "MULTIPOLYGON(((0 20,0 30,10 30,10 20,0 20)),((0 0,0 10,10 10,10 0,0 0)))",
                            "11F00F212");

    test_geometry<ls, mpoly>("LINESTRING(10 1,10 5,2 2)",
                            "MULTIPOLYGON(((0 0,0 10,10 10,10 0,0 0),(10 5,2 8,2 2,10 5)),((10 5,3 3,3 7,10 5)))",
                            "F1FF0F212");
    test_geometry<ls, mpoly>("LINESTRING(10 1,10 5,2 8)",
                            "MULTIPOLYGON(((0 0,0 10,10 10,10 0,0 0),(10 5,2 8,2 2,10 5)),((10 5,3 3,3 7,10 5)))",
                            "F1FF0F212");
    test_geometry<ls, mpoly>("LINESTRING(10 1,10 5,3 3)",
                            "MULTIPOLYGON(((0 0,0 10,10 10,10 0,0 0),(10 5,2 8,2 2,10 5)),((10 5,3 3,3 7,10 5)))",
                            "F1FF0F212");
    test_geometry<ls, mpoly>("LINESTRING(10 1,10 5,3 7)",
                            "MULTIPOLYGON(((0 0,0 10,10 10,10 0,0 0),(10 5,2 8,2 2,10 5)),((10 5,3 3,3 7,10 5)))",
                            "F1FF0F212");
    test_geometry<ls, mpoly>("LINESTRING(10 1,10 5,5 5)",
                            "MULTIPOLYGON(((0 0,0 10,10 10,10 0,0 0),(10 5,2 8,2 2,10 5)),((10 5,3 3,3 7,10 5)))",
                            "11F00F212");

    test_geometry<ls, mpoly>("LINESTRING(0 0,10 0,10 10,0 10,0 0)",
                             "MULTIPOLYGON(((0 0,0 10,10 10,10 0,0 0)),((20 0,20 10,30 20,20 0)))",
                             "F1FFFF212");
}

template <typename P>
void test_multi_linestring_multi_polygon()
{
    typedef bg::model::linestring<P> ls;
    typedef bg::model::polygon<P> poly;
    typedef bg::model::multi_linestring<ls> mls;
    typedef bg::model::multi_polygon<poly> mpoly;

    // polygons with some ring equal to the linestrings
    test_geometry<mls, mpoly>("MULTILINESTRING((0 0,10 0,10 10,0 10,0 0),(20 20,50 50,20 80,20 20))",
                              "MULTIPOLYGON(((0 0,0 10,10 10,10 0,0 0)))",
                              "F11FFF2F2");

    test_geometry<mls, mpoly>("MULTILINESTRING((0 0,10 0,10 10,0 10,0 0),(2 2,5 5,2 8,2 2))",
                              "MULTIPOLYGON(((0 0,0 10,10 10,10 0,0 0),(2 2,5 5,2 8,2 2)))",
                              "F1FFFF2F2");


    test_geometry<mls, mpoly>("MULTILINESTRING((0 0,10 0,10 10),(10 10,0 10,0 0))",
                              "MULTIPOLYGON(((0 0,0 10,10 10,10 0,0 0)))",
                              "F1FFFF2F2");
    test_geometry<mls, mpoly>("MULTILINESTRING((0 0,10 0,10 10),(10 10,0 10,0 0),(20 20,50 50,20 80,20 20))",
                              "MULTIPOLYGON(((0 0,0 10,10 10,10 0,0 0)))",
                              "F11FFF2F2");

    // disjoint
    test_geometry<mls, mpoly>("MULTILINESTRING((20 20,30 30),(30 30,40 40))",
                              "MULTIPOLYGON(((0 0,0 10,10 10,10 0,0 0)))",
                              "FF1FF0212");
}

template <typename P>
void polygon_polygon()
{
    typedef bg::model::polygon<P> poly;

    // touching
    test_geometry<poly, poly>("POLYGON((0 0,0 10,10 10,10 0,0 0))",
                              "POLYGON((10 0,10 10,20 10,20 0,10 0))",
                              "FF2F11212");
    test_geometry<poly, poly>("POLYGON((0 0,0 10,10 10,10 0,0 0))",
                              "POLYGON((0 -10,0 0,10 0,10 -10,0 -10))",
                              "FF2F11212");
    test_geometry<poly, poly>("POLYGON((0 0,0 10,10 10,10 0,0 0))",
                              "POLYGON((10 0,15 10,20 10,20 0,10 0))",
                              "FF2F01212");

    // containing
    test_geometry<poly, poly>("POLYGON((0 0,0 10,10 10,10 0,0 0))",
                              "POLYGON((5 5,5 10,6 10,6 5,5 5))",
                              "212F11FF2");
    test_geometry<poly, poly>("POLYGON((0 0,0 10,10 10,10 0,0 0))",
                              "POLYGON((5 5,5 10,6 5,5 5))",
                              "212F01FF2");
    test_geometry<poly, poly>("POLYGON((0 0,0 10,10 10,10 0,0 0))",
                              "POLYGON((5 5,5 6,6 6,6 5,5 5))",
                              "212FF1FF2");

    // fully containing
    test_geometry<poly, poly>("POLYGON((0 0,0 10,10 10,10 0,0 0))",
                              "POLYGON((5 5,5 9,6 9,6 5,5 5))",
                              "212FF1FF2");
    // fully containing, with a hole
    test_geometry<poly, poly>("POLYGON((0 0,0 10,10 10,10 0,0 0),(3 3,7 3,7 7,3 7,3 3))",
                              "POLYGON((1 1,1 9,9 9,9 1,1 1))",
                              "2121F12F2");
    // fully containing, both with holes
    test_geometry<poly, poly>("POLYGON((0 0,0 10,10 10,10 0,0 0),(3 3,7 3,7 7,3 7,3 3))",
                              "POLYGON((1 1,1 9,9 9,9 1,1 1),(2 2,8 2,8 8,2 8,2 2))",
                              "212FF1FF2");
    // fully containing, both with holes
    test_geometry<poly, poly>("POLYGON((0 0,0 10,10 10,10 0,0 0),(3 3,7 3,7 7,3 7,3 3))",
                              "POLYGON((1 1,1 9,9 9,9 1,1 1),(4 4,6 4,6 6,4 6,4 4))",
                              "2121F12F2");

    // overlapping
    test_geometry<poly, poly>("POLYGON((0 0,0 10,10 10,10 0,0 0))",
                              "POLYGON((5 0,5 10,20 10,20 0,5 0))",
                              "212111212");
    test_geometry<poly, poly>("POLYGON((0 0,0 10,10 10,15 5,10 0,0 0))",
                              "POLYGON((10 0,5 5,10 10,20 10,20 0,10 0))",
                              "212101212");

    // equal
    test_geometry<poly, poly>("POLYGON((0 0,0 10,10 10,10 0,0 0))",
                              "POLYGON((10 10,10 5,10 0,5 0,0 0,0 10,5 10,10 10))",
                              "2FFF1FFF2");

    // disjoint
    test_geometry<poly, poly>("POLYGON((0 0,0 10,10 10,10 0,0 0))",
                              "POLYGON((0 20,0 30,10 30,10 20,0 20))",
                              "FF2FF1212");
    // disjoint
    test_geometry<poly, poly>("POLYGON((0 0,0 10,10 10,10 0,0 0),(3 3,7 3,7 7,3 7,3 3))",
                              "POLYGON((0 20,0 30,10 30,10 20,0 20))",
                              "FF2FF1212");

    // equal non-simple / non-simple hole
    test_geometry<poly, poly>("POLYGON((0 0,0 10,10 10,10 0,0 0),(5 5,10 5,5 6,5 5))",
                              "POLYGON((0 0,0 10,10 10,10 5,5 6,5 5,10 5,10 0,0 0))",
                              "2FFF1FFF2");

    // within non-simple / simple
    test_geometry<poly, poly>("POLYGON((0 0,0 10,10 10,10 5,5 6,5 5,10 5,10 0,0 0))",
                              "POLYGON((0 0,5 5,10 5,10 0,0 0))",
                              "212F11FF2");
    // within non-simple hole / simple
    test_geometry<poly, poly>("POLYGON((0 0,0 10,10 10,10 0,0 0),(5 5,10 5,5 6,5 5))",
                              "POLYGON((0 0,5 5,10 5,10 0,0 0))",
                              "212F11FF2");


    // not within non-simple / simple
    test_geometry<poly, poly>("POLYGON((0 0,0 10,10 10,10 5,5 6,5 5,10 5,10 0,0 0))",
                              "POLYGON((0 0,0 10,10 10,10 0,0 0))",
                              "2FF11F2F2");
    // not within non-simple hole / simple
    test_geometry<poly, poly>("POLYGON((0 0,0 10,10 10,10 0,0 0),(5 5,10 5,5 6,5 5))",
                              "POLYGON((0 0,0 10,10 10,10 0,0 0))",
                              "2FF11F2F2");
    // not within simple hole / simple
    test_geometry<poly, poly>("POLYGON((0 0,0 10,10 10,10 0,0 0),(5 5,9 5,5 6,5 5))",
                              "POLYGON((0 0,0 10,10 10,9 5,10 0,0 0))",
                              "2121112F2");

    // within non-simple fake hole / simple
    test_geometry<poly, poly>("POLYGON((0 0,0 10,10 10,10 5,4 7,4 3,10 5,10 0,0 0))",
                              "POLYGON((0 0,0 10,10 10,10 0,0 0))",
                              "2FF11F2F2");
    // within non-simple fake hole / non-simple fake hole
    test_geometry<poly, poly>("POLYGON((0 0,0 10,10 10,10 5,4 7,4 3,10 5,10 0,0 0))",
                              "POLYGON((0 0,0 10,10 10,10 5,5 6,5 4,10 5,10 0,0 0))",
                              "2FF11F212");
    // within non-simple fake hole / non-simple hole
    test_geometry<poly, poly>("POLYGON((0 0,0 10,10 10,10 5,4 7,4 3,10 5,10 0,0 0))",
                              "POLYGON((0 0,0 10,10 10,10 0,0 0),(10 5,5 6,5 4,10 5))",
                              "2FF11F212");
    // containing non-simple fake hole / non-simple hole
    test_geometry<poly, poly>("POLYGON((0 0,0 10,10 10,10 5,4 7,4 3,10 5,10 0,0 0))",
                              "POLYGON((0 0,0 10,10 10,10 0,0 0),(10 5,3 8,3 2,10 5))",
                              "212F11FF2");

    // within non-simple hole / simple
    test_geometry<poly, poly>("POLYGON((0 0,0 10,10 10,10 0,0 0),(10 5,4 7,4 3,10 5))",
                              "POLYGON((0 0,0 10,10 10,10 0,0 0))",
                              "2FF11F2F2");
    // within non-simple hole / non-simple fake hole
    test_geometry<poly, poly>("POLYGON((0 0,0 10,10 10,10 0,0 0),(10 5,4 7,4 3,10 5))",
                              "POLYGON((0 0,0 10,10 10,10 5,5 6,5 4,10 5,10 0,0 0))",
                              "2FF11F212");
    // containing non-simple hole / non-simple fake hole
    test_geometry<poly, poly>("POLYGON((0 0,0 10,10 10,10 0,0 0),(10 5,4 7,4 3,10 5))",
                              "POLYGON((0 0,0 10,10 10,10 5,3 8,3 2,10 5,10 0,0 0))",
                              "212F11FF2");
    // equal non-simple hole / non-simple fake hole
    test_geometry<poly, poly>("POLYGON((0 0,0 10,10 10,10 0,0 0),(10 5,4 7,4 3,10 5))",
                              "POLYGON((0 0,0 10,10 10,10 5,4 7,4 3,10 5,10 0,0 0))",
                              "2FFF1FFF2");
    // within non-simple hole / non-simple hole
    test_geometry<poly, poly>("POLYGON((0 0,0 10,10 10,10 0,0 0),(10 5,4 7,4 3,10 5))",
                              "POLYGON((0 0,0 10,10 10,10 0,0 0),(10 5,5 6,5 4,10 5))",
                              "2FF11F212");
    // containing non-simple hole / non-simple hole
    test_geometry<poly, poly>("POLYGON((0 0,0 10,10 10,10 0,0 0),(10 5,4 7,4 3,10 5))",
                              "POLYGON((0 0,0 10,10 10,10 0,0 0),(10 5,3 8,3 2,10 5))",
                              "212F11FF2");
    // equal non-simple hole / non-simple hole
    test_geometry<poly, poly>("POLYGON((0 0,0 10,10 10,10 0,0 0),(10 5,4 7,4 3,10 5))",
                              "POLYGON((0 0,0 10,10 10,10 0,0 0),(10 5,4 7,4 3,10 5))",
                              "2FFF1FFF2");

    // intersecting non-simple hole / non-simple hole - touching holes
    test_geometry<poly, poly>("POLYGON((0 0,0 10,10 10,10 0,0 0),(5 5,10 5,5 6,5 5))",
                              "POLYGON((0 0,0 10,10 10,10 0,0 0),(0 5,5 4,5 5,0 5))",
                              "21211F2F2");
    // intersecting non-simple fake hole / non-simple hole - touching holes
    test_geometry<poly, poly>("POLYGON((0 0,0 10,10 10,10 5,5 6,5 5,10 5,10 0,0 0))",
                              "POLYGON((0 0,0 10,10 10,10 0,0 0),(0 5,5 4,5 5,0 5))",
                              "21211F2F2");
    // intersecting non-simple fake hole / non-simple fake hole - touching holes
    test_geometry<poly, poly>("POLYGON((0 0,0 10,10 10,10 5,5 6,5 5,10 5,10 0,0 0))",
                              "POLYGON((0 0,0 5,5 4,5 5,0 5,0 10,10 10,10 0,0 0))",
                              "21211F2F2");

    // intersecting simple - i/i
    test_geometry<poly, poly>("POLYGON((0 0,0 10,4 10,6 8,5 5,6 2,4 0,0 0))",
                              "POLYGON((5 5,4 8,6 10,10 10,10 0,6 0,4 2,5 5))",
                              "212101212");
    // intersecting non-simple hole / non-simple hole - i/i
    test_geometry<poly, poly>("POLYGON((0 0,0 10,4 10,6 8,5 5,6 2,4 0,0 0),(5 5,2 6,2 4,5 5))",
                              "POLYGON((5 5,4 8,6 10,10 10,10 0,6 0,4 2,5 5),(5 5,8 4,8 6,5 5))",
                              "212101212");
    // intersecting non-simple hole / simple - i/i
    test_geometry<poly, poly>("POLYGON((0 0,0 10,4 10,6 8,5 5,6 2,4 0,0 0),(5 5,2 6,2 4,5 5))",
                              "POLYGON((5 5,4 8,6 10,10 10,10 0,6 0,4 2,5 5))",
                              "212101212");
}

template <typename P>
void multi_polygon_multi_polygon()
{
    typedef bg::model::polygon<P> poly;
    typedef bg::model::multi_polygon<poly> mpoly;

    test_geometry<mpoly, mpoly>("MULTIPOLYGON(((0 0,0 10,10 10,10 0,0 0)))",
                                "MULTIPOLYGON(((5 5,5 10,6 10,6 5,5 5)),((0 20,0 30,10 30,10 20,0 20)))",
                                "212F11212");
    test_geometry<mpoly, mpoly>("MULTIPOLYGON(((0 0,0 10,10 10,10 0,0 0)),((0 20,0 30,10 30,10 20,0 20)))",
                                "MULTIPOLYGON(((5 5,5 10,6 10,6 5,5 5)))",
                                "212F11FF2");

    test_geometry<mpoly, mpoly>("MULTIPOLYGON(((0 0,0 10,10 10,10 0,0 0)))",
                                "MULTIPOLYGON(((5 5,5 6,6 6,6 5,5 5)),((0 20,0 30,10 30,10 20,0 20)))",
                                "212FF1212");
    test_geometry<mpoly, mpoly>("MULTIPOLYGON(((0 0,0 10,10 10,10 0,0 0)),((0 20,0 30,10 30,10 20,0 20)))",
                                "MULTIPOLYGON(((5 5,5 6,6 6,6 5,5 5)))",
                                "212FF1FF2");
}

template <typename P>
void test_all()
{
    test_point_point<P>();
    test_point_multipoint<P>();
    test_multipoint_multipoint<P>();
    test_point_linestring<P>();
    test_point_multilinestring<P>();
    test_linestring_linestring<P>();
    test_linestring_multi_linestring<P>();
    test_multi_linestring_multi_linestring<P>();
    test_linestring_polygon<P>();
    test_linestring_multi_polygon<P>();
    test_multi_linestring_multi_polygon<P>();
    polygon_polygon<P>();
    multi_polygon_multi_polygon<P>();
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
