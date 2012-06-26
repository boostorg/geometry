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
void test_rtree_by_point()
{
    typedef bg::model::box<P> B;
    typedef std::pair<B, int> V;
    B qbox;
    {
        bgi::rtree<P, Algo> tree;
        std::vector<P> input;        
        test_generate_rtree(tree, input, qbox);
        test_intersects(tree, input, qbox);
    }
    {
        bgi::rtree<B, Algo> tree;
        std::vector<B> input;
        test_generate_rtree(tree, input, qbox);
        test_intersects(tree, input, qbox);
    }
    {
        bgi::rtree<V, Algo> tree;
        std::vector<V> input;
        test_generate_rtree(tree, input, qbox);
        test_intersects(tree, input, qbox);
    }
}

int test_main(int, char* [])
{
    typedef bg::model::point<int, 2, bg::cs::cartesian> P2ic;
    typedef bg::model::point<float, 2, bg::cs::cartesian> P2fc;
    typedef bg::model::point<double, 2, bg::cs::cartesian> P2dc;

    test_rtree_by_point<P2ic, bgi::linear<4, 2> >();
    test_rtree_by_point<P2fc, bgi::linear<4, 2> >();
    test_rtree_by_point<P2dc, bgi::linear<4, 2> >();
    test_rtree_by_point<P2ic, bgi::quadratic<4, 2> >();
    test_rtree_by_point<P2fc, bgi::quadratic<4, 2> >();
    test_rtree_by_point<P2dc, bgi::quadratic<4, 2> >();
    test_rtree_by_point<P2ic, bgi::rstar<4, 2> >();
    test_rtree_by_point<P2fc, bgi::rstar<4, 2> >();
    test_rtree_by_point<P2dc, bgi::rstar<4, 2> >();
    
#ifdef HAVE_TTMATH
    typedef bg::model::point<ttmath_big, 2, bg::cs::cartesian> P2ttmc;

    test_rtree_by_point<P2ttmc, bgi::linear<4, 2> >();
    test_rtree_by_point<P2ttmc, bgi::quadratic<4, 2> >();
    test_rtree_by_point<P2ttmc, bgi::rstar<4, 2> >();
#endif

    return 0;
}
