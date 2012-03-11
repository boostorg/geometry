// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2010-2012 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <geometry_test_common.hpp>

#define BOOST_GEOMETRY_TEST_BUFFER_POLYGON
#include <test_buffer.hpp>

#include <boost/geometry/multi/multi.hpp> // TODO: more specific
#include <boost/geometry/multi/geometries/multi_geometries.hpp>
#include <boost/geometry/extensions/algorithms/buffer/multi_buffer_inserter.hpp>



static std::string const simplex
    = "MULTIPOLYGON(((0 1,2 5,5 3,0 1)),((1 1,5 2,5 0,1 1)))";

static std::string const zonethru
    = "MULTIPOLYGON(((0 0,0 6,5 6,5 4,3 4,3 0,0 0)),((5 0,5 2,7 2,7 6,10 6,10 0,5 0)))";

static std::string const wrapped
    = "MULTIPOLYGON(((0 0,0 10,10 10,10 0,0 0),(2 2,8 2,8 8,2 8,2 2)),((4 4,4 6,6 6,6 4,4 4)))";

// From robustness tests

// Case with duplicate points (due to chained boxes) (round)
static std::string const rt_a
    = "MULTIPOLYGON(((2 7,2 8,3 8,3 7,2 7)),((5 4,5 5,6 5,6 4,5 4)),((5 8,6 8,6 7,6 6,5 6,5 7,4 7,4 8,5 8)),((3 5,4 5,4 4,3 4,2 4,2 5,3 5)))";

// Case with u-u (miter)
static std::string const rt_b
    = "MULTIPOLYGON(((8 4,8 5,9 5,9 4,8 4)),((6 2,6 3,7 3,7 2,6 2)),((8 0,8 1,9 1,9 0,8 0)),((9 7,9 8,10 8,10 7,9 7)))";

// Case with geometry::equals( turn.point(7.0000000000000000, 4.3368086899420177e-019), helper_segment(7.0000000000000000, 0.00000000000000000))) (round)
static std::string const rt_c
	= "MULTIPOLYGON(((6 1,6 2,7 2,7 1,6 1)),((8 0,8 1,9 1,9 0,8 0)))";

// Case with round corner on same perpendicular points (round)
static std::string const rt_d
	= "MULTIPOLYGON(((2 2,2 3,3 2,2 2)),((2 5,2 6,3 5,2 5)),((2 4,2 5,3 4,2 4)),((3 2,3 3,4 2,3 2)),((4 4,4 5,5 4,4 4)),((5 6,5 5,4 5,4 6,5 7,5 6)),((2 2,3 1,3 0,2 0,2 1,1 1,1 2,2 2)),((1 3,1 2,0 2,1 3)),((1 4,2 4,2 3,1 3,1 4)))";

// Case with missing turning point (miter) and many intersections (round, OK)
static std::string const rt_e
	= "MULTIPOLYGON(((0 6,0 7,1 6,0 6)),((3 7,3 8,4 8,4 7,3 7)),((4 6,4 7,5 7,4 6)),((3 6,3 7,4 6,3 6)),((1 9,2 10,2 9,1 9)),((1 9,1 8,0 8,0 9,1 9)),((3 5,3 4,2 4,2 5,2 6,3 5)))";

// Extact of e (miter)
static std::string const rt_f
	= "MULTIPOLYGON(((0 6,0 7,1 6,0 6)),((1 9,1 8,0 8,0 9,1 9)))";

static std::string const rt_g
	= "MULTIPOLYGON(((3 8,3 9,4 9,3 8)),((7 5,7 6,8 5,7 5)),((1 8,1 9,2 9,1 8)),((1 6,1 7,2 7,1 6)))";



