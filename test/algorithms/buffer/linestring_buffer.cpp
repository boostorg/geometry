// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//#define BOOST_GEOMETRY_DEBUG_WITH_MAPPER
//#define BOOST_GEOMETRY_DEBUG_ASSEMBLE
//#define BOOST_GEOMETRY_DEBUG_IDENTIFIER

#include <geometry_test_common.hpp>

#include <boost/geometry/algorithms/buffer.hpp>
#include <boost/geometry/core/coordinate_type.hpp>

#include <boost/geometry/strategies/strategies.hpp>

#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <test_common/test_point.hpp>

#include <test_buffer.hpp>

static std::string const simplex = "LINESTRING(0 0,4 5)";
static std::string const one_bend = "LINESTRING(0 0,4 5,7 4)";
static std::string const two_bends = "LINESTRING(0 0,4 5,7 4,10 6)";
static std::string const overlapping = "LINESTRING(0 0,4 5,7 4,10 6, 10 2,2 2)";

template <typename P>
void test_all()
{
    namespace bg = boost::geometry;
    namespace buf = boost::geometry::strategy::buffer;
    typedef bg::linestring<P> linestring;
    typedef bg::polygon<P> polygon;

    double right = 1.5;

    test_one<linestring, buf::join_round, polygon>("simplex", simplex, 'r', 19.2093727122985, 1.5, right);
    test_one<linestring, buf::join_miter, polygon>("simplex", simplex, 'm', 19.2093727122985, 1.5, right);

    test_one<linestring, buf::join_round, polygon>("one_bend", one_bend, 'r', 28.4879539312069, 1.5, right);
    test_one<linestring, buf::join_miter, polygon>("one_bend", one_bend, 'm', 28.6962056928037, 1.5, right);

    test_one<linestring, buf::join_round, polygon>("two_bends", two_bends, 'r', 39.2220036534424, 1.5, right);
    test_one<linestring, buf::join_miter, polygon>("two_bends", two_bends, 'm', 39.5128595191957, 1.5, right);

    test_one<linestring, buf::join_round, polygon>("overlapping", overlapping, 'r', 65.646005724872, 1.5, right);
    test_one<linestring, buf::join_miter, polygon>("overlapping", overlapping, 'm', 68.1395194809293, 1.5, right);
}

int test_main(int, char* [])
{
    namespace bg = boost::geometry;
    test_all<bg::point<double, 2, bg::cs::cartesian> >();


    return 0;
}
