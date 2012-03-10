// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2010-2012 Barend Gehrels, Amsterdam, the Netherlands.

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

static std::string const for_collinear = "LINESTRING(2 0,0 0,0 4,6 4,6 0,4 0)";
static std::string const for_collinear2 = "LINESTRING(2 1,2 0,0 0,0 4,6 4,6 0,4 0,4 1)";

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

    test_one<linestring, buf::join_round, polygon>("simplex", simplex, 'r', 19.209, 1.5, 1.5);
    test_one<linestring, buf::join_miter, polygon>("simplex", simplex, 'm', 19.209, 1.5, 1.5);

    test_one<linestring, buf::join_miter, polygon>("simplex_asym_neg", simplex, 'm', 3.202, +1.5, -1.0);
    test_one<linestring, buf::join_miter, polygon>("simplex_asym_pos", simplex, 'm', 3.202, -1.5, +1.0);

    //test_one<linestring, buf::join_round, polygon>("straight", straight, 'r', 19.2093727122985, 1.5, 1.5);
    //test_one<linestring, buf::join_miter, polygon>("straight", straight, 'm', 19.2093727122985, 1.5, 1.5);

    test_one<linestring, buf::join_round, polygon>("one_bend", one_bend, 'r', 28.488, 1.5, 1.5);
    test_one<linestring, buf::join_miter, polygon>("one_bend", one_bend, 'm', 28.696, 1.5, 1.5);

    test_one<linestring, buf::join_round, polygon>("two_bends", two_bends, 'r', 39.222, 1.5, 1.5);
    test_one<linestring, buf::join_miter, polygon>("two_bends", two_bends, 'm', 39.513, 1.5, 1.5);
    test_one<linestring, buf::join_round, polygon>("two_bends_left", two_bends, 'r', 20.028, 1.5, 0.0);
    test_one<linestring, buf::join_miter, polygon>("two_bends_left", two_bends, 'm', 20.225, 1.5, 0.0);
    test_one<linestring, buf::join_round, polygon>("two_bends_right", two_bends, 'r', 19.211, 0.0, 1.5);
    test_one<linestring, buf::join_miter, polygon>("two_bends_right", two_bends, 'm', 19.288, 0.0, 1.5);


	// Next (and all similar cases) which a offsetted-one-sided buffer has to be fixed.
    //test_one<linestring, buf::join_miter, polygon>("two_bends_neg", two_bends, 'm', 99, +1.5, -1.0);
    //test_one<linestring, buf::join_miter, polygon>("two_bends_pos", two_bends, 'm', 99, -1.5, +1.0);
    //test_one<linestring, buf::join_round, polygon>("two_bends_neg", two_bends, 'r', 99, +1.5, -1.0);
    //test_one<linestring, buf::join_round, polygon>("two_bends_pos", two_bends, 'r', 99, -1.5, +1.0);

    test_one<linestring, buf::join_round, polygon>("overlapping150", overlapping, 'r', 65.646, 1.5, 1.5);
    test_one<linestring, buf::join_miter, polygon>("overlapping150", overlapping, 'm', 68.140, 1.5, 1.5);
	// Different cases with intersection points on flat and (left/right from line itself)
    test_one<linestring, buf::join_round, polygon>("overlapping_asym_150_010", overlapping, 'r', 48.308, 1.5, 0.25);
    test_one<linestring, buf::join_miter, polygon>("overlapping_asym_150_010", overlapping, 'm', 50.770, 1.5, 0.25);
    test_one<linestring, buf::join_round, polygon>("overlapping_asym_150_075", overlapping, 'r', 58.506, 1.5, 0.75);
    test_one<linestring, buf::join_miter, polygon>("overlapping_asym_150_075", overlapping, 'm', 60.985, 1.5, 0.75);
    test_one<linestring, buf::join_round, polygon>("overlapping_asym_150_100", overlapping, 'r', 62.514, 1.5, 1.0);
    test_one<linestring, buf::join_miter, polygon>("overlapping_asym_150_100", overlapping, 'm', 64.984, 1.5, 1.0);

    test_one<linestring, buf::join_round, polygon>("for_collinear", for_collinear, 'r', 68.561, 2.0, 2.0);
    test_one<linestring, buf::join_miter, polygon>("for_collinear", for_collinear, 'm', 72, 2.0, 2.0);
    test_one<linestring, buf::join_round, polygon>("for_collinear2", for_collinear2, 'r', 74.387, 2.0, 2.0);
    test_one<linestring, buf::join_miter, polygon>("for_collinear2", for_collinear2, 'm', 78.0, 2.0, 2.0);

    //test_one<linestring, buf::join_round, polygon>("curve", curve, 'r', 99, 5.0, 3.0);
    //test_one<linestring, buf::join_miter, polygon>("curve", curve, 'm', 99, 5.0, 3.0);

    test_one<linestring, buf::join_round, polygon>("chained2", chained2, 'r', 11.3137, 2.5, 1.5);
    test_one<linestring, buf::join_round, polygon>("chained3", chained3, 'r', 16.9706, 2.5, 1.5);
    test_one<linestring, buf::join_round, polygon>("chained4", chained4, 'r', 22.6274, 2.5, 1.5);

    //test_one<linestring, buf::join_round, polygon>("reallife1", reallife1, 'r', 99, 16.5, 6.5);
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
