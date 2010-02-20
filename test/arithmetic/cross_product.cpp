// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright (c) 2009 Mateusz Loskot, (mateusz@loskot.net)
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <ggl_test_common.hpp>

#include <boost/geometry/arithmetic/cross_product.hpp>

#include <boost/geometry/algorithms/assign.hpp>

#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/adapted/c_array_cartesian.hpp>
#include <boost/geometry/geometries/adapted/tuple_cartesian.hpp>
#include <test_common/test_point.hpp>

using namespace boost::geometry;

template <typename P>
void test_2d()
{
    P p1;
    boost::geometry::assign(p1, 20, 30);
    P p2;
    boost::geometry::assign(p2, 45, 70);
    P c = boost::geometry::cross_product(p1, p2);

    typedef typename boost::geometry::coordinate_type<P>::type scalar_type;
    BOOST_CHECK_EQUAL(boost::geometry::get<0>(c), scalar_type(50));
}

template <typename P>
void test_3d()
{
    P p1;
    boost::geometry::assign(p1, 20, 30, 10);
    P p2;
    boost::geometry::assign(p2, 45, 70, 20);
    P c = boost::geometry::cross_product(p1, p2);
    
    typedef typename boost::geometry::coordinate_type<P>::type scalar_type;
    BOOST_CHECK_EQUAL(boost::geometry::get<0>(c), scalar_type(-100));
    BOOST_CHECK_EQUAL(boost::geometry::get<1>(c), scalar_type(50));
    BOOST_CHECK_EQUAL(boost::geometry::get<2>(c), scalar_type(50));
}

#ifdef TEST_FAIL_CROSS_PRODUCT
template <typename P>
void test_4d()
{
    P p1;
    boost::geometry::assign(p1, 20, 30, 10, 15);
    P p2;
    boost::geometry::assign(p2, 45, 70, 20, 35);
    P c = boost::geometry::cross_product(p1, p2);
}
#endif

int test_main(int, char* [])
{
    test_2d<point<int, 2, boost::geometry::cs::cartesian> >();
    test_2d<point<float, 2, boost::geometry::cs::cartesian> >();
    test_2d<point<double, 2, boost::geometry::cs::cartesian> >();

    test_3d<point<int, 3, boost::geometry::cs::cartesian> >();
    test_3d<point<float, 3, boost::geometry::cs::cartesian> >();
    test_3d<point<double, 3, boost::geometry::cs::cartesian> >();

#ifdef TEST_FAIL_CROSS_PRODUCT
    test_4d<point<int, 3, boost::geometry::cs::cartesian> >();
    test_4d<point<float, 3, boost::geometry::cs::cartesian> >();
    test_4d<point<double, 3, boost::geometry::cs::cartesian> >();
#endif

    return 0;
}

