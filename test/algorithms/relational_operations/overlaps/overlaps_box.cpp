// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2015 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2014, 2015.
// Modifications copyright (c) 2014-2015 Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "test_overlaps.hpp"

template <typename P>
void test_box_box_2d()
{
#if defined(BOOST_GEOMETRY_COMPILE_FAIL)
    test_geometry<P, P>("POINT(1 1)", "POINT(1 1)", true);
#endif

    test_geometry<bg::model::box<P>, bg::model::box<P> >("BOX(1 1, 3 3)", "BOX(0 0,2 2)", true);

    // touch -> false
    test_geometry<bg::model::box<P>, bg::model::box<P> >("BOX(1 1, 3 3)", "BOX(3 3,5 5)", false);

    // disjoint -> false
    test_geometry<bg::model::box<P>, bg::model::box<P> >("BOX(1 1, 3 3)", "BOX(4 4,6 6)", false);

    // within -> false
    test_geometry<bg::model::box<P>, bg::model::box<P> >("BOX(1 1, 5 5)", "BOX(2 2,3 3)", false);

    // within+touch -> false
    test_geometry<bg::model::box<P>, bg::model::box<P> >("BOX(1 1, 5 5)", "BOX(2 2,5 5)", false);
}

template <typename P>
void test_3d()
{
    test_geometry<bg::model::box<P>, bg::model::box<P> >("BOX(1 1 1, 3 3 3)", "BOX(0 0 0,2 2 2)", true);
    test_geometry<bg::model::box<P>, bg::model::box<P> >("BOX(1 1 1, 3 3 3)", "BOX(3 3 3,5 5 5)", false);
    test_geometry<bg::model::box<P>, bg::model::box<P> >("BOX(1 1 1, 3 3 3)", "BOX(4 4 4,6 6 6)", false);
}

template <typename P>
void test_eps()
{
    typedef typename bg::coordinate_type<P>::type coord_type;
    coord_type const eps = std::numeric_limits<coord_type>::epsilon();
    
    bg::model::box<P> b1(P(0, 0), P(1, 1));
    bg::model::box<P> b2(P(-1, -1), P(eps/2, eps/2));

    test_geometry(b1, b2,
                  "box(P(0, 0), P(1, 1))", "box(P(-1, -1), P(eps/2, eps/2))",
                  false);
}

template <typename P>
void test_2d()
{
    test_box_box_2d<P>();
}

int test_main( int , char* [] )
{
    test_2d<bg::model::d2::point_xy<int> >();
    test_2d<bg::model::d2::point_xy<double> >();
    test_eps<bg::model::d2::point_xy<double> >();

#if defined(HAVE_TTMATH)
    test_2d<bg::model::d2::point_xy<ttmath_big> >();
    test_eps<bg::model::d2::point_xy<ttmath_big> >();
#endif

   //test_3d<bg::model::point<double, 3, bg::cs::cartesian> >();

    return 0;
}
