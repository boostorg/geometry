// Boost.Geometry
// Unit Test

// Copyright (c) 2014 Adam Wulkiewicz, Lodz, Poland.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <geometry_test_common.hpp>

#include <boost/range.hpp>

#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/geometries/point_xy.hpp>

#include <boost/geometry/algorithms/num_points.hpp>
#include <boost/geometry/multi/algorithms/num_points.hpp>
#include <boost/geometry/algorithms/num_geometries.hpp>

template <typename P>
void test_all_2d()
{
#if !defined(BOOST_NO_CXX11_HDR_INITIALIZER_LIST) && !defined(BOOST_NO_CXX11_UNIFIED_INITIALIZATION_SYNTAX)

    typedef bg::model::multi_point<P> mpt;
    typedef bg::model::linestring<P> ls;
    typedef bg::model::multi_linestring<ls> mls;
    typedef bg::model::ring<P> ring;
    typedef bg::model::polygon<P> poly;
    typedef bg::model::multi_polygon<poly> mpoly;

    mpt mpt1 = {{0, 0}, {1, 0}, {2, 0}};
    BOOST_CHECK(bg::num_geometries(mpt1) == 3);
    BOOST_CHECK(bg::num_points(mpt1) == 3);
    mpt1 = {{0, 0}, {1, 0}, {2, 0}, {3, 0}};
    BOOST_CHECK(bg::num_points(mpt1) == 4);

    ls ls1 = {{0, 0}, {1, 0}, {2, 0}};
    BOOST_CHECK(bg::num_geometries(ls1) == 1);
    BOOST_CHECK(bg::num_points(ls1) == 3);
    ls1 = {{0, 0}, {1, 0}, {2, 0}, {3, 0}};
    BOOST_CHECK(bg::num_points(ls1) == 4);

    mls mls1 = {{{0, 0}, {1, 0}, {2, 0}}, {{3, 0}, {4, 0}}};
    BOOST_CHECK(bg::num_geometries(mls1) == 2);
    BOOST_CHECK(bg::num_points(mls1) == 5);
    mls1 = {{{0, 0}, {1, 0}, {2, 0}}, {{3, 0}, {4, 0}, {5, 0}}};
    BOOST_CHECK(bg::num_points(mls1) == 6);

    ring r1 = {{0, 0}, {0, 1}, {1, 1}, {1, 0}, {0, 0}};
    BOOST_CHECK(bg::num_geometries(r1) == 1);
    BOOST_CHECK(bg::num_points(r1) == 5);
    r1 = {{0, 0}, {0, 1}, {1, 2}, {2, 1}, {1, 0}, {0, 0}};
    BOOST_CHECK(bg::num_points(r1) == 6);

    poly p1 = {{{0, 0}, {0, 9}, {9, 9}, {9, 0}, {0, 0}}, {{1, 1}, {2, 1}, {2, 2}, {1, 2}, {1, 1}}};
    BOOST_CHECK(bg::num_geometries(p1) == 1);
    BOOST_CHECK(bg::num_points(p1) == 10);
    poly p2 = {{{0, 0}, {0, 9}, {9, 9}, {9, 0}, {0, 0}}};
    BOOST_CHECK(bg::num_geometries(p2) == 1);
    BOOST_CHECK(bg::num_points(p2) == 5);
    p2 = {{{0, 0}, {0, 8}, {8, 9}, {9, 8}, {8, 0}, {0, 0}}};
    BOOST_CHECK(bg::num_points(p2) == 6);

    mpoly mp1 = {{{{0, 0}, {0, 1}, {1, 1}, {1, 0}, {0, 0}}}, {{{2, 2}, {2, 3}, {3, 3}, {3, 2}, {2, 2}}}};
    BOOST_CHECK(bg::num_geometries(mp1) == 2);
    BOOST_CHECK(bg::num_points(mp1) == 10);
    mp1 = {{{{0, 0}, {0, 1}, {1, 2}, {2, 1}, {1, 0}, {0, 0}}}, {{{2, 2}, {2, 3}, {3, 3}, {3, 2}, {2, 2}}}};
    BOOST_CHECK(bg::num_points(mp1) == 11);

#endif
}

int test_main(int, char* [])
{
    test_all_2d< bg::model::point<float, 2, bg::cs::cartesian> >();
    test_all_2d< bg::model::d2::point_xy<float> >();

    return 0;
}

