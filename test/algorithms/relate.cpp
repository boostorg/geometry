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

    to_svg<ls, ls>("LINESTRING(0 0,2 0)", "LINESTRING(1 0,3 0)", "lsls0000.svg");
    to_svg<ls, ls>("LINESTRING(1 0,3 0)", "LINESTRING(2 0,0 0)", "lsls0001.svg");
    to_svg<ls, ls>("LINESTRING(0 0,2 0)", "LINESTRING(3 0,1 0)", "lsls0002.svg");

    to_svg<ls, ls>("LINESTRING(0 0,1 0)", "LINESTRING(1 0,2 0)", "lsls0003.svg");
    to_svg<ls, ls>("LINESTRING(0 0,1 0)", "LINESTRING(2 0,1 0)", "lsls0004.svg");
    to_svg<ls, ls>("LINESTRING(1 0,2 0)", "LINESTRING(1 0,0 0)", "lsls0005.svg");

    to_svg<ls, ls>("LINESTRING(0 0,2 0)", "LINESTRING(0 0,2 0)", "lsls0006.svg");
    to_svg<ls, ls>("LINESTRING(0 0,2 0)", "LINESTRING(2 0,0 0)", "lsls0007.svg");

    to_svg<ls, ls>("LINESTRING(0 0,3 0)", "LINESTRING(1 0,2 0)", "lsls0008.svg");
    to_svg<ls, ls>("LINESTRING(0 0,3 0)", "LINESTRING(2 0,1 0)", "lsls0009.svg");

    to_svg<ls, ls>("LINESTRING(0 0,1 0)", "LINESTRING(1 0,1 1)", "lsls00010.svg");
    to_svg<ls, ls>("LINESTRING(0 0,1 0)", "LINESTRING(1 1,1 0)", "lsls00011.svg");
    to_svg<ls, ls>("LINESTRING(0 0,1 0)", "LINESTRING(0 0,0 1)", "lsls00012.svg");
    to_svg<ls, ls>("LINESTRING(0 0,1 0)", "LINESTRING(0 1,0 0)", "lsls00013.svg");
    to_svg<ls, ls>("LINESTRING(0 0,2 0)", "LINESTRING(1 0,1 1)", "lsls00014.svg");
    to_svg<ls, ls>("LINESTRING(0 0,2 0)", "LINESTRING(1 1,1 0)", "lsls00015.svg");

    to_svg<ls, ls>("LINESTRING(0 0,2 0)", "LINESTRING(1 0,3 1)", "lsls00016.svg");

    to_svg<ls, ls>("LINESTRING(0 0,2 0)", "LINESTRING(2 0,1 0)", "lsls00017.svg");
    to_svg<ls, ls>("LINESTRING(0 0,2 0)", "LINESTRING(0 0,1 0)", "lsls00018.svg");
    to_svg<ls, ls>("LINESTRING(0 0,2 0)", "LINESTRING(1 0,0 0)", "lsls00019.svg");
    to_svg<ls, ls>("LINESTRING(0 0,2 0)", "LINESTRING(1 0,2 0)", "lsls00020.svg");

    to_svg<ls, ls>("LINESTRING(0 0,2 0)", "LINESTRING(0 0,2 0)", "lsls000.svg");
    to_svg<ls, ls>("LINESTRING(0 0,2 0,2 0,3 0)", "LINESTRING(0 0,2 0)", "lsls001.svg");
    to_svg<ls, ls>("LINESTRING(1 0,1 1)", "LINESTRING(0 0,1 0,2 0)", "lsls002.svg");

    to_svg<ls, ls>("LINESTRING(0 0,2 0,4 0,6 0,8 0)", "LINESTRING(1 0,3 0,5 0,6 0,9 0)", "lsls01.svg");
    to_svg<ls, ls>("LINESTRING(0 0,2 0,4 0,4 4)", "LINESTRING(1 0,3 0,4 0,4 2,4 5)", "lsls02.svg");
    to_svg<ls, ls>("LINESTRING(0 0,2 0,4 0,4 4)", "LINESTRING(1 0,4 0,4 4)", "lsls031.svg");
    to_svg<ls, ls>("LINESTRING(0 0,2 0,4 0,4 4)", "LINESTRING(4 0,2 0,0 0)", "lsls032.svg");
    to_svg<ls, ls>("LINESTRING(0 0,2 0,4 0,4 4)", "LINESTRING(4 0,2 2,0 2)", "lsls0321.svg");
    to_svg<ls, ls>("LINESTRING(0 0,2 0,4 0,4 4)", "LINESTRING(4 0,2 0)", "lsls033.svg");
    to_svg<ls, ls>("LINESTRING(0 0,2 0,4 0,4 4)", "LINESTRING(4 0,4 4)", "lsls034.svg");
    to_svg<ls, ls>("LINESTRING(0 0,2 0,4 0,4 4)", "LINESTRING(4 0,3 1)", "lsls035.svg");
    to_svg<ls, ls>("LINESTRING(0 0,2 0,4 0,4 4)", "LINESTRING(4 0,4 -1)", "lsls036.svg");
    to_svg<ls, ls>("LINESTRING(0 0,2 0,4 0,4 4)", "LINESTRING(1 0,4 0,4 3)", "lsls04.svg");
    to_svg<ls, ls>("LINESTRING(1 0,2 0,4 0,6 0,8 0)", "LINESTRING(0 0,3 0,5 0,6 0,9 0)", "lsls05.svg");
    
    to_svg<ls, ls>("LINESTRING(0 0,10 0,10 10)", "LINESTRING(1 0,10 9)", "lsls061.svg");
    to_svg<ls, ls>("LINESTRING(0 0,10 0,10 10)", "LINESTRING(1 0,10 -9)", "lsls062.svg");
    to_svg<ls, ls>("LINESTRING(0 0,10 0,10 10)", "LINESTRING(1 0,-10 9)", "lsls063.svg");
    to_svg<ls, ls>("LINESTRING(0 0,10 0,10 10)", "LINESTRING(1 0,-10 -9)", "lsls064.svg");
    to_svg<ls, ls>("LINESTRING(0 0,1 0,10 9,10 10)", "LINESTRING(1 0,10 9)", "lsls065.svg");
    to_svg<ls, ls>("LINESTRING(0 0,10 0,10 10)", "LINESTRING(1 0,9 9)", "lsls071.svg");
    to_svg<ls, ls>("LINESTRING(0 0,10 0,10 10)", "LINESTRING(1 0,9 -9)", "lsls072.svg");
    to_svg<ls, ls>("LINESTRING(0 0,10 0,10 10)", "LINESTRING(1 0,-9 9)", "lsls073.svg");
    to_svg<ls, ls>("LINESTRING(0 0,10 0,10 10)", "LINESTRING(1 0,-9 -9)", "lsls074.svg");
    to_svg<ls, ls>("LINESTRING(0 0,1 0,10 0,10 10)", "LINESTRING(1 0,9 9)", "lsls081.svg");
    to_svg<ls, ls>("LINESTRING(0 0,1 0,10 0,10 10)", "LINESTRING(0 0,9 9)", "lsls082.svg");
    to_svg<ls, ls>("LINESTRING(0 0,1 0)", "LINESTRING(1 0,9 9)", "lsls083.svg");
    to_svg<ls, ls>("LINESTRING(0 0,1 0)", "LINESTRING(9 9,1 0)", "lsls084.svg");
    to_svg<ls, ls>("LINESTRING(0 0,1 0)", "LINESTRING(1 0,2 0)", "lsls085.svg");
    to_svg<ls, ls>("LINESTRING(0 0,1 0)", "LINESTRING(2 0,1 0)", "lsls086.svg");
    to_svg<ls, ls>("LINESTRING(0 0,10 0,10 10)", "LINESTRING(1 1,10 5)", "lsls091.svg");
    to_svg<ls, ls>("LINESTRING(0 0,10 0,10 5,10 10)", "LINESTRING(1 1,10 5)", "lsls092.svg");
    to_svg<ls, ls>("LINESTRING(0 0,10 0,10 10)", "LINESTRING(19 1,10 5)", "lsls093.svg");
    to_svg<ls, ls>("LINESTRING(0 0,10 0,10 5,10 10)", "LINESTRING(19 1,10 5)", "lsls094.svg");

    test_geometry<ls, ls>("LINESTRING(0 0, 2 2, 3 2)", "LINESTRING(0 0, 2 2, 3 2)", "1FFF0FFFT");
    test_geometry<ls, ls>("LINESTRING(0 0,3 2)", "LINESTRING(0 0, 2 2, 3 2)", "FF1F0F1FT");

    test_geometry<ls, ls>("LINESTRING(0 0, 1 1, 2 2, 3 2)", "LINESTRING(0 0, 2 2, 3 2)", "1FFF0FFFT");
    test_geometry<ls, ls>("LINESTRING(3 2, 2 2, 1 1, 0 0)", "LINESTRING(0 0, 2 2, 3 2)", "1FFF0FFFT");
    test_geometry<ls, ls>("LINESTRING(0 0, 1 1, 2 2, 3 2)", "LINESTRING(3 2, 2 2, 0 0)", "1FFF0FFFT");
    test_geometry<ls, ls>("LINESTRING(3 2, 2 2, 1 1, 0 0)", "LINESTRING(3 2, 2 2, 0 0)", "1FFF0FFFT");

    test_geometry<ls, ls>("LINESTRING(3 1, 2 2, 1 1, 0 0)", "LINESTRING(0 0, 2 2, 3 2)", "1F1F0010T");
    test_geometry<ls, ls>("LINESTRING(3 3, 2 2, 1 1, 0 0)", "LINESTRING(0 0, 2 2, 3 2)", "1F1F0010T");

    test_geometry<ls, ls>("LINESTRING(0 0, 1 1, 2 2, 2 3)", "LINESTRING(0 0, 2 2, 2 3)", "1FFF0FFFT");
    test_geometry<ls, ls>("LINESTRING(2 3, 2 2, 1 1, 0 0)", "LINESTRING(0 0, 2 2, 2 3)", "1FFF0FFFT");
    test_geometry<ls, ls>("LINESTRING(0 0, 1 1, 2 2, 2 3)", "LINESTRING(2 3, 2 2, 0 0)", "1FFF0FFFT");
    test_geometry<ls, ls>("LINESTRING(2 3, 2 2, 1 1, 0 0)", "LINESTRING(2 3, 2 2, 0 0)", "1FFF0FFFT");

    test_geometry<ls, ls>("LINESTRING(1 1, 2 2, 3 2)", "LINESTRING(0 0, 2 2, 4 2)", "1FF0FF10T");
    test_geometry<ls, ls>("LINESTRING(3 2, 2 2, 1 1)", "LINESTRING(0 0, 2 2, 4 2)", "1FF0FF10T");
    test_geometry<ls, ls>("LINESTRING(1 1, 2 2, 3 2)", "LINESTRING(4 2, 2 2, 0 0)", "1FF0FF10T");
    test_geometry<ls, ls>("LINESTRING(3 2, 2 2, 1 1)", "LINESTRING(4 2, 2 2, 0 0)", "1FF0FF10T");

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

    test_geometry<ls, ls>("LINESTRING(0 0,2 2,3 3,1 1,5 3)", "LINESTRING(0 0,3 3,6 3)", "1F100F10T");
    test_geometry<ls, ls>("LINESTRING(5 3,1 1,3 3,2 2,0 0)", "LINESTRING(0 0,3 3,6 3)", "1F100F10T");
    test_geometry<ls, ls>("LINESTRING(0 0,2 2,3 3,1 1,5 3)", "LINESTRING(6 3,3 3,0 0)", "1F100F10T");
    test_geometry<ls, ls>("LINESTRING(5 3,1 1,3 3,2 2,0 0)", "LINESTRING(6 3,3 3,0 0)", "1F100F10T");

    test_geometry<ls, ls>("LINESTRING(0 5,5 5,10 5,10 10,5 10,5 5,5 0)",
                          "LINESTRING(0 5,5 5,5 10,10 10,10 5,5 5,5 0)", "1FFF0FFFT");
    to_svg<ls, ls>("LINESTRING(0 5,5 5,10 5,10 10,5 10,5 5,5 0)", "LINESTRING(0 5,5 5,5 10,10 10,10 5,5 5,5 0)", "lsls11.svg");
    to_svg<ls, ls>("LINESTRING(0 5,5 5,10 5,10 10,5 10,5 5,5 0)", "LINESTRING(5 0,5 5,10 5,10 10,5 10,5 5,0 5)", "lsls12.svg");
    to_svg<ls, ls>("LINESTRING(5 0,5 5,5 10,10 10,10 5,5 5,0 5)", "LINESTRING(0 5,5 5,5 10,10 10,10 5,5 5,5 0)", "lsls13.svg");
    to_svg<ls, ls>("LINESTRING(5 0,5 5,5 10,10 10,10 5,5 5,0 5)", "LINESTRING(5 0,5 5,10 5,10 10,5 10,5 5,0 5)", "lsls14.svg");

    to_svg<ls, ls>("LINESTRING(0 5,10 5,10 10,5 10,5 0)", "LINESTRING(0 5,5 5,5 10,10 10,10 5,5 5,5 0)", "lsls15.svg");
    to_svg<ls, ls>("LINESTRING(0 5,10 5,10 10,5 10,5 0)", "LINESTRING(5 0,5 10,10 10,10 5,0 5)", "lsls16.svg");
    to_svg<ls, ls>("LINESTRING(0 5,10 5)", "LINESTRING(5 0,5 10,10 10,10 5,0 5)", "lsls161.svg");
    to_svg<ls, ls>("LINESTRING(0 5,8 5,10 5)", "LINESTRING(5 0,5 10,10 10,10 5,0 5)", "lsls162.svg");
    to_svg<ls, ls>("LINESTRING(0 5,8 5)", "LINESTRING(5 0,5 10,10 10,10 5,0 5)", "lsls1631.svg");
    to_svg<ls, ls>("LINESTRING(0 5,1 5,7 5,8 5)", "LINESTRING(5 0,5 10,10 10,10 5,0 5)", "lsls1632.svg");
    to_svg<ls, ls>("LINESTRING(0 5,1 5,7 5,8 5)", "LINESTRING(5 10,10 10,10 5,0 5)", "lsls1633.svg");
    to_svg<ls, ls>("LINESTRING(0 5,8 4)", "LINESTRING(5 0,5 10,10 10,10 5,0 5)", "lsls1641.svg");
    to_svg<ls, ls>("LINESTRING(0 5,8 6)", "LINESTRING(5 0,5 10,10 10,10 5,0 5)", "lsls1642.svg");
    to_svg<ls, ls>("LINESTRING(1 5,8 4)", "LINESTRING(5 0,5 10,10 10,10 5,0 5)", "lsls1643.svg");
    to_svg<ls, ls>("LINESTRING(1 5,8 5)", "LINESTRING(5 0,5 10,10 10,10 5,0 5)", "lsls1644.svg");
    to_svg<ls, ls>("LINESTRING(0 5,5 5,8 4)", "LINESTRING(5 0,5 10,10 10,10 5,0 5)", "lsls165.svg");
    to_svg<ls, ls>("LINESTRING(0 5,5 5,8 5)", "LINESTRING(5 0,5 10,10 10,10 5,0 5)", "lsls166.svg");
    to_svg<ls, ls>("LINESTRING(0 5,5 5,8 5)", "LINESTRING(0 10,10 0,5 0,5 10,10 10,10 5,0 5)", "lsls167.svg");
    to_svg<ls, ls>("LINESTRING(0 5,5 5,8 5)", "LINESTRING(0 10,5 5,10 0,5 0,5 5,5 10,10 10,10 5,0 5)", "lsls168.svg");

    to_svg<ls, ls>("LINESTRING(0 0,0 10,10 10,10 0,0 0)", "LINESTRING(0 2,0 0,10 0,10 10,0 10,0 8,0 2)", "lsls1690.svg");
    to_svg<ls, ls>("LINESTRING(0 0,10 0,10 10,0 10,0 0)", "LINESTRING(0 8,0 0,10 0,10 10,0 10,0 8)", "lsls1691.svg");
    to_svg<ls, ls>("LINESTRING(0 0,10 0,10 10,0 10,0 0)", "LINESTRING(0 2,0 0,10 0,10 10,0 10,0 8)", "lsls1692.svg");
    to_svg<ls, ls>("LINESTRING(0 0,0 10,10 10,10 0,0 0)", "LINESTRING(0 2,0 0,10 0,10 10,0 10,0 8)", "lsls1693.svg");
    to_svg<ls, ls>("LINESTRING(0 2,0 0,10 0,10 10,0 10,0 8)", "LINESTRING(0 0,10 0,10 10,0 10,0 0)", "lsls1694.svg");
    to_svg<ls, ls>("LINESTRING(0 2,0 0,10 0,10 10,0 10,0 8)", "LINESTRING(0 0,0 10,10 10,10 0,0 0)", "lsls1695.svg");

    to_svg<ls>("LINESTRING(0 8,0 0,10 0,10 10,0 10,0 2)", "ls1.svg");
    to_svg<ls>("LINESTRING(8 8,0 0,10 0,10 10,0 10,8 2)", "ls2.svg");

    typedef bg::model::multi_linestring<ls> mls;
    to_svg<ls, mls>("LINESTRING(0 5,10 5,10 10,5 10,5 0)", "MULTILINESTRING((5 0,5 7),(5 8,5 10,10 10,10 5,0 5))", "lsls17.svg");
    to_svg<ls, mls>("LINESTRING(0 5,10 5,10 10,5 10,5 0)", "MULTILINESTRING((5 0,5 4,5 6,5 7),(5 8,5 10,10 10,10 5,0 5))", "lsls18.svg");
    to_svg<ls, mls>("LINESTRING(0 5,10 5,10 10,5 10,5 0)", "MULTILINESTRING((5 0,5 8),(5 7,5 10,10 10,10 5,0 5))", "lsls19.svg");
    to_svg<mls, ls>("MULTILINESTRING((5 0,5 7),(5 8,5 10,10 10,10 5,0 5))", "LINESTRING(0 5,10 5,10 10,5 10,5 0)", "lsls20.svg");
    to_svg<mls, ls>("MULTILINESTRING((5 0,5 8),(5 7,5 10,10 10,10 5,0 5))", "LINESTRING(0 5,10 5,10 10,5 10,5 0)", "lsls21.svg");

    to_svg<ls, ls>("LINESTRING(0 5,5 5,10 5,10 10,5 10,5 5,5 0)", "LINESTRING(0 5,5 5,0 10,10 10,10 5,5 5,5 0)", "lsls100.svg");

    to_svg<ls, ls>("LINESTRING(5 0,5 5,5 0)", "LINESTRING(0 5,5 5,0 10,2 10,5 5,5 10,10 10,10 5,5 5,10 2,10 0,8 0,5 5,5 0)", "lsls101.svg");

    // TEST ERROR - wrong result
//    test_geometry<ls, ls>("LINESTRING(0 5,5 5,10 5,10 10,5 10,5 5,5 0)",
//                          "LINESTRING(0 5,5 5,0 10,10 10,10 5,5 5,5 0)", "1FFF0FFFT");

    test_geometry<ls, ls>("LINESTRING(0 0,1 1)", "LINESTRING(0 1,1 0)", "0F1FF010T");
    test_geometry<ls, ls>("LINESTRING(0 0,1 1)", "LINESTRING(1 1,2 0)", "FF1F0010T");
    test_geometry<ls, ls>("LINESTRING(0 0,1 1)", "LINESTRING(2 0,1 1)", "FF1F0010T");
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
