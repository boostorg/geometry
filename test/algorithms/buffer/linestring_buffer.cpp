// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2012-2014 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <test_buffer.hpp>

#include <boost/geometry/algorithms/buffer.hpp>
#include <boost/geometry/core/coordinate_type.hpp>

#include <boost/geometry/strategies/strategies.hpp>

#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <test_common/test_point.hpp>


static std::string const simplex = "LINESTRING(0 0,4 5)";
static std::string const straight = "LINESTRING(0 0,4 5,8 10)";
static std::string const one_bend = "LINESTRING(0 0,4 5,7 4)";
static std::string const two_bends = "LINESTRING(0 0,4 5,7 4,10 6)";
static std::string const overlapping = "LINESTRING(0 0,4 5,7 4,10 6, 10 2,2 2)";
static std::string const curve = "LINESTRING(2 7,3 5,5 4,7 5,8 7)";
static std::string const tripod = "LINESTRING(5 0,5 5,1 8,5 5,9 8)"; // with spike

static std::string const for_collinear = "LINESTRING(2 0,0 0,0 4,6 4,6 0,4 0)";
static std::string const for_collinear2 = "LINESTRING(2 1,2 0,0 0,0 4,6 4,6 0,4 0,4 1)";

static std::string const chained2 = "LINESTRING(0 0,1 1,2 2)";
static std::string const chained3 = "LINESTRING(0 0,1 1,2 2,3 3)";
static std::string const chained4 = "LINESTRING(0 0,1 1,2 2,3 3,4 4)";

static std::string const reallife1 = "LINESTRING(76396.40464822574 410095.6795147947,76397.85016212701 410095.211865792,76401.30666443033 410095.0466387949,76405.05892643372 410096.1007777959,76409.45103273794 410098.257640797,76412.96309264141 410101.6522238015)";
static std::string const aimes175 = "LINESTRING(-2.3116 52.354326,-2.311555 52.35417,-2.311489 52.354145,-2.311335 52.354178)";


