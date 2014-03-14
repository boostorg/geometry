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

//#include "to_svg.hpp"

template <typename T>
void test_all()
{
    typedef bg::model::point<T, 2, bg::cs::cartesian> pt;
    typedef bg::model::linestring<pt> ls;
    typedef bg::model::polygon<pt> poly;

    test_geometry<ls, poly>("LINESTRING(15 5,24 5,20 2,19 0,13 -4,1 0,10 0,13 3,15 7,16 10,10 10,8 10,4 6,2 8,1 10)",
                            "POLYGON((0 0,5 5,0 10,20 10,20 2,19 0,0 0)(10 3,15 3,15 7,10 7,10 3))",
                            expected("miu")("iuu")("tcc")("tuu")("mcu")("mic")("muu")("tiu")("mcu")("mic")("mcc")("miu")("mxu").vec);

    test_geometry<ls, poly>("LINESTRING(5 0,5 5,10 5)", "POLYGON((0 0,0 10,10 10,10 0,0 0))",
                            "miu", "mxu");
    test_geometry<ls, poly>("LINESTRING(0 0,5 5,10 0)", "POLYGON((0 0,0 10,10 10,10 0,0 0))",
                            "tiu", "txu");
    test_geometry<ls, poly>("LINESTRING(0 0,5 0,5 5,10 5,10 0)", "POLYGON((0 0,0 10,10 10,10 0,0 0))",
                            expected("tcu")("mic")("mcc")("txu").vec);
    test_geometry<ls, poly>("LINESTRING(10 0,5 0,5 5,10 5,10 10)", "POLYGON((0 0,0 10,10 10,10 0,0 0))",
                            expected("tcc")("miu")("mcu")("txc").vec);

    test_geometry<ls, poly>("LINESTRING(0 0,10 0,10 10)",
                            "POLYGON((0 0,5 5,0 10,20 10,20 2,19 0,0 0)(10 3,15 3,15 7,10 7,10 3))",
                            expected("tcu")("mic")("mcu")("mic")("mxu").vec);

    test_geometry<ls, poly>("LINESTRING(11 1,10 0,0 0)", "POLYGON((0 0,0 10,10 10,10 0,0 0))",
                            "tcc", "txu");
    test_geometry<ls, poly>("LINESTRING(0 0,10 0,11 1)", "POLYGON((0 0,0 10,10 10,10 0,0 0))",
                            "tcu", "tuc");
    test_geometry<ls, poly>("LINESTRING(10 0,0 0,-1 1)", "POLYGON((0 0,0 10,10 10,10 0,0 0))",
                            "tcc", "tuu");

    // true hole
    test_geometry<ls, poly>("LINESTRING(9 1,10 5,9 9)",
                            "POLYGON((0 0,0 10,10 10,10 5,10 0,0 0)(2 2,10 5,2 8,2 2))",
                            expected("tiu")("tiu").vec);
    test_geometry<ls, poly>("LINESTRING(10 1,10 5,10 9)",
                            "POLYGON((0 0,0 10,10 10,10 5,10 0,0 0)(2 2,10 5,2 8,2 2))",
                            expected("mcu")("ecc")("tiu")("mxc").vec);

    // fake hole
    test_geometry<ls, poly>("LINESTRING(9 1,10 5,9 9)",
                            "POLYGON((0 0,0 10,10 10,10 5,2 8,2 2,10 5,10 0,0 0))",
                            expected("tuu")("tiu").vec);
    test_geometry<ls, poly>("LINESTRING(10 1,10 5,10 9)",
                            "POLYGON((0 0,0 10,10 10,10 5,2 8,2 2,10 5,10 0,0 0))",
                            expected("mcu")("tuc")("tcu")("mxc").vec);
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
