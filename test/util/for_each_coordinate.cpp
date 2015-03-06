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

#if !defined(BOOST_TEST_MODULE)
# define BOOST_TEST_MODULE Boost.Geometry Util for_each_coordinate
#endif

#include <sstream>

#include <geometry_test_common.hpp>

#include <boost/mpl/list.hpp>

#include <boost/geometry/util/for_each_coordinate.hpp>

#include <boost/geometry/algorithms/assign.hpp>

#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/adapted/c_array.hpp>
#include <boost/geometry/geometries/adapted/boost_tuple.hpp>
#include <test_common/test_point.hpp>

BOOST_GEOMETRY_REGISTER_C_ARRAY_CS(cs::cartesian)
BOOST_GEOMETRY_REGISTER_BOOST_TUPLE_CS(cs::cartesian)

using namespace boost::unit_test;

BOOST_AUTO_TEST_SUITE(for_each_coordinate)

struct test_operation
{
    template <typename P, int I>
    static void apply(P& p)
    {
        bg::set<I>(p, bg::get<I>(p) * 10);
    }
};

struct get_operation
{
    std::string s;

    template <typename P, int I>
    inline void apply(P const& p)
    {
        std::ostringstream out;
        out << bg::get<I>(p);
        s += out.str();
    }
};

typedef boost::mpl::list
<
    int[3],
    float[3],
    double[3],
    test::test_point,
    bg::model::point<int, 3, bg::cs::cartesian>,
    bg::model::point<float, 3, bg::cs::cartesian>,
    bg::model::point < double, 3, bg::cs::cartesian >
> test_types;
BOOST_AUTO_TEST_CASE_TEMPLATE(all, P, test_types)
{
    P p;
    bg::assign_values(p, 1, 2, 3);
    bg::for_each_coordinate(p, test_operation());
    BOOST_CHECK(bg::get<0>(p) == 10);
    BOOST_CHECK(bg::get<1>(p) == 20);
    BOOST_CHECK(bg::get<2>(p) == 30);

    P const& cp = p;
    get_operation op;
    op = bg::for_each_coordinate(cp, op);
    BOOST_CHECK(op.s == std::string("102030"));
}

BOOST_AUTO_TEST_SUITE_END()
