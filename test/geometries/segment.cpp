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

#include <boost/geometry/geometries/register/point.hpp>
#include <boost/geometry/geometries/register/segment.hpp>

#include <boost/geometry/geometries/adapted/c_array_cartesian.hpp>
#include <boost/geometry/geometries/adapted/tuple_cartesian.hpp>

#include <boost/geometry/util/write_dsv.hpp>


#include <test_common/test_point.hpp>


template <typename P>
void test_all()
{
    typedef boost::geometry::segment<P> S;

    P p1;
    P p2;
    S s(p1, p2);
    BOOST_CHECK_EQUAL(&s.first, &p1);
    BOOST_CHECK_EQUAL(&s.second, &p2);

    // Compilation tests, all things should compile.
    BOOST_CONCEPT_ASSERT( (boost::geometry::concept::ConstSegment<S>) );
    BOOST_CONCEPT_ASSERT( (boost::geometry::concept::Segment<S>) );

    typedef typename boost::geometry::coordinate_type<S>::type T;
    typedef typename boost::geometry::point_type<S>::type SP;


    //std::cout << sizeof(typename coordinate_type<S>::type) << std::endl;

    typedef boost::geometry::segment<const P> CS;
    //BOOST_CONCEPT_ASSERT( (concept::ConstSegment<CS>) );

    CS cs(p1, p2);

    typedef typename boost::geometry::coordinate_type<CS>::type CT;
    typedef typename boost::geometry::point_type<CS>::type CSP;
}

struct custom_point
{
    double x, y;
};
struct custom_segment
{
    custom_point one, two;
};
template <typename P>
struct custom_segment_of
{
    P p1, p2;
};
struct custom_segment_4
{
    double a, b, c, d;
};

BOOST_GEOMETRY_REGISTER_POINT_2D(custom_point, double, boost::geometry::cs::cartesian, x, y)
BOOST_GEOMETRY_REGISTER_SEGMENT(custom_segment, custom_point, one, two)
BOOST_GEOMETRY_REGISTER_SEGMENT_TEMPLATIZED(custom_segment_of, p1, p2)
BOOST_GEOMETRY_REGISTER_SEGMENT_2D_4VALUES(custom_segment_4, custom_point, a, b, c, d)


template <typename S>
void test_custom()
{
    S seg;
    boost::geometry::set<0,0>(seg, 1);
    boost::geometry::set<0,1>(seg, 2);
    boost::geometry::set<1,0>(seg, 3);
    boost::geometry::set<1,1>(seg, 4);
    std::ostringstream out;
    out << boost::geometry::dsv(seg);
    BOOST_CHECK_EQUAL(out.str(), "((1, 2), (3, 4))");
}


int test_main(int, char* [])
{
    test_all<int[3]>();
    test_all<float[3]>();
    test_all<double[3]>();
    //test_all<test_point>();
    test_all<boost::geometry::point<int, 3, boost::geometry::cs::cartesian> >();
    test_all<boost::geometry::point<float, 3, boost::geometry::cs::cartesian> >();
    test_all<boost::geometry::point<double, 3, boost::geometry::cs::cartesian> >();

    test_custom<custom_segment>();
    test_custom<custom_segment_of<boost::geometry::point<double, 2, boost::geometry::cs::cartesian> > >();
    test_custom<custom_segment_of<custom_point> >();
    test_custom<custom_segment_4>();

    return 0;
}
