// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2015, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_TEST_MODULE
#define BOOST_TEST_MODULE test_douglas_peucker
#endif

#ifdef BOOST_GEOMETRY_TEST_DEBUG
#ifndef BOOST_GEOMETRY_DEBUG_DOUGLAS_PEUCKER
#define BOOST_GEOMETRY_DEBUG_DOUGLAS_PEUCKER
#endif
#endif

#include <iostream>
#include <iterator>
#include <sstream>
#include <string>

#include <boost/test/included/unit_test.hpp>

#include <boost/assign/list_of.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/tuple/tuple.hpp>

#include <boost/geometry/core/point_type.hpp>
#include <boost/geometry/core/tags.hpp>

#include <boost/geometry/strategies/distance.hpp>
#include <boost/geometry/strategies/strategies.hpp>

#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/geometries/adapted/boost_tuple.hpp>
#include <boost/geometry/geometries/register/multi_point.hpp>

#include <boost/geometry/algorithms/equals.hpp>
#include <boost/geometry/algorithms/comparable_distance.hpp>

#include <boost/geometry/io/wkt/wkt.hpp>
#include <boost/geometry/io/dsv/write.hpp>


namespace bg = ::boost::geometry;
namespace ba = ::boost::assign;
namespace services = bg::strategy::distance::services;

typedef boost::tuple<double, double> tuple_point_type;
typedef std::vector<tuple_point_type> tuple_multi_point_type;

BOOST_GEOMETRY_REGISTER_BOOST_TUPLE_CS(cs::cartesian)
BOOST_GEOMETRY_REGISTER_MULTI_POINT(tuple_multi_point_type)
BOOST_GEOMETRY_REGISTER_MULTI_POINT_TEMPLATED(std::vector)

typedef bg::strategy::distance::projected_point<> distance_strategy_type;
typedef bg::strategy::distance::projected_point
    <
        void, bg::strategy::distance::comparable::pythagoras<>
    > comparable_distance_strategy_type;


template <typename CoordinateType>
struct default_simplify_strategy
{
    typedef bg::model::point<CoordinateType, 2, bg::cs::cartesian> point_type;
    typedef typename bg::strategy::distance::services::default_strategy
        <
            bg::point_tag, bg::segment_tag, point_type
        >::type default_distance_strategy_type;

    typedef bg::strategy::simplify::douglas_peucker
        <
            point_type, default_distance_strategy_type
        > type;
};


template <typename CoordinateType>
struct simplify_regular_distance_strategy
{
    typedef bg::model::point<CoordinateType, 2, bg::cs::cartesian> point_type;
    typedef bg::strategy::simplify::douglas_peucker
        <
            point_type, distance_strategy_type
        > type;
};

template <typename CoordinateType>
struct simplify_comparable_distance_strategy
{
    typedef bg::model::point<CoordinateType, 2, bg::cs::cartesian> point_type;
    typedef bg::strategy::simplify::douglas_peucker
        <
            point_type, comparable_distance_strategy_type
        > type;
};



template <typename Geometry>
inline Geometry from_wkt(std::string const& wkt)
{
    Geometry geometry;
    boost::geometry::read_wkt(wkt, geometry);
    return geometry;
}

template <typename Iterator>
inline std::ostream& print_point_range(std::ostream& os,
                                       Iterator first,
                                       Iterator beyond,
                                       std::string const& header)
{
    os << header << "(";
    for (Iterator it = first; it != beyond; ++it)
    {
        os << " " << bg::dsv(*it);
    }
    os << " )";
    return os;
}


struct equals
{
    template <typename Iterator1, typename Iterator2>
    static inline bool apply(Iterator1 begin1, Iterator1 end1,
                             Iterator2 begin2, Iterator2 end2)
    {
        std::size_t num_points1 = std::distance(begin1, end1);
        std::size_t num_points2 = std::distance(begin2, end2);

        if ( num_points1 != num_points2 )
        {
            return false;
        }

        Iterator1 it1 = begin1;
        Iterator2 it2 = begin2;
        for (; it1 != end1; ++it1, ++it2)
        {
            if ( !bg::equals(*it1, *it2) )
            {
                return false;
            }
        }
        return true;
    }

    template <typename Range1, typename Range2>
    static inline bool apply(Range1 const& range1, Range2 const& range2)
    {
        return apply(boost::begin(range1), boost::end(range1),
                     boost::begin(range2), boost::end(range2));
    }
};




