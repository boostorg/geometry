// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2007-2014 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2014 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2014 Mateusz Loskot, London, UK.
// Copyright (c) 2013-2014 Adam Wulkiewicz, Lodz, Poland.

// This file was modified by Oracle on 2014-2021.
// Modifications copyright (c) 2014-2021, Oracle and/or its affiliates.
// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <geometry_test_common.hpp>

#include <boost/geometry/algorithms/comparable_distance.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/io/wkt/read.hpp>
#include <boost/geometry/strategies/strategies.hpp>
#include <boost/geometry/util/type_traits.hpp>

#include <boost/variant/variant.hpp>

#include <type_traits>

namespace bg = boost::geometry;

template <typename T>
struct test_variant_different_default_strategy
{
    static inline void apply()
    {
        typedef bg::model::point<T, 2, bg::cs::cartesian> point_type;
        typedef bg::model::segment<point_type> segment_type;
        typedef bg::model::box<point_type> box_type;
        typedef boost::variant<point_type, segment_type, box_type> variant_type;

        point_type point;
        bg::read_wkt("POINT(1 3)", point);

        segment_type seg;
        bg::read_wkt("LINESTRING(1 1,4 4)", seg);

        box_type box;
        bg::read_wkt("BOX(-1 -1,0 0)", box);

        variant_type v1, v2;

        using variant_cdistance_t = typename bg::comparable_distance_result
            <
                variant_type, variant_type, bg::default_strategy
            >::type;
        using cdistance_t =  typename bg::comparable_distance_result
            <
                point_type, point_type, bg::default_strategy
            >::type;
        BOOST_GEOMETRY_STATIC_ASSERT(
            (std::is_same<variant_cdistance_t, cdistance_t>::value),
            "Unexpected result type",
            variant_cdistance_t, cdistance_t);

        // Default strategy
        v1 = point;
        v2 = point;
        BOOST_CHECK_CLOSE(bg::comparable_distance(v1, v2),
                          bg::comparable_distance(point, point),
                          0.0001);
        BOOST_CHECK_CLOSE(bg::comparable_distance(v1, point),
                          bg::comparable_distance(point, point),
                          0.0001);
        BOOST_CHECK_CLOSE(bg::comparable_distance(point, v2),
                          bg::comparable_distance(point, point),
                          0.0001);
        v1 = point;
        v2 = seg;
        BOOST_CHECK_CLOSE(bg::comparable_distance(v1, v2),
                          bg::comparable_distance(point, seg),
                          0.0001);
        BOOST_CHECK_CLOSE(bg::comparable_distance(v1, seg),
                          bg::comparable_distance(point, seg),
                          0.0001);
        BOOST_CHECK_CLOSE(bg::comparable_distance(point, v2),
                          bg::comparable_distance(point, seg), 0.0001);
        v1 = point;
        v2 = box;
        BOOST_CHECK_CLOSE(bg::comparable_distance(v1, v2),
                          bg::comparable_distance(point, box),
                          0.0001);
        BOOST_CHECK_CLOSE(bg::comparable_distance(v1, box),
                          bg::comparable_distance(point, box),
                          0.0001);
        BOOST_CHECK_CLOSE(bg::comparable_distance(point, v2),
                          bg::comparable_distance(point, box), 0.0001);
    }
};

