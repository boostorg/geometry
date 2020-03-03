// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2020 Digvijay Janartha, Hamirpur, India.

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>

#include <geometry_test_common.hpp>

#include <boost/core/ignore_unused.hpp>
#include <boost/geometry/algorithms/make.hpp>
#include <boost/geometry/algorithms/append.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/concepts/polygon_concept.hpp>
#include <boost/geometry/geometries/adapted/c_array.hpp>
#include <boost/geometry/geometries/adapted/boost_tuple.hpp>
#include <boost/geometry/io/dsv/write.hpp>

#include <test_common/test_point.hpp>

BOOST_GEOMETRY_REGISTER_C_ARRAY_CS(cs::cartesian)
BOOST_GEOMETRY_REGISTER_BOOST_TUPLE_CS(cs::cartesian)


template <typename P>
bg::model::polygon<P> create_polygon()
{   
    bg::model::polygon<P> pl1;
    P p1;
    P p2;
    P p3;
    bg::assign_values(p1, 1, 2);
    bg::assign_values(p2, 2, 0);
    bg::assign_values(p3, 0, 0);
    
    bg::append(pl1, p1);
    bg::append(pl1, p2);
    bg::append(pl1, p3);
    bg::append(pl1, p1);
    return pl1;
}

template <typename PL, typename P>
void check_polygon(PL& to_check, P p1, P p2, P p3)
{   
    std::ostringstream out;
    out << bg::dsv(to_check);
    BOOST_CHECK_EQUAL(out.str(), "(((1, 2), (2, 0), (0, 0), (1, 2)))");
}

template <typename P>
void test_construction()
{
    bg::model::polygon<P> pl1(create_polygon<P>());
    check_polygon(pl1, P(1, 2), P(2, 0), P(0, 0));
}

template <typename P>
void test_compilation()
{   
    typedef bg::model::polygon<P> PL;

    BOOST_CONCEPT_ASSERT( (bg::concepts::ConstPolygon<PL>) );
    BOOST_CONCEPT_ASSERT( (bg::concepts::Polygon<PL>) );

    typedef typename bg::coordinate_type<PL>::type T;
    typedef typename bg::point_type<PL>::type PPL;
    boost::ignore_unused<T, PPL>();
}

template <typename P>
void test_all()
{   
    test_construction<P>();
    test_compilation<P>();
}

template <typename P>
void test_custom_polygon(bg::model::ring<P> IL)
{   
    std::initializer_list<bg::model::ring<P> > RIL = {IL};
    bg::model::polygon<P> pl1(RIL);
    std::ostringstream out;
    out << bg::dsv(pl1);
    BOOST_CHECK_EQUAL(out.str(), "(((2, 2), (2, 0), (0, 0), (0, 2), (2, 2)))");
}

template <typename P>
void test_custom()
{   
    std::initializer_list<P> IL = {P(2, 2), P(2, 0), P(0, 0), P(0, 2), P(2, 2)};
    bg::model::ring<P> r1(IL);
    test_custom_polygon<P>(r1);
}


int test_main(int, char* [])
{   
    test_all<bg::model::point<int, 2, bg::cs::cartesian> >();
    test_all<bg::model::point<float, 2, bg::cs::cartesian> >();
    test_all<bg::model::point<double, 2, bg::cs::cartesian> >();

    test_custom<bg::model::point<double, 2, bg::cs::cartesian> >();
    test_custom<bg::model::d2::point_xy<double> >();

    return 0;
}
