// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2012 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2012 Mateusz Loskot, London, UK.

// This file was modified by Oracle on 2014.
// Modifications copyright (c) 2014 Oracle and/or its affiliates.

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

#include "test_get_turns.hpp"
#include <boost/geometry/geometries/geometries.hpp>

//TEST
#include <to_svg.hpp>
//#include <boost/geometry.hpp>
//#include <boost/geometry/multi/geometries/multi_linestring.hpp>

template <typename T>
void test_all()
{
    typedef bg::model::point<T, 2, bg::cs::cartesian> pt;
    typedef bg::model::linestring<pt> ls;
    typedef bg::model::polygon<pt> poly;

    to_svg<ls, poly>("LINESTRING(15 5,24 5,20 2,19 0,13 -4,1 0,10 0,13 3,15 7,16 10,10 10,8 10,4 6,2 8,1 10)",
                     "POLYGON((0 0,5 5,0 10,20 10,20 2,19 0,0 0)(10 3,15 3,15 7,10 7,10 3))",
                     "ls_poly1.svg");
    to_svg<ls, poly>("LINESTRING(15 3,15 5,24 5,20 2,19 0,13 -4,1 0,10 0,13 3,15 7,16 10,10 10,8 10,4 6,2 8,1 10)",
                     "POLYGON((0 0,5 5,0 10,20 10,20 2,19 0,0 0)(10 3,15 3,15 7,10 7,10 3))",
                     "ls_poly2.svg");
    to_svg<ls, poly>("LINESTRING(15 7,15 5,24 5,20 2,19 0,13 -4,1 0,10 0,13 3,15 7,16 10,10 10,8 10,4 6,2 8,1 10)",
                     "POLYGON((0 0,5 5,0 10,20 10,20 2,19 0,0 0)(10 3,15 3,15 7,10 7,10 3))",
                     "ls_poly3.svg");
    to_svg<ls, poly>("LINESTRING(15 5,15 7,24 5,20 2,19 0,13 -4,1 0,10 0,13 3,15 7,16 10,10 10,8 10,4 6,2 8,1 10)",
                     "POLYGON((0 0,5 5,0 10,20 10,20 2,19 0,0 0)(10 3,15 3,15 7,10 7,10 3))",
                     "ls_poly4.svg");
    to_svg<ls, poly>("LINESTRING(15 5,15 3,24 5,20 2,19 0,13 -4,1 0,10 0,13 3,15 7,16 10,10 10,8 10,4 6,2 8,1 10)",
                     "POLYGON((0 0,5 5,0 10,20 10,20 2,19 0,0 0)(10 3,15 3,15 7,10 7,10 3))",
                     "ls_poly5.svg");

    to_svg<poly, ls>("POLYGON((0 0,5 5,0 10,20 10,20 2,19 0,0 0)(10 3,15 3,15 7,10 7,10 3))",
                     "LINESTRING(15 5,24 5,20 2,19 0,13 -4,1 0,10 0,13 3,15 7,16 10,10 10,8 10,4 6,2 8,1 10)",
                     "poly_ls.svg");
}

int test_main(int, char* [])
{
    test_all<float>();
    test_all<double>();

#if ! defined(_MSC_VER)
    test_all<long double>();
#endif

#if defined(HAVE_TTMATH)
    test_all<ttmath_big>();
#endif
    return 0;
}