template <typename T, typename ExpectedResultType = double>
struct test_variant_same_default_strategy
{
    static inline void apply()
    {
        typedef bg::model::point<T, 2, bg::cs::cartesian> point_type;
        typedef bg::model::segment<point_type> segment_type;
        typedef bg::model::linestring<point_type> linestring_type;
        typedef boost::variant
            <
                point_type, segment_type, linestring_type
            > variant_type;

        point_type point;
        bg::read_wkt("POINT(1 3)", point);

        segment_type seg;
        bg::read_wkt("LINESTRING(1 1,4 4)", seg);

        linestring_type linestring;
        bg::read_wkt("LINESTRING(-1 -1,-1 0,0 0,0 -1,-1 -1)", linestring);

        variant_type v1, v2;

        using variant_cdistance_t = typename bg::comparable_distance_result
            <
                variant_type, variant_type, bg::default_strategy
            >::type;
        BOOST_GEOMETRY_STATIC_ASSERT(
            (std::is_same<variant_cdistance_t, ExpectedResultType>::value),
            "Unexpected result type",
            variant_cdistance_t, ExpectedResultType);

        using cdistance_t = typename bg::comparable_distance_result
            <
                point_type, point_type, bg::default_strategy
            >::type;
        BOOST_GEOMETRY_STATIC_ASSERT(
            (std::is_same<cdistance_t, ExpectedResultType>::value),
            "Unexpected result type",
            cdistance_t, ExpectedResultType);

        // Default strategy
        v1 = point;
        v2 = point;
        BOOST_CHECK_CLOSE(bg::comparable_distance(v1, v2),
                          bg::comparable_distance(point, point),
                          0.0001);
        BOOST_CHECK_CLOSE(bg::comparable_distance(v1, point),
                          bg::comparable_distance(point, point),
                          0.0001);
        BOOST_CHECK_CLOSE(bg::comparable_distance(point, v2),
                          bg::comparable_distance(point, point),
                          0.0001);
        v1 = point;
        v2 = seg;
        BOOST_CHECK_CLOSE(bg::comparable_distance(v1, v2),
                          bg::comparable_distance(point, seg),
                          0.0001);
        BOOST_CHECK_CLOSE(bg::comparable_distance(v1, seg),
                          bg::comparable_distance(point, seg),
                          0.0001);
        BOOST_CHECK_CLOSE(bg::comparable_distance(point, v2),
                          bg::comparable_distance(point, seg),
                          0.0001);
        v1 = point;
        v2 = linestring;
        BOOST_CHECK_CLOSE(bg::comparable_distance(v1, v2),
                          bg::comparable_distance(point, linestring),
                          0.0001);
        BOOST_CHECK_CLOSE(bg::comparable_distance(v1, linestring),
                          bg::comparable_distance(point, linestring),
                          0.0001);
        BOOST_CHECK_CLOSE(bg::comparable_distance(point, v2),
                          bg::comparable_distance(point, linestring),
                          0.0001);
    }
};

template <typename T, typename ExpectedResultType = T>
struct test_variant_with_strategy
{
    static inline void apply()
    {
        typedef bg::strategy::distance::projected_point<T> strategy_type;

        typedef bg::model::point<T, 2, bg::cs::cartesian> point_type;
        typedef bg::model::segment<point_type> segment_type;
        typedef bg::model::linestring<point_type> linestring_type;
        typedef bg::model::multi_linestring
            <
                linestring_type
            > multi_linestring_type;
        typedef boost::variant
            <
                segment_type, linestring_type, multi_linestring_type
            > variant_type;

        segment_type seg;
        bg::read_wkt("LINESTRING(1 1,4 4)", seg);

        linestring_type ls;
        bg::read_wkt("LINESTRING(-1 -1,-1 0,0 0,0 -1,-1 -1)", ls);

        multi_linestring_type mls;
        bg::read_wkt("MULTILINESTRING((10 0,20 0),(30 0,40 0))", mls);

        variant_type v1, v2;

        strategy_type strategy;

        using variant_cdistance_t = typename bg::comparable_distance_result
            <
                variant_type, variant_type, strategy_type
            >::type;
        BOOST_GEOMETRY_STATIC_ASSERT(
            (std::is_same<variant_cdistance_t, ExpectedResultType>::value),
            "Unexpected result type",
            variant_cdistance_t, ExpectedResultType);

        using cdistance_t = typename bg::comparable_distance_result
            <
                segment_type, linestring_type, strategy_type
            >::type;
        BOOST_GEOMETRY_STATIC_ASSERT(
            (std::is_same<cdistance_t, ExpectedResultType>::value),
            "Unexpected result type",
            cdistance_t, ExpectedResultType);

        // Passed strategy
        v1 = seg;
        v2 = seg;
        BOOST_CHECK_CLOSE(bg::comparable_distance(v1, v2, strategy),
                          bg::comparable_distance(seg, seg, strategy),
                          0.0001);
        BOOST_CHECK_CLOSE(bg::comparable_distance(v1, seg, strategy),
                          bg::comparable_distance(seg, seg, strategy),
                          0.0001);
        BOOST_CHECK_CLOSE(bg::comparable_distance(seg, v2, strategy),
                          bg::comparable_distance(seg, seg, strategy),
                          0.0001);
        v1 = seg;
        v2 = ls;
        BOOST_CHECK_CLOSE(bg::comparable_distance(v1, v2, strategy),
                          bg::comparable_distance(seg, ls, strategy),
                          0.0001);
        BOOST_CHECK_CLOSE(bg::comparable_distance(v1, ls, strategy),
                          bg::comparable_distance(seg, ls, strategy),
                          0.0001);
        BOOST_CHECK_CLOSE(bg::comparable_distance(seg, v2, strategy),
                          bg::comparable_distance(seg, ls, strategy),
                          0.0001);
        v1 = seg;
        v2 = mls;
        BOOST_CHECK_CLOSE(bg::comparable_distance(v1, v2, strategy),
                          bg::comparable_distance(seg, mls, strategy),
                          0.0001);
        BOOST_CHECK_CLOSE(bg::comparable_distance(v1, mls, strategy),
                          bg::comparable_distance(seg, mls, strategy),
                          0.0001);
        BOOST_CHECK_CLOSE(bg::comparable_distance(seg, v2, strategy),
                          bg::comparable_distance(seg, mls, strategy),
                          0.0001);
        v1 = ls;
        v2 = mls;
        BOOST_CHECK_CLOSE(bg::comparable_distance(v1, v2, strategy),
                          bg::comparable_distance(ls, mls, strategy),
                          0.0001);
        BOOST_CHECK_CLOSE(bg::comparable_distance(v1, mls, strategy),
                          bg::comparable_distance(ls, mls, strategy),
                          0.0001);
        BOOST_CHECK_CLOSE(bg::comparable_distance(ls, v2, strategy),
                          bg::comparable_distance(ls, mls, strategy),
                          0.0001);
    }
};

