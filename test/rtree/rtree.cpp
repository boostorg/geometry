// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2012 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2012 Mateusz Loskot, London, UK.

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <rtree/test_rtree.hpp>

#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>

template<typename P, typename Algo>
void test_rtree_by_point2()
{
    typedef bg::model::box<P> B;

    std::vector<B> input;

    for ( size_t i = 0 ; i < 20 ; i += 3 )
    {
        for ( size_t j = 1 ; j < 31 ; j += 4 )
        {
            input.push_back(B(P(i, j), P(i + 2, j + 3)));
        }
    }

    bgi::rtree<B, Algo> tree;
    tree.insert(input.begin(), input.end());

    std::vector<B> expected_output;
    expected_output.push_back(B(P(3, 1), P(5, 4)));
    expected_output.push_back(B(P(6, 1), P(8, 4)));

    B qbox(P(4, 2), P(7, 3));

    test(tree, qbox, expected_output);
    test(tree, bgi::intersects(qbox), expected_output);
    test(tree, !bgi::not_intersects(qbox), expected_output);
    test(tree, !bgi::disjoint(qbox), expected_output);
    test(tree, bgi::not_disjoint(qbox), expected_output);
}

int test_main(int, char* [])
{
    typedef bg::model::point<int, 2, bg::cs::cartesian> P2ic;
    typedef bg::model::point<float, 2, bg::cs::cartesian> P2fc;
    typedef bg::model::point<double, 2, bg::cs::cartesian> P2dc;

    test_rtree_by_point2<P2ic, bgi::linear<4, 2> >();
    test_rtree_by_point2<P2fc, bgi::linear<4, 2> >();
    test_rtree_by_point2<P2dc, bgi::linear<4, 2> >();
    test_rtree_by_point2<P2ic, bgi::quadratic<4, 2> >();
    test_rtree_by_point2<P2fc, bgi::quadratic<4, 2> >();
    test_rtree_by_point2<P2dc, bgi::quadratic<4, 2> >();
    test_rtree_by_point2<P2ic, bgi::rstar<4, 2> >();
    test_rtree_by_point2<P2fc, bgi::rstar<4, 2> >();
    test_rtree_by_point2<P2dc, bgi::rstar<4, 2> >();
    
#ifdef HAVE_TTMATH
    typedef bg::model::point<ttmath_big, 2, bg::cs::cartesian> P2ttmc;

    test_rtree_by_point2<P2ttmc, bgi::linear<4, 2> >();
    test_rtree_by_point2<P2ttmc, bgi::quadratic<4, 2> >();
    test_rtree_by_point2<P2ttmc, bgi::rstar<4, 2> >();
#endif

    return 0;
}
