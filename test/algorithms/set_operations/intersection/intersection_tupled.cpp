// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2020, Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#include <geometry_test_common.hpp>

#include <boost/geometry/algorithms/correct.hpp>
#include <boost/geometry/algorithms/equals.hpp>
#include <boost/geometry/algorithms/intersection.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/io/wkt/wkt.hpp>
#include <boost/geometry/strategies/cartesian/area.hpp>
#include <boost/geometry/strategies/cartesian/intersection.hpp>
#include <boost/geometry/strategies/cartesian/point_in_poly_winding.hpp>
#include <boost/geometry/strategies/cartesian/point_in_point.hpp>

#include <boost/tuple/tuple.hpp>

typedef bg::model::point<double, 2, bg::cs::cartesian> Pt;
typedef bg::model::linestring<Pt> Ls;
typedef bg::model::polygon<Pt> Po;
typedef bg::model::ring<Pt> R;
typedef bg::model::multi_point<Pt> MPt;
typedef bg::model::multi_linestring<Ls> MLs;
typedef bg::model::multi_polygon<Po> MPo;

#ifdef BOOST_GEOMETRY_CXX11_TUPLE

#include <tuple>

#endif

template <typename G>
inline void check(G const& g, std::string const& expected)
{
    if (expected.empty())
    {
        BOOST_CHECK_MESSAGE(
            boost::empty(g),
            bg::wkt(g) << " expected to be empty"
        );
    }
    else
    {
        G expect;
        bg::read_wkt(expected, expect);
        bg::correct(expect);
        BOOST_CHECK_MESSAGE(
            bg::equals(g, expect),
            bg::wkt(g) << " different than expected: " << expected
        );
    }
}

inline void check(boost::tuple<MPt, MLs, MPo> const& tup,
                  std::string const& out_p_str,
                  std::string const& out_l_str,
                  std::string const& out_a_str)
{
    check(boost::get<0>(tup), out_p_str);
    check(boost::get<1>(tup), out_l_str);
    check(boost::get<2>(tup), out_a_str);
}

inline void check(std::pair<MPt, MLs> const& pair,
                  std::string const& out_p_str,
                  std::string const& out_l_str,
                  std::string const& )
{
    check(pair.first, out_p_str);
    check(pair.second, out_l_str);
}

#ifdef BOOST_GEOMETRY_CXX11_TUPLE

inline void check(std::tuple<MPt, MLs, MPo> const& tup,
                  std::string const& out_p_str,
                  std::string const& out_l_str,
                  std::string const& out_a_str)
{
    check(std::get<0>(tup), out_p_str);
    check(std::get<1>(tup), out_l_str);
    check(std::get<2>(tup), out_a_str);
}

#endif

template <typename In1, typename In2, typename Tup>
inline void test_one(std::string const& in1_str,
                     std::string const& in2_str,
                     std::string const& out_p_str = "",
                     std::string const& out_l_str = "",
                     std::string const& out_a_str = "")
{
    In1 in1;
    bg::read_wkt(in1_str, in1);
    bg::correct(in1);

    In2 in2;
    bg::read_wkt(in2_str, in2);
    bg::correct(in2);

    {
        Tup result;
        bg::intersection(in1, in2, result);
        check(result, out_p_str, out_l_str, out_a_str);
    }
    {
        Tup result;
        bg::intersection(in2, in1, result);
        check(result, out_p_str, out_l_str, out_a_str);
    }
}

template <typename Tup>
inline void test_pp()
{
    test_one<Pt, Pt, Tup>(
        "POINT(0 0)",
        "POINT(0 0)",
        "MULTIPOINT(0 0)");

    test_one<Pt, MPt, Tup>(
        "POINT(0 0)",
        "MULTIPOINT(0 0, 1 1)",
        "MULTIPOINT(0 0)");

    test_one<MPt, MPt, Tup>(
        "MULTIPOINT(0 0, 1 1, 2 2)",
        "MULTIPOINT(1 1, 2 2, 3 3)",
        "MULTIPOINT(1 1, 2 2)");
}