template <typename T, bool IsIntergral = std::is_integral<T>::value>
struct check_result
{
    template <typename ExpectedResult>
    static inline void apply(T const& value,
                             ExpectedResult const& expected_value)
    {
        BOOST_CHECK_EQUAL(value, expected_value);
    }
};

template <typename T>
struct check_result<T, false>
{
    template <typename ExpectedResult>
    static inline void apply(T const& value,
                             ExpectedResult const& expected_value)
    {
        BOOST_CHECK_CLOSE(value, expected_value, 0.0001);
    }
};

template <typename T>
struct test_variant_boxes
{
    static inline void apply()
    {
        typedef bg::model::point<T, 2, bg::cs::cartesian> point_type;
        typedef bg::model::box<point_type> box_type;
        typedef boost::variant<box_type> variant_type;

        box_type box1, box2;
        bg::read_wkt("BOX(-1 -1,0 0)", box1);
        bg::read_wkt("BOX(1 1,2 2)", box2);

        variant_type v1 = box1, v2 = box2;

        typedef typename std::conditional
            <
                std::is_floating_point<T>::value,
                double,
                typename bg::util::detail::default_integral::type
            >::type expected_result_type;

        using variant_cdistance_t = typename bg::comparable_distance_result
            <
                variant_type, variant_type, bg::default_strategy
            >::type;
        BOOST_GEOMETRY_STATIC_ASSERT(
            (std::is_same<variant_cdistance_t, expected_result_type>::value),
            "Unexpected result type",
            variant_cdistance_t, expected_result_type);

        // Default strategy
        check_result<T>::apply(bg::comparable_distance(v1, v2),
                               bg::comparable_distance(box1, box2));
        check_result<T>::apply(bg::comparable_distance(v1, box2),
                               bg::comparable_distance(box1, box2));
        check_result<T>::apply(bg::comparable_distance(box1, v2),
                               bg::comparable_distance(box1, box2));
    }
};


int test_main(int, char* [])
{
    // test variant support
    test_variant_different_default_strategy<double>::apply();

    test_variant_same_default_strategy<double>::apply();
    test_variant_same_default_strategy<int>::apply();
    test_variant_same_default_strategy<long>::apply();

    test_variant_with_strategy<double>::apply();
    test_variant_with_strategy<float>::apply();
    test_variant_with_strategy<long double>::apply();
    test_variant_with_strategy<int, double>::apply();

    test_variant_boxes<double>::apply();
    test_variant_boxes<int>::apply();
    test_variant_boxes<long>::apply();

    return 0;
}