template <typename Geometry>
struct test_one_case
{
    template <typename Strategy, typename Range>
    static inline void apply(std::string const& wkt,
                             double max_distance,
                             Strategy const& strategy,
                             Range const& expected_result,
                             std::string const& case_id)
    {
        typedef typename bg::point_type<Geometry>::type point_type;
        std::vector<point_type> result;

        Geometry geometry = from_wkt<Geometry>(wkt);

        std::string typeid_name
            = typeid(typename bg::coordinate_type<point_type>::type).name();

#ifdef BOOST_GEOMETRY_TEST_DEBUG
        std::cout << case_id << " - " << typeid_name
                  << std::endl;
        std::cout << wkt << std::endl;
#endif

        strategy.apply(geometry, std::back_inserter(result), max_distance);

#ifdef BOOST_GEOMETRY_TEST_DEBUG
        print_point_range(std::cout, boost::begin(result), boost::end(result),
                          "output: ");
        std::cout << std::endl << std::endl;
#endif
        std::stringstream sstr;
        print_point_range(sstr, boost::begin(expected_result),
                          boost::end(expected_result),
                          "");

        BOOST_CHECK_MESSAGE(equals::apply(result, expected_result),
                            "case id: " << case_id << " - " << typeid_name
                            << ", geometry: " << wkt
                            << ", Expected: " << sstr.str()
                            << " - Detected: " << bg::wkt(result));

#ifdef BOOST_GEOMETRY_TEST_DEBUG
        std::cout << "---------------" << std::endl;
        std::cout << "---------------" << std::endl;
        std::cout << std::endl << std::endl;
#endif
    }
};


template <typename CoordinateType, typename Strategy>
inline void test_with_strategy()
{
    std::cout.precision(20);
    Strategy strategy;

    typedef bg::model::point<CoordinateType, 2, bg::cs::cartesian> point_type;
    typedef bg::model::linestring<point_type> linestring_type;
    typedef bg::model::segment<point_type> segment_type;
    typedef test_one_case<linestring_type> tester;

    linestring_type ls1, ls2;
    linestring_type res1, res2;
    point_type const p1(-6,-13), p2(0,-15);
    segment_type const s(point_type(12,-3), point_type(-12,5));

    if (bg::comparable_distance(p1, s) >= bg::comparable_distance(p2, s))
    {
        tester::apply("LINESTRING(12 -3, 4 8,-6 -13,-9 4,0 -15,-12 5)",
                      10,
                      strategy,
                      ba::tuple_list_of(12,-3)(4,8)(-6,-13)(-12,5),
                      "l01");
    }
    else
    {
        tester::apply("LINESTRING(12 -3, 4 8,-6 -13,-9 4,0 -15,-12 5)",
                      10,
                      strategy,
                      ba::tuple_list_of(12,-3)(4,8)(-6,-13)(-9,4)(0,-15)(-12,5),
                      "l01");
    }

    tester::apply("LINESTRING(-6 -13,-9 4,0 -15,-12 5)",
                  10,
                  strategy,
                  ba::tuple_list_of(-6,-13)(-12,5),
                  "l02");

    tester::apply("LINESTRING(12 -3, 4 8,-6 -13,-9 4,0 -14,-12 5)",
                  10,
                  strategy,
                  ba::tuple_list_of(12,-3)(4,8)(-6,-13)(-12,5),
                  "l03");

    tester::apply("LINESTRING(12 -3, 4 8,-6 -13,-9 4,0 -14,-12 5)",
                  14,
                  strategy,
                  ba::tuple_list_of(12,-3)(-6,-13)(-12,5),
                  "l04");

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << "*************************************************";
    std::cout << std::endl;
    std::cout << std::endl;
#endif
}


BOOST_AUTO_TEST_CASE( test_default_strategy )
{
    test_with_strategy<int, default_simplify_strategy<int>::type>();
    test_with_strategy<float, default_simplify_strategy<float>::type>();
    test_with_strategy<double, default_simplify_strategy<double>::type>();
    test_with_strategy
        <
            long double,
            default_simplify_strategy<long double>::type
        >();
}

BOOST_AUTO_TEST_CASE( test_with_regular_distance_strategy )
{
    test_with_strategy
        <
            int,
            simplify_regular_distance_strategy<int>::type
        >();

    test_with_strategy
        <
            float,
            simplify_regular_distance_strategy<float>::type
        >();

    test_with_strategy
        <
            double,
            simplify_regular_distance_strategy<double>::type
        >();
    test_with_strategy
        <
            long double,
            simplify_regular_distance_strategy<long double>::type
        >();
}

BOOST_AUTO_TEST_CASE( test_with_comparable_distance_strategy )
{
    test_with_strategy
        <
            int,
            simplify_comparable_distance_strategy<int>::type
        >();
    test_with_strategy
        <
            float,
            simplify_comparable_distance_strategy<float>::type
        >();
    test_with_strategy
        <
            double,
            simplify_comparable_distance_strategy<double>::type
        >();
    test_with_strategy
        <
            long double,
            simplify_comparable_distance_strategy<long double>::type
        >();
}