template <typename P>
void test_all()
{
    namespace buf = bg::strategy::buffer;

    typedef bg::model::polygon<P> polygon_type;
    typedef bg::model::multi_polygon<polygon_type> multi_polygon_type;
    test_one<multi_polygon_type, buf::join_round, polygon_type>("multi_simplex_05", simplex, 'r', 23.7030, 0.5);
    test_one<multi_polygon_type, buf::join_miter, polygon_type>("multi_simplex_05", simplex, 'm', 24.5965, 0.5);
    test_one<multi_polygon_type, buf::join_round, polygon_type>("multi_simplex_10", simplex, 'r', 34.2532, 1.0);
    test_one<multi_polygon_type, buf::join_miter, polygon_type>("multi_simplex_10", simplex, 'm', 38.1379, 1.0);
    test_one<multi_polygon_type, buf::join_round, polygon_type>("multi_simplex_20", simplex, 'r', 59.9159, 2.0);
    test_one<multi_polygon_type, buf::join_miter, polygon_type>("multi_simplex_20", simplex, 'm', 77.7060, 2.0);
    test_one<multi_polygon_type, buf::join_round, polygon_type>("multi_simplex_50", simplex, 'r', 174.46, 5.0);
    test_one<multi_polygon_type, buf::join_miter, polygon_type>("multi_simplex_50", simplex, 'm', 298.797, 5.0);

    test_one<multi_polygon_type, buf::join_round, polygon_type>("zonethru_05", zonethru, 'r', 67.4627, 0.5);
    test_one<multi_polygon_type, buf::join_miter, polygon_type>("zonethru_05", zonethru, 'm', 68.0000, 0.5);
    test_one<multi_polygon_type, buf::join_round, polygon_type>("zonethru_10", zonethru, 'r', 93.8508, 1.0, -999, 1);
    test_one<multi_polygon_type, buf::join_miter, polygon_type>("zonethru_10", zonethru, 'm', 96.0000, 1.0);
    test_one<multi_polygon_type, buf::join_round, polygon_type>("zonethru_15", zonethru, 'r', 114.584, 1.5);
    test_one<multi_polygon_type, buf::join_miter, polygon_type>("zonethru_15", zonethru, 'm', 117.000, 1.5);
    test_one<multi_polygon_type, buf::join_round, polygon_type>("wrapped_05", wrapped, 'r', 104.570, 0.5);
    test_one<multi_polygon_type, buf::join_miter, polygon_type>("wrapped_05", wrapped, 'm', 105.000, 0.5);
    test_one<multi_polygon_type, buf::join_round, polygon_type>("wrapped_10", wrapped, 'r', 142.281, 1.0);
    test_one<multi_polygon_type, buf::join_miter, polygon_type>("wrapped_10", wrapped, 'm', 144.000, 1.0);
    test_one<multi_polygon_type, buf::join_round, polygon_type>("wrapped_15", wrapped, 'r', 167.066, 1.5);
    test_one<multi_polygon_type, buf::join_miter, polygon_type>("wrapped_15", wrapped, 'm', 169.000, 1.5);

    test_one<multi_polygon_type, buf::join_round, polygon_type>("rt_a", rt_a, 'r', 34.5344, 1.0);
    test_one<multi_polygon_type, buf::join_miter, polygon_type>("rt_a", rt_a, 'm', 36, 1.0);
    test_one<multi_polygon_type, buf::join_round, polygon_type>("rt_b", rt_b, 'r', 31.4186, 1.0);
    test_one<multi_polygon_type, buf::join_miter, polygon_type>("rt_b", rt_b, 'm', 34, 1.0);
    test_one<multi_polygon_type, buf::join_round, polygon_type>("rt_c", rt_c, 'r', 14.7093, 1.0);
    test_one<multi_polygon_type, buf::join_miter, polygon_type>("rt_c", rt_c, 'm', 16, 1.0);
    test_one<multi_polygon_type, buf::join_round, polygon_type>("rt_d", rt_d, 'r', 18.8726, 0.3);
    test_one<multi_polygon_type, buf::join_miter, polygon_type>("rt_d", rt_d, 'm', 19.8823, 0.3);
    test_one<multi_polygon_type, buf::join_round, polygon_type>("rt_e", rt_e, 'r', 14.1866, 0.3);
    test_one<multi_polygon_type, buf::join_miter, polygon_type>("rt_e", rt_e, 'm', 15.1198, 0.3);
    test_one<multi_polygon_type, buf::join_round, polygon_type>("rt_f", rt_f, 'r', 4.28937, 0.3);
    test_one<multi_polygon_type, buf::join_miter, polygon_type>("rt_f", rt_f, 'm', 4.60853, 0.3);
    test_one<multi_polygon_type, buf::join_round, polygon_type>("rt_g", rt_g, 'r', 24.719, 1.0);
    test_one<multi_polygon_type, buf::join_miter, polygon_type>("rt_g", rt_g, 'm', 30.3137, 1.0);
}


#ifdef HAVE_TTMATH
#include <ttmath_stub.hpp>
#endif

int test_main(int, char* [])
{
    test_all<bg::model::point<double, 2, bg::cs::cartesian> >();
    //test_all<bg::model::point<tt, 2, bg::cs::cartesian> >();
    
    return 0;
}

