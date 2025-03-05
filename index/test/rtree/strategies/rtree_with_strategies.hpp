// Boost.Geometry Index
// Unit Test

// Copyright (c) 2025 Adam Wulkiewicz, Lodz, Poland.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_INDEX_TEST_RTREE_STRATEGIES_HPP
#define BOOST_GEOMETRY_INDEX_TEST_RTREE_STRATEGIES_HPP

#include <vector>

#define BOOST_GEOMETRY_INDEX_DETAIL_EXPERIMENTAL_PREDICATES

#include <geometry_index_test_common.hpp>

#include <boost/geometry/geometries/register/point.hpp>
#include <boost/geometry/geometries/register/box.hpp>
#include <boost/geometry/geometries/register/segment.hpp>

struct point { double x, y; };
struct box { point mi, ma; };
struct segment { point first, second; };

BOOST_GEOMETRY_REGISTER_POINT_2D(point, double, cs::undefined, x, y)
BOOST_GEOMETRY_REGISTER_BOX(box, point, mi, ma)
BOOST_GEOMETRY_REGISTER_SEGMENT(segment, point, first, second)

template <typename QueryValue, typename Rtree>
constexpr bool is_bp_or_bb_v = (bg::util::is_box<QueryValue>::value
                                && bg::util::is_point<typename Rtree::value_type>::value)
                            || (bg::util::is_box<QueryValue>::value
                                && bg::util::is_box<typename Rtree::value_type>::value);

template <typename QueryValue, typename Rtree>
constexpr bool is_pb_or_bb_v = (bg::util::is_point<QueryValue>::value
                                && bg::util::is_box<typename Rtree::value_type>::value)
                            || (bg::util::is_box<QueryValue>::value
                                && bg::util::is_box<typename Rtree::value_type>::value);

template <typename QueryValue, typename Rtree>
constexpr bool is_pp_or_bb_v = (bg::util::is_point<QueryValue>::value
                                && bg::util::is_point<typename Rtree::value_type>::value)
                            || (bg::util::is_box<QueryValue>::value
                                && bg::util::is_box<typename Rtree::value_type>::value);

template
<
    typename QueryValue, typename Rtree,
    std::enable_if_t<is_bp_or_bb_v<QueryValue, Rtree>, int> = 0
>
void test_queries_bp_bb(Rtree const& rtree)
{
    // These predicates use algorithms that are not implemented for
    // some geometry combinations
    std::vector<typename Rtree::value_type> values;
    rtree.query(bgi::covered_by(QueryValue{}), std::back_inserter(values));
    rtree.query(bgi::disjoint(QueryValue{}), std::back_inserter(values));
    rtree.query(bgi::within(QueryValue{}), std::back_inserter(values));
}

template
<
    typename QueryValue, typename Rtree,
    std::enable_if_t<(!is_bp_or_bb_v<QueryValue, Rtree>), int> = 0
>
void test_queries_bp_bb(Rtree const& ) {}

template
<
    typename QueryValue, typename Rtree,
    std::enable_if_t<is_pb_or_bb_v<QueryValue, Rtree>, int> = 0
>
void test_queries_pb_bb(Rtree const& rtree)
{
    // These predicates use algorithms that are not implemented for
    // some geometry combinations
    std::vector<typename Rtree::value_type> values;
    rtree.query(bgi::contains(QueryValue{}), std::back_inserter(values));
    rtree.query(bgi::covers(QueryValue{}), std::back_inserter(values));
}

template
<
    typename QueryValue, typename Rtree,
    std::enable_if_t<(!is_pb_or_bb_v<QueryValue, Rtree>), int> = 0
>
void test_queries_pb_bb(Rtree const& ) {}

template
<
    typename QueryValue, typename Rtree,
    std::enable_if_t<is_pp_or_bb_v<QueryValue, Rtree>, int> = 0
>
void test_queries_pp_bb(Rtree const& rtree)
{
    // These predicates use algorithms that are not implemented for
    // some geometry combinations
    std::vector<typename Rtree::value_type> values;
    rtree.query(bgi::overlaps(QueryValue{}), std::back_inserter(values));
    rtree.query(bgi::touches(QueryValue{}), std::back_inserter(values));
}

template
<
    typename QueryValue, typename Rtree,
    std::enable_if_t<(!is_pp_or_bb_v<QueryValue, Rtree>), int> = 0
>
void test_queries_pp_bb(Rtree const& ) {}

template <typename QueryValue, typename Rtree>
void test_queries(Rtree const& rtree)
{
    std::vector<typename Rtree::value_type> values;
    rtree.query(bgi::intersects(QueryValue{}), std::back_inserter(values));
    rtree.query(bgi::nearest(QueryValue{}, 1), std::back_inserter(values));
    test_queries_bp_bb<QueryValue>(rtree);
    test_queries_pb_bb<QueryValue>(rtree);
    test_queries_pp_bb<QueryValue>(rtree);
}

template <typename Value, typename Params, typename Strategies>
void test_strategies()
{
    std::vector<Value> values;
    bgi::rtree<Value, bgi::parameters<Params, Strategies>> rtree{values};
    rtree.insert(Value{});
    rtree.remove(Value{});

    test_queries<point>(rtree);
    test_queries<box>(rtree);
    test_queries<segment>(rtree);
}

template <typename Value, typename Params>
void test_rtree()
{
    test_strategies<Value, Params, bg::strategies::index::cartesian<>>();
    test_strategies<Value, Params, bg::strategies::cartesian<>>();
    test_strategies<Value, Params, bg::strategies::index::spherical<>>();
    test_strategies<Value, Params, bg::strategies::spherical<>>();
    test_strategies<Value, Params, bg::strategies::index::geographic<>>();
    test_strategies<Value, Params, bg::strategies::geographic<>>();
}

#endif // BOOST_GEOMETRY_INDEX_TEST_RTREE_STRATEGIES_HPP
