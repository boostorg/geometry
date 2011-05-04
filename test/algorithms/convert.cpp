// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2007-2011 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2011 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2011 Mateusz Loskot, London, UK.

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <geometry_test_common.hpp>


#include <boost/geometry/algorithms/assign.hpp>
#include <boost/geometry/algorithms/convert.hpp>
#include <boost/geometry/algorithms/make.hpp>
#include <boost/geometry/algorithms/num_points.hpp>

#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/geometries/adapted/c_array.hpp>
#include <boost/geometry/geometries/adapted/boost_tuple.hpp>
#include <test_common/test_point.hpp>

BOOST_GEOMETRY_REGISTER_C_ARRAY_CS(cs::cartesian)
BOOST_GEOMETRY_REGISTER_BOOST_TUPLE_CS(cs::cartesian)


template <typename P>
void test_all()
{
    typedef bg::model::box<P> box_type;

    P p;
    bg::assign_values(p, 1, 2);

    box_type b;
    bg::convert(p, b);

    BOOST_CHECK_CLOSE((bg::get<0, 0>(b)), 1.0, 0.001);
    BOOST_CHECK_CLOSE((bg::get<0, 1>(b)), 2.0, 0.001);
    BOOST_CHECK_CLOSE((bg::get<1, 0>(b)), 1.0, 0.001);
    BOOST_CHECK_CLOSE((bg::get<1, 1>(b)), 2.0, 0.001);
}

template <typename P>
void test_std()
{
    test_all<P>();

    typedef bg::model::box<P> box_type;
    typedef bg::model::ring<P> ring_type;
    typedef bg::model::polygon<P> polygon_type;

    box_type b;
    bg::set<bg::min_corner, 0>(b, 1);
    bg::set<bg::min_corner, 1>(b, 2);
    bg::set<bg::max_corner, 0>(b, 3);
    bg::set<bg::max_corner, 1>(b, 4);

    ring_type ring;
    bg::convert(b, ring);

    //std::cout << bg::wkt(b) << std::endl;
    //std::cout << bg::wkt(ring) << std::endl;

    typename boost::range_const_iterator<ring_type>::type it = ring.begin();
    BOOST_CHECK_CLOSE(bg::get<0>(*it), 1.0, 0.001);
    BOOST_CHECK_CLOSE(bg::get<1>(*it), 2.0, 0.001);
    it++;
    BOOST_CHECK_CLOSE(bg::get<0>(*it), 1.0, 0.001);
    BOOST_CHECK_CLOSE(bg::get<1>(*it), 4.0, 0.001);
    it++;
    BOOST_CHECK_CLOSE(bg::get<0>(*it), 3.0, 0.001);
    BOOST_CHECK_CLOSE(bg::get<1>(*it), 4.0, 0.001);
    it++;
    BOOST_CHECK_CLOSE(bg::get<0>(*it), 3.0, 0.001);
    BOOST_CHECK_CLOSE(bg::get<1>(*it), 2.0, 0.001);
    it++;
    BOOST_CHECK_CLOSE(bg::get<0>(*it), 1.0, 0.001);
    BOOST_CHECK_CLOSE(bg::get<1>(*it), 2.0, 0.001);

    BOOST_CHECK_EQUAL(ring.size(), 5u);


    polygon_type polygon;

    bg::convert(ring, polygon);
    BOOST_CHECK_EQUAL(bg::num_points(polygon), 5u);

    bg::convert(polygon, ring);
    BOOST_CHECK_EQUAL(bg::num_points(ring), 5u);
}

int test_main(int, char* [])
{
    test_std<bg::model::point<int, 2, bg::cs::cartesian> >();
    test_std<bg::model::point<float, 2, bg::cs::cartesian> >();
    test_std<bg::model::point<double, 2, bg::cs::cartesian> >();

#ifdef HAVE_TTMATH
    test_std<bg::model::point<ttmath_big, 2, bg::cs::cartesian> >();
#endif
    return 0;
}
