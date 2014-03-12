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
            "relate: " << wkt1
            << " and " << wkt2
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
void test_linestring_polygon()
{
    typedef bg::model::linestring<P> ls;
    typedef bg::model::polygon<P> poly;

    // LS disjoint
    test_geometry<ls, poly>("LINESTRING(11 0,11 10)", "POLYGON((0 0,0 10,10 10,10 0,0 0))", "FF1FF0212");
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
    test_linestring_polygon<P>();
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