template <typename Tup>
inline void test_pl()
{
    test_one<Pt, Ls, Tup>(
        "POINT(0 0)",
        "LINESTRING(0 0, 1 1)",
        "MULTIPOINT(0 0)");

    test_one<Pt, MLs, Tup>(
        "POINT(1 1)",
        "MULTILINESTRING((0 0, 1 1),(1 1, 2 2),(4 4, 5 5))",
        "MULTIPOINT(1 1)");

    test_one<MPt, Ls, Tup>(
        "MULTIPOINT(0 0, 1 1, 2 2, 3 3)",
        "LINESTRING(0 0, 1 1)",
        "MULTIPOINT(0 0, 1 1)");

    test_one<MPt, MLs, Tup>(
        "MULTIPOINT(0 0, 1 1, 2 2, 3 3)",
        "MULTILINESTRING((0 0, 1 1),(1 1, 2 2),(4 4, 5 5))",
        "MULTIPOINT(0 0, 1 1, 2 2)");
}

template <typename Tup>
inline void test_pa()
{
    test_one<Pt, R, Tup>(
        "POINT(0 0)",
        "POLYGON((0 0, 0 5, 5 5, 5 0, 0 0))",
        "MULTIPOINT(0 0)");

    test_one<Pt, Po, Tup>(
        "POINT(0 0)",
        "POLYGON((0 0, 0 5, 5 5, 5 0, 0 0),(0 0, 4 1, 4 4, 1 4, 0 0))",
        "MULTIPOINT(0 0)");

    test_one<Pt, MPo, Tup>(
        "POINT(0 0)",
        "MULTIPOLYGON(((0 0, 0 5, 5 5, 5 0, 0 0),(0 0, 4 1, 4 4, 1 4, 0 0)),((0 0, 1 2, 2 2, 2 1, 0 0)))",
        "MULTIPOINT(0 0)");

    test_one<MPt, R, Tup>(
        "MULTIPOINT(0 0, 1 1, 2 2, 3 3, 4 4, 5 5, 6 6)",
        "POLYGON((0 0, 0 5, 5 5, 5 0, 0 0))",
        "MULTIPOINT(0 0, 1 1, 2 2, 3 3, 4 4, 5 5)");

    test_one<MPt, Po, Tup>(
        "MULTIPOINT(0 0, 1 1, 2 2, 3 3, 4 4, 5 5, 6 6)",
        "POLYGON((0 0, 0 5, 5 5, 5 0, 0 0),(0 0, 4 1, 4 4, 1 4, 0 0))",
        "MULTIPOINT(0 0, 4 4, 5 5)");

    test_one<MPt, MPo, Tup>(
        "MULTIPOINT(0 0, 1 1, 2 2, 3 3, 4 4, 5 5, 6 6)",
        "MULTIPOLYGON(((0 0, 0 5, 5 5, 5 0, 0 0),(0 0, 4 1, 4 4, 1 4, 0 0)),((0 0, 1 2, 2 2, 2 1, 0 0)))",
        "MULTIPOINT(0 0, 1 1, 2 2, 4 4, 5 5)");
}

template <typename Tup>
inline void test_ll()
{
    test_one<Ls, Ls, Tup>(
        "LINESTRING(0 0, 1 0, 2 1, 3 0)",
        "LINESTRING(0 0, 1 0, 3 0, 4 0)",
        "MULTIPOINT(3 0)",
        "MULTILINESTRING((0 0, 1 0))");

    test_one<Ls, MLs, Tup>(
        "LINESTRING(0 0, 1 0, 2 1, 3 0)",
        "MULTILINESTRING((0 0, 1 0, 3 0),(2 1, 2 2))",
        "MULTIPOINT(3 0, 2 1)",
        "MULTILINESTRING((0 0, 1 0))");

    test_one<MLs, MLs, Tup>(
        "MULTILINESTRING((0 0, 1 0, 2 1),(2 1, 3 0))",
        "MULTILINESTRING((0 0, 1 0, 3 0),(2 1, 2 2))",
        "MULTIPOINT(3 0, 2 1)",
        "MULTILINESTRING((0 0, 1 0))");
}

template <typename Tup>
inline void test_pair()
{
    test_pp<Tup>();
    test_pl<Tup>();
    test_pa<Tup>();
    test_ll<Tup>();
}

template <typename Tup>
inline void test_tuple()
{
    test_pp<Tup>();
    test_pl<Tup>();
    test_pa<Tup>();
    test_ll<Tup>();
    //test_la<Tup>();
    //test_aa<Tup>();
}

int test_main(int, char* [])
{
    test_pair<std::pair<MPt, MLs> >();
    test_tuple<boost::tuple<MPt, MLs, MPo> >();

#ifdef BOOST_GEOMETRY_CXX11_TUPLE
    test_tuple<std::tuple<MPt, MLs, MPo> >();
#endif

    return 0;
}