template <typename P>
void test_all()
{
    namespace buf = bg::strategy::buffer;
    typedef bg::model::linestring<P> linestring;
    typedef bg::model::polygon<P> polygon;

    // Simplex (join-type is not relevant)
    test_one<linestring, buf::join_miter, buf::end_flat, polygon>("simplex", simplex, 19.209, 1.5, 1.5);
    test_one<linestring, buf::join_miter, buf::end_round, polygon>("simplex", simplex, 26.2733, 1.5, 1.5);

    test_one<linestring, buf::join_miter, buf::end_flat, polygon>("simplex_asym_neg", simplex, 3.202, +1.5, -1.0);
    test_one<linestring, buf::join_miter, buf::end_flat, polygon>("simplex_asym_pos", simplex, 3.202, -1.0, +1.5);
    // Do not work yet:
    //    test_one<linestring, buf::join_miter, buf::end_round, polygon>("simplex_asym_neg", simplex, 3.202, +1.5, -1.0);
    //    test_one<linestring, buf::join_miter, buf::end_round, polygon>("simplex_asym_pos", simplex, 3.202, -1.0, +1.5);

    // Generates a reverse polygon, with a negative area, which will be made empty TODO decide about this
    test_one<linestring, buf::join_miter, buf::end_flat, polygon>("simplex_asym_neg_rev", simplex, 0, +1.0, -1.5);
    test_one<linestring, buf::join_miter, buf::end_flat, polygon>("simplex_asym_pos_rev", simplex, 0, -1.5, +1.0);

    test_one<linestring, buf::join_round, buf::end_flat, polygon>("straight", straight, 38.4187, 1.5, 1.5);
    test_one<linestring, buf::join_miter, buf::end_flat, polygon>("straight", straight, 38.4187, 1.5, 1.5);

    // One bend/two bends (tests join-type)
    test_one<linestring, buf::join_round, buf::end_flat, polygon>("one_bend", one_bend, 28.488, 1.5, 1.5);
    test_one<linestring, buf::join_miter, buf::end_flat, polygon>("one_bend", one_bend, 28.696, 1.5, 1.5);
    test_one<linestring, buf::join_round_by_divide, buf::end_flat, polygon>("one_bend", one_bend, 28.488, 1.5, 1.5);

    test_one<linestring, buf::join_round, buf::end_round, polygon>("one_bend", one_bend, 35.5603, 1.5, 1.5);
    test_one<linestring, buf::join_miter, buf::end_round, polygon>("one_bend", one_bend, 35.7601, 1.5, 1.5);

    test_one<linestring, buf::join_round, buf::end_flat, polygon>("two_bends", two_bends, 39.235, 1.5, 1.5);
    test_one<linestring, buf::join_round_by_divide, buf::end_flat, polygon>("two_bends", two_bends, 39.235, 1.5, 1.5);
    test_one<linestring, buf::join_miter, buf::end_flat, polygon>("two_bends", two_bends, 39.513, 1.5, 1.5);
    test_one<linestring, buf::join_round, buf::end_flat, polygon>("two_bends_left", two_bends, 20.028, 1.5, 0.0);
    test_one<linestring, buf::join_miter, buf::end_flat, polygon>("two_bends_left", two_bends, 20.225, 1.5, 0.0);
    test_one<linestring, buf::join_round, buf::end_flat, polygon>("two_bends_right", two_bends, 19.211, 0.0, 1.5);
    test_one<linestring, buf::join_miter, buf::end_flat, polygon>("two_bends_right", two_bends, 19.288, 0.0, 1.5);


    // Next (and all similar cases) which a offsetted-one-sided buffer has to be fixed. TODO
    //test_one<linestring, buf::join_miter, buf::end_flat, polygon>("two_bends_neg", two_bends, 99, +1.5, -1.0);
    //test_one<linestring, buf::join_miter, buf::end_flat, polygon>("two_bends_pos", two_bends, 99, -1.5, +1.0);
    //test_one<linestring, buf::join_round, buf::end_flat, polygon>("two_bends_neg", two_bends, 99, +1.5, -1.0);
    //test_one<linestring, buf::join_round, buf::end_flat, polygon>("two_bends_pos", two_bends, 99, -1.5, +1.0);

    test_one<linestring, buf::join_round, buf::end_flat, polygon>("overlapping150", overlapping, 65.6786, 1.5, 1.5);
    test_one<linestring, buf::join_miter, buf::end_flat, polygon>("overlapping150", overlapping, 68.140, 1.5, 1.5);

    // Cases below should still be fixed because of remaining flat-end/inside bug

#if defined(BOOST_GEOMETRY_BUFFER_INCLUDE_FAILING_TESTS)
    // Different cases with intersection points on flat and (left/right from line itself)
    test_one<linestring, buf::join_round, buf::end_flat, polygon>("overlapping_asym_150_010", overlapping, 48.308, 1.5, 0.25);
    test_one<linestring, buf::join_miter, buf::end_flat, polygon>("overlapping_asym_150_010", overlapping, 50.770, 1.5, 0.25);
    test_one<linestring, buf::join_round, buf::end_flat, polygon>("overlapping_asym_150_075", overlapping, 58.506, 1.5, 0.75);
    test_one<linestring, buf::join_miter, buf::end_flat, polygon>("overlapping_asym_150_075", overlapping, 60.985, 1.5, 0.75);
    test_one<linestring, buf::join_round, buf::end_flat, polygon>("overlapping_asym_150_100", overlapping, 62.514, 1.5, 1.0);
    test_one<linestring, buf::join_miter, buf::end_flat, polygon>("overlapping_asym_150_100", overlapping, 64.984, 1.5, 1.0);
#endif

#if defined(BOOST_GEOMETRY_BUFFER_INCLUDE_FAILING_TESTS)
    // Having flat end
    test_one<linestring, buf::join_round, buf::end_flat, polygon>("for_collinear", for_collinear, 68.561, 2.0, 2.0);
    test_one<linestring, buf::join_miter, buf::end_flat, polygon>("for_collinear", for_collinear, 72, 2.0, 2.0);
    test_one<linestring, buf::join_round, buf::end_flat, polygon>("for_collinear2", for_collinear2, 74.387, 2.0, 2.0);
    test_one<linestring, buf::join_miter, buf::end_flat, polygon>("for_collinear2", for_collinear2, 78.0, 2.0, 2.0);
#endif

#if defined(BOOST_GEOMETRY_BUFFER_INCLUDE_FAILING_TESTS)
    // Having flat end causing self-intersection
    test_one<linestring, buf::join_round, buf::end_flat, polygon>("curve", curve, 54.8448, 5.0, 3.0);
    test_one<linestring, buf::join_miter, buf::end_flat, polygon>("curve", curve, 55.3875, 5.0, 3.0);
#endif

    test_one<linestring, buf::join_miter, buf::end_flat, polygon>("tripod", tripod, 74.25, 3.0);
    test_one<linestring, buf::join_miter, buf::end_round, polygon>("tripod", tripod, 116.6336, 3.0);

    test_one<linestring, buf::join_round, buf::end_flat, polygon>("chained2", chained2, 11.3137, 2.5, 1.5);
    test_one<linestring, buf::join_round, buf::end_flat, polygon>("chained3", chained3, 16.9706, 2.5, 1.5);
    test_one<linestring, buf::join_round, buf::end_flat, polygon>("chained4", chained4, 22.6274, 2.5, 1.5);

    //test_one<linestring, buf::join_round, buf::end_flat, polygon>("reallife1", reallife1, 99, 16.5, 6.5);

    test_one<linestring, buf::join_miter, buf::end_flat, polygon>("aimes175", aimes175, 2.81111809385947709e-08, 0.000036, 0.000036, true, 0.0001);
    test_one<linestring, buf::join_round, buf::end_round, polygon>("aimes175", aimes175, 3.21215765097804251e-08, 0.000036, 0.000036, true, 0.0001);
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
