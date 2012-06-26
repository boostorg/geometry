// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2011-2012 Adam Wulkiewicz, Lodz, Poland.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_TEST_RTREE_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_TEST_RTREE_HPP

#include <geometry_index_test_common.hpp>

#include <boost/foreach.hpp>
#include <vector>

#include <boost/geometry/extensions/index/rtree/rtree.hpp>

// TODO: fix this issue differently
#ifdef max
#undef max
#endif
#include <boost/geometry/extensions/index/rtree/visitors/are_levels_ok.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/are_boxes_ok.hpp>

template <typename Rtree, typename Iter, typename Value>
Iter test_find(Rtree const& rtree, Iter first, Iter last, Value const& value)
{
    for ( ; first != last ; ++first )
        if ( rtree.translator().equals(value, *first) )
            return first;
    return first;
}

template <typename Rtree, typename Value>
void test_compare_outputs(Rtree const& rtree, std::vector<Value> const& output, std::vector<Value> const& expected_output)
{
    typename Rtree::translator_type tr = rtree.translator();

    BOOST_CHECK( expected_output.size() == output.size() );
    BOOST_FOREACH(Value const& v, expected_output)
    {
        BOOST_CHECK(test_find(rtree, output.begin(), output.end(), v) != output.end() );
    }
}

template <typename Rtree, typename Value, typename Predicates>
void test(Rtree & rtree, Predicates const& pred, std::vector<Value> const& expected_output)
{
    BOOST_CHECK( bgi::are_levels_ok(rtree) );
    BOOST_CHECK( bgi::are_boxes_ok(rtree) );

    std::vector<Value> output;
    size_t n = rtree.query(pred, std::back_inserter(output));

    BOOST_CHECK( expected_output.size() == n );
    test_compare_outputs(rtree, output, expected_output);
}

#endif
