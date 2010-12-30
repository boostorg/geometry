// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>

#include <geometry_test_common.hpp>

#include <boost/geometry/geometries/concepts/segment_concept.hpp>

#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/segment.hpp>


#include <boost/geometry/geometries/adapted/c_array_cartesian.hpp>
#include <boost/geometry/geometries/adapted/tuple_cartesian.hpp>

#include <boost/geometry/util/write_dsv.hpp>


#include <test_common/test_point.hpp>
#include <test_geometries/custom_segment.hpp>


template <typename P>
void test_all()
{
    typedef bg::model::referring_segment<P> S;

    P p1;
    P p2;
    S s(p1, p2);
    BOOST_CHECK_EQUAL(&s.first, &p1);
    BOOST_CHECK_EQUAL(&s.second, &p2);

    // Compilation tests, all things should compile.
    BOOST_CONCEPT_ASSERT( (bg::concept::ConstSegment<S>) );
    BOOST_CONCEPT_ASSERT( (bg::concept::Segment<S>) );

    typedef typename bg::coordinate_type<S>::type T;
    typedef typename bg::point_type<S>::type SP;


    //std::cout << sizeof(typename coordinate_type<S>::type) << std::endl;

    typedef bg::model::referring_segment<P const> CS;
    //BOOST_CONCEPT_ASSERT( (concept::ConstSegment<CS>) );

    CS cs(p1, p2);

    typedef typename bg::coordinate_type<CS>::type CT;
    typedef typename bg::point_type<CS>::type CSP;
}



template <typename S>
void test_custom()
{
    S seg;
    bg::set<0,0>(seg, 1);
    bg::set<0,1>(seg, 2);
    bg::set<1,0>(seg, 3);
    bg::set<1,1>(seg, 4);
    std::ostringstream out;
    out << bg::dsv(seg);
    BOOST_CHECK_EQUAL(out.str(), "((1, 2), (3, 4))");
}


int test_main(int, char* [])
{
    test_all<int[3]>();
    test_all<float[3]>();
    test_all<double[3]>();
    //test_all<test_point>();
    test_all<bg::model::point<int, 3, bg::cs::cartesian> >();
    test_all<bg::model::point<float, 3, bg::cs::cartesian> >();
    test_all<bg::model::point<double, 3, bg::cs::cartesian> >();

    test_custom<test::custom_segment>();
    test_custom<test::custom_segment_of<bg::model::point<double, 2, bg::cs::cartesian> > >();
    test_custom<test::custom_segment_of<test::custom_point_for_segment> >();
    test_custom<test::custom_segment_4>();

    return 0;
}

