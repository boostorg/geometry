// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <geometry_test_common.hpp>

#include <boost/geometry/algorithms/assign.hpp>
#include <boost/geometry/algorithms/num_points.hpp>

#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/geometries/adapted/c_array_cartesian.hpp>
#include <boost/geometry/geometries/adapted/tuple_cartesian.hpp>
#include <test_common/test_point.hpp>


template <typename L>
void check_linestring_2d(const L& line)
{
    BOOST_CHECK((boost::size(line) == 3));
    BOOST_CHECK((bg::num_points(line) == 3));

    typedef typename bg::point_type<L>::type point_type;
    point_type const& p0 = line[0];
    BOOST_CHECK(bg::get<0>(p0) == 1);
    BOOST_CHECK(bg::get<1>(p0) == 2);

    point_type const& p1 = line[1];
    BOOST_CHECK(bg::get<0>(p1) == 3);
    BOOST_CHECK(bg::get<1>(p1) == 4);

    point_type const& p2 = line[2];
    BOOST_CHECK(bg::get<0>(p2) == 5);
    BOOST_CHECK(bg::get<1>(p2) == 6);
}

template <typename P>
void test_assign_linestring_2d()
{
    bg::model::linestring<P> line;

    // Test assignment of plain array (note that this is only possible if adapted c-array is included!
    const double coors[3][2] = { {1, 2}, {3, 4}, {5, 6} };
    bg::assign(line, coors);
    check_linestring_2d(line);

    // Test assignment of point array
    P points[3];
    bg::assign(points[0], 1, 2);
    bg::assign(points[1], 3, 4);
    bg::assign(points[2], 5, 6);
    bg::assign(line, points);
    check_linestring_2d(line);

    // Test assignment of array with different point-type
    boost::tuple<float, float> tuples[3];
    tuples[0] = boost::make_tuple(1, 2);
    tuples[1] = boost::make_tuple(3, 4);
    tuples[2] = boost::make_tuple(5, 6);
    bg::assign(line, tuples);
    check_linestring_2d(line);
}

template <typename P>
void test_assign_box_2d()
{

    typedef bg::model::box<P> B;
    B b;
    bg::assign(b, 1, 2, 3, 4);
    BOOST_CHECK((bg::get<bg::min_corner, 0>(b) == 1));
    BOOST_CHECK((bg::get<bg::min_corner, 1>(b) == 2));
    BOOST_CHECK((bg::get<bg::max_corner, 0>(b) == 3));
    BOOST_CHECK((bg::get<bg::max_corner, 1>(b) == 4));

    bg::assign_zero(b);
    BOOST_CHECK((bg::get<bg::min_corner, 0>(b) == 0));
    BOOST_CHECK((bg::get<bg::min_corner, 1>(b) == 0));
    BOOST_CHECK((bg::get<bg::max_corner, 0>(b) == 0));
    BOOST_CHECK((bg::get<bg::max_corner, 1>(b) == 0));

    bg::assign_inverse(b);
    BOOST_CHECK((bg::get<bg::min_corner, 0>(b) > 9999));
    BOOST_CHECK((bg::get<bg::min_corner, 1>(b) > 9999));
    BOOST_CHECK((bg::get<bg::max_corner, 0>(b) < 9999));
    BOOST_CHECK((bg::get<bg::max_corner, 1>(b) < 9999));

}

template <typename P>
void test_assign_point_3d()
{
    P p;
    bg::assign(p, 1, 2, 3);
    BOOST_CHECK(bg::get<0>(p) == 1);
    BOOST_CHECK(bg::get<1>(p) == 2);
    BOOST_CHECK(bg::get<2>(p) == 3);

    bg::detail::assign::assign_value(p, 123);
    BOOST_CHECK(bg::get<0>(p) == 123);
    BOOST_CHECK(bg::get<1>(p) == 123);
    BOOST_CHECK(bg::get<2>(p) == 123);

    bg::assign_zero(p);
    BOOST_CHECK(bg::get<0>(p) == 0);
    BOOST_CHECK(bg::get<1>(p) == 0);
    BOOST_CHECK(bg::get<2>(p) == 0);

}

template <typename P>
void test_assign_point_2d()
{
    P p;
    bg::assign(p, 1, 2);
    BOOST_CHECK(bg::get<0>(p) == 1);
    BOOST_CHECK(bg::get<1>(p) == 2);

    bg::detail::assign::assign_value(p, 123);
    BOOST_CHECK(bg::get<0>(p) == 123);
    BOOST_CHECK(bg::get<1>(p) == 123);

    bg::assign_zero(p);
    BOOST_CHECK(bg::get<0>(p) == 0);
    BOOST_CHECK(bg::get<1>(p) == 0);
}

int test_main(int, char* [])
{
    test_assign_point_3d<int[3]>();
    test_assign_point_3d<float[3]>();
    test_assign_point_3d<double[3]>();
    test_assign_point_3d<test::test_point>();
    test_assign_point_3d<bg::model::point<int, 3, bg::cs::cartesian> >();
    test_assign_point_3d<bg::model::point<float, 3, bg::cs::cartesian> >();
    test_assign_point_3d<bg::model::point<double, 3, bg::cs::cartesian> >();

    test_assign_point_2d<int[2]>();
    test_assign_point_2d<float[2]>();
    test_assign_point_2d<double[2]>();
    test_assign_point_2d<bg::model::point<int, 2, bg::cs::cartesian> >();
    test_assign_point_2d<bg::model::point<float, 2, bg::cs::cartesian> >();
    test_assign_point_2d<bg::model::point<double, 2, bg::cs::cartesian> >();

    test_assign_box_2d<int[2]>();
    test_assign_box_2d<float[2]>();
    test_assign_box_2d<double[2]>();
    test_assign_box_2d<bg::model::point<int, 2, bg::cs::cartesian> >();
    test_assign_box_2d<bg::model::point<float, 2, bg::cs::cartesian> >();
    test_assign_box_2d<bg::model::point<double, 2, bg::cs::cartesian> >();

    test_assign_linestring_2d<bg::model::point<int, 2, bg::cs::cartesian> >();
    test_assign_linestring_2d<bg::model::point<float, 2, bg::cs::cartesian> >();
    test_assign_linestring_2d<bg::model::point<double, 2, bg::cs::cartesian> >();


    return 0;
}