/*
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
to_svg<ls, ls>("LINESTRING(1 0,1 1)", "LINESTRING(0 0,1 0,2 0)", "lsls0020.svg");
to_svg<ls, ls>("LINESTRING(1 0,0 0)", "LINESTRING(0 0,1 0,2 0)", "lsls0021.svg");
to_svg<ls, ls>("LINESTRING(1 0,2 0)", "LINESTRING(0 0,1 0,2 0)", "lsls0022.svg");
to_svg<ls, ls>("LINESTRING(1 1,1 0)", "LINESTRING(0 0,1 0,2 0)", "lsls0023.svg");
to_svg<ls, ls>("LINESTRING(0 0,1 0)", "LINESTRING(0 0,1 0,2 0)", "lsls0024.svg");
to_svg<ls, ls>("LINESTRING(2 0,1 0)", "LINESTRING(0 0,1 0,2 0)", "lsls0025.svg");
to_svg<ls, ls>("LINESTRING(0 0,1 0,2 0)", "LINESTRING(1 0,1 1)", "lsls00200.svg");
to_svg<ls, ls>("LINESTRING(0 0,1 0,2 0)", "LINESTRING(1 0,0 0)", "lsls00211.svg");
to_svg<ls, ls>("LINESTRING(0 0,1 0,2 0)", "LINESTRING(1 0,2 0)", "lsls00222.svg");
to_svg<ls, ls>("LINESTRING(0 0,1 0,2 0)", "LINESTRING(1 1,1 0)", "lsls00233.svg");
to_svg<ls, ls>("LINESTRING(0 0,1 0,2 0)", "LINESTRING(0 0,1 0)", "lsls00244.svg");
to_svg<ls, ls>("LINESTRING(0 0,1 0,2 0)", "LINESTRING(2 0,1 0)", "lsls00255.svg");

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

to_svg<ls, ls>("LINESTRING(5 3,1 1,3 3,2 2,0 0)", "LINESTRING(0 0,3 3,6 3)", "1F100F10T.svg");

to_svg<ls, ls>("LINESTRING(0 0,2 0,4 0)", "LINESTRING(1 1,1 0,3 0,3 1)", "lsls_01.svg");
to_svg<ls, ls>("LINESTRING(0 0,2 0,4 0)", "LINESTRING(1 -1,1 0,3 0,3 -1)", "lsls_02.svg");
to_svg<ls, ls>("LINESTRING(0 0,2 0,4 0)", "LINESTRING(3 1,3 0,1 0,1 1)", "lsls_03.svg");
to_svg<ls, ls>("LINESTRING(0 0,2 0,4 0)", "LINESTRING(3 -1,3 0,1 0,1 -1)", "lsls_04.svg");
to_svg<ls, ls>("LINESTRING(0 0,2 0,3 0,4 0,6 0)", "LINESTRING(2 1,2 0,4 0,4 1)", "lsls_05.svg");
to_svg<ls, ls>("LINESTRING(0 0,2 0,3 0,4 0,6 0)", "LINESTRING(2 -1,2 0,4 0,4 -1)", "lsls_06.svg");
to_svg<ls, ls>("LINESTRING(0 0,2 0,3 0,4 0,6 0)", "LINESTRING(4 1,4 0,2 0,2 1)", "lsls_07.svg");
to_svg<ls, ls>("LINESTRING(0 0,2 0,3 0,4 0,6 0)", "LINESTRING(4 -1,4 0,2 0,2 -1)", "lsls_08.svg");

to_svg<ls, ls>("LINESTRING(0 0,2 0,4 0)", "LINESTRING(1 1,1 0,2 0,3 0,3 1)", "lsls_11.svg");
to_svg<ls, ls>("LINESTRING(0 0,2 0,4 0)", "LINESTRING(1 -1,1 0,2 0,3 0,3 -1)", "lsls_12.svg");
to_svg<ls, ls>("LINESTRING(0 0,2 0,4 0)", "LINESTRING(3 1,3 0,2 0,1 0,1 1)", "lsls_13.svg");
to_svg<ls, ls>("LINESTRING(0 0,2 0,4 0)", "LINESTRING(3 -1,3 0,2 0,1 0,1 -1)", "lsls_14.svg");
to_svg<ls, ls>("LINESTRING(0 0,2 0,3 0,4 0,6 0)", "LINESTRING(2 1,2 0,3 0,4 0,4 1)", "lsls_15.svg");
to_svg<ls, ls>("LINESTRING(0 0,2 0,3 0,4 0,6 0)", "LINESTRING(2 -1,2 0,3 0,4 0,4 -1)", "lsls_16.svg");
to_svg<ls, ls>("LINESTRING(0 0,2 0,3 0,4 0,6 0)", "LINESTRING(4 1,4 0,3 0,2 0,2 1)", "lsls_17.svg");
to_svg<ls, ls>("LINESTRING(0 0,2 0,3 0,4 0,6 0)", "LINESTRING(4 -1,4 0,3 0,2 0,2 -1)", "lsls_18.svg");

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
*/
