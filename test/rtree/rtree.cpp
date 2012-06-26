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

template <typename Value, typename Algo, typename Box>
void test_intersects(bgi::rtree<Value, Algo> const& tree, std::vector<Value> const& input, Box const& qbox)
{
    std::vector<Value> expected_output;

    BOOST_FOREACH(Value const& v, input)
        if ( bg::intersects(tree.translator()(v), qbox) )
            expected_output.push_back(v);

    test(tree, qbox, expected_output);
    test(tree, bgi::intersects(qbox), expected_output);
    test(tree, !bgi::not_intersects(qbox), expected_output);
    test(tree, !bgi::disjoint(qbox), expected_output);
    test(tree, bgi::not_disjoint(qbox), expected_output);
}

template<typename P, typename Algo>
void test_rtree_point_2d()
{
    typedef bg::model::box<P> B;

    std::vector<P> input;
    B qbox;
    test_generate<2>::apply(std::back_inserter(input), qbox, test_gen_point<P>);

    bgi::rtree<P, Algo> tree;
    tree.insert(input.begin(), input.end());

    test_intersects(tree, input, qbox);
}

template<typename P, typename Algo>
void test_rtree_box_2d()
{
    typedef bg::model::box<P> B;
    
    std::vector<B> input;
    B qbox;
    test_generate<2>::apply(std::back_inserter(input), qbox, test_gen_box<P>);

    bgi::rtree<B, Algo> tree;
    tree.insert(input.begin(), input.end());

    test_intersects(tree, input, qbox);
}

template<typename P, typename Algo>
void test_rtree_box_pair_2d()
{
    typedef bg::model::box<P> B;
    typedef std::pair<B, int> V;

    std::vector<V> input;
    B qbox;
    test_generate<2>::apply(std::back_inserter(input), qbox, test_gen_box_pair<P>);

    bgi::rtree<V, Algo> tree;
    tree.insert(input.begin(), input.end());

    test_intersects(tree, input, qbox);
}

template<typename P, typename Algo>
void test_rtree_by_point2()
{
    test_rtree_point_2d<P, Algo>();
    test_rtree_box_2d<P, Algo>();
    test_rtree_box_pair_2d<P, Algo>();
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
