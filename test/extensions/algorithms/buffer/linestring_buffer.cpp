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
static std::string const straight = "LINESTRING(0 0,4 5,8 10)";
static std::string const one_bend = "LINESTRING(0 0,4 5,7 4)";
static std::string const two_bends = "LINESTRING(0 0,4 5,7 4,10 6)";
static std::string const overlapping = "LINESTRING(0 0,4 5,7 4,10 6, 10 2,2 2)";
static std::string const curve = "LINESTRING(2 7,3 5,5 4,7 5,8 7)";

static std::string const chained2 = "LINESTRING(0 0,1 1,2 2)";
static std::string const chained3 = "LINESTRING(0 0,1 1,2 2,3 3)";
static std::string const chained4 = "LINESTRING(0 0,1 1,2 2,3 3,4 4)";

static std::string const reallife1 = "LINESTRING(76396.40464822574 410095.6795147947,76397.85016212701 410095.211865792,76401.30666443033 410095.0466387949,76405.05892643372 410096.1007777959,76409.45103273794 410098.257640797,76412.96309264141 410101.6522238015)";



template <typename P>
void test_all()
{
    namespace buf = bg::strategy::buffer;
    typedef bg::model::linestring<P> linestring;
    typedef bg::model::polygon<P> polygon;

    double factor = +1.0; // does NOT yet work for negative buffer
    double right = factor * 1.0;

    test_one<linestring, buf::join_round, polygon>("simplex", simplex, 'r', 19.2093727122985, 1.5, right);
    test_one<linestring, buf::join_miter, polygon>("simplex", simplex, 'm', 19.2093727122985, 1.5, right);

    test_one<linestring, buf::join_round, polygon>("straight", straight, 'r', 19.2093727122985, 1.5, right);
    test_one<linestring, buf::join_miter, polygon>("straight", straight, 'm', 19.2093727122985, 1.5, right);

    test_one<linestring, buf::join_round, polygon>("one_bend", one_bend, 'r', 28.4879539312069, 1.5, right);
    test_one<linestring, buf::join_miter, polygon>("one_bend", one_bend, 'm', 28.6962056928037, 1.5, right);

    test_one<linestring, buf::join_round, polygon>("two_bends", two_bends, 'r', 39.2220036534424, 1.5, right);
    test_one<linestring, buf::join_miter, polygon>("two_bends", two_bends, 'm', 39.5128595191957, 1.5, right);

    test_one<linestring, buf::join_round, polygon>("overlapping", overlapping, 'r', 65.646005724872, 1.5, right);
    test_one<linestring, buf::join_miter, polygon>("overlapping", overlapping, 'm', 68.1395194809293, 1.5, right);

    test_one<linestring, buf::join_round, polygon>("curve", curve, 'r', 65.646005724872, 5.0, factor * 3.0);
    test_one<linestring, buf::join_miter, polygon>("curve", curve, 'm', 68.1395194809293, 5.0, factor * 3.0);

    test_one<linestring, buf::join_round, polygon>("chained2", chained2, 'r', 65.646005724872, 2.5, factor * 1.5);
    test_one<linestring, buf::join_round, polygon>("chained3", chained3, 'r', 65.646005724872, 2.5, factor * 1.5);
    test_one<linestring, buf::join_round, polygon>("chained4", chained4, 'r', 65.646005724872, 2.5, factor * 1.5);

    test_one<linestring, buf::join_round, polygon>("reallife1", reallife1, 'r', 65.646005724872, 16.5, factor * 6.5);
}


//#define HAVE_TTMATH
#ifdef HAVE_TTMATH
#include <ttmath_stub.hpp>
#endif


int test_main(int, char* [])
{
    test_all<bg::model::point<double, 2, bg::cs::cartesian> >();
    //test_all<bg::model::point<tt, 2, bg::cs::cartesian> >();


    return 0;
}
