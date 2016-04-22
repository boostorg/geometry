// Boost.Geometry
// Unit Test

// Copyright (c) 2016, Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include "test_get_turns.hpp"
#include <boost/geometry/geometries/geometries.hpp>

#include <algorithms/overlay/overlay_cases.hpp>


template <typename T>
void test_all()
{
    typedef bg::model::point<T, 2, bg::cs::spherical_equatorial<bg::degree> > pt;
    //typedef bg::model::ring<pt> ring;
    typedef bg::model::polygon<pt> poly;
    //typedef bg::model::multi_polygon<poly> mpoly;

    test_geometry<poly, poly>(case_1[0], case_1[1],
                              expected("iiu")("iui")("iiu")("iui")("iiu")("iui"));

    test_geometry<poly, poly>(case_2[0], case_2[1],
                              expected("iui")("iiu")("iui")("iiu")("iui")("iiu")("iui")("iiu"));

#ifdef BOOST_GEOMETRY_ENABLE_FAILING
    test_geometry<poly, poly>(case_3_sph[0], case_3_sph[1],
                              expected("miu")("miu")("miu")("miu"));
#endif

}

int test_main(int, char* [])
{
    test_all<float>();
    test_all<double>();

//#if ! defined(_MSC_VER)
//    test_all<long double>();
//#endif

//#if defined(HAVE_TTMATH)
//    test_all<ttmath_big>();
//#endif
    return 0;
}
