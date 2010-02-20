// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <geometry_test_common.hpp>


#include <boost/geometry/algorithms/assign.hpp>
#include <boost/geometry/algorithms/convert.hpp>
#include <boost/geometry/algorithms/make.hpp>
#include <boost/geometry/algorithms/num_points.hpp>

#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/geometries/adapted/c_array_cartesian.hpp>
#include <boost/geometry/geometries/adapted/tuple_cartesian.hpp>
#include <test_common/test_point.hpp>

template <typename P>
void test_all()
{
    typedef boost::geometry::box<P> box_type;

    P p;
    boost::geometry::assign(p, 1, 2);

    box_type b;
    boost::geometry::convert(p, b);

    BOOST_CHECK_CLOSE(double(boost::geometry::get<0, 0>(b)), 1.0, 0.001);
    BOOST_CHECK_CLOSE(double(boost::geometry::get<0, 1>(b)), 2.0, 0.001);
    BOOST_CHECK_CLOSE(double(boost::geometry::get<1, 0>(b)), 1.0, 0.001);
    BOOST_CHECK_CLOSE(double(boost::geometry::get<1, 1>(b)), 2.0, 0.001);
}

template <typename P>
void test_std()
{
    test_all<P>();

    typedef boost::geometry::box<P> box_type;
    typedef boost::geometry::linear_ring<P> ring_type;
    typedef boost::geometry::polygon<P> polygon_type;

    box_type b;
    boost::geometry::set<boost::geometry::min_corner, 0>(b, 1);
    boost::geometry::set<boost::geometry::min_corner, 1>(b, 2);
    boost::geometry::set<boost::geometry::max_corner, 0>(b, 3);
    boost::geometry::set<boost::geometry::max_corner, 1>(b, 4);

    ring_type ring;
    boost::geometry::convert(b, ring);

    //std::cout << boost::geometry::wkt(b) << std::endl;
    //std::cout << boost::geometry::wkt(ring) << std::endl;

    typename boost::range_const_iterator<ring_type>::type it = ring.begin();
    BOOST_CHECK_CLOSE(double(boost::geometry::get<0>(*it)), 1.0, 0.001);
    BOOST_CHECK_CLOSE(double(boost::geometry::get<1>(*it)), 2.0, 0.001);
    it++;
    BOOST_CHECK_CLOSE(double(boost::geometry::get<0>(*it)), 1.0, 0.001);
    BOOST_CHECK_CLOSE(double(boost::geometry::get<1>(*it)), 4.0, 0.001);
    it++;
    BOOST_CHECK_CLOSE(double(boost::geometry::get<0>(*it)), 3.0, 0.001);
    BOOST_CHECK_CLOSE(double(boost::geometry::get<1>(*it)), 4.0, 0.001);
    it++;
    BOOST_CHECK_CLOSE(double(boost::geometry::get<0>(*it)), 3.0, 0.001);
    BOOST_CHECK_CLOSE(double(boost::geometry::get<1>(*it)), 2.0, 0.001);
    it++;
    BOOST_CHECK_CLOSE(double(boost::geometry::get<0>(*it)), 1.0, 0.001);
    BOOST_CHECK_CLOSE(double(boost::geometry::get<1>(*it)), 2.0, 0.001);

    BOOST_CHECK_EQUAL(ring.size(), 5);


    polygon_type polygon;

    boost::geometry::convert(ring, polygon);
    BOOST_CHECK_EQUAL(boost::geometry::num_points(polygon), 5);

    boost::geometry::convert(polygon, ring);
    BOOST_CHECK_EQUAL(boost::geometry::num_points(ring), 5);
}

int test_main(int, char* [])
{
    test_std<boost::geometry::point<int, 2, boost::geometry::cs::cartesian> >();
    test_std<boost::geometry::point<float, 2, boost::geometry::cs::cartesian> >();
    test_std<boost::geometry::point<double, 2, boost::geometry::cs::cartesian> >();

    return 0;
}
