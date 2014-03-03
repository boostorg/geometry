// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2012 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2012 Mateusz Loskot, London, UK.

// This file was modified by Oracle on 2014.
// Modifications copyright (c) 2014 Oracle and/or its affiliates.

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

#ifndef BOOST_GEOMETRY_TEST_ALGORITHMS_OVERLAY_TEST_GET_TURNS_HPP
#define BOOST_GEOMETRY_TEST_ALGORITHMS_OVERLAY_TEST_GET_TURNS_HPP

#include <iostream>
#include <iomanip>

#include <geometry_test_common.hpp>

#include <boost/geometry/strategies/strategies.hpp>

#include <boost/geometry/algorithms/detail/overlay/get_turns.hpp>

#include <boost/geometry/algorithms/detail/overlay/debug_turn_info.hpp>

#include <boost/geometry/io/wkt/read.hpp>
#include <boost/geometry/io/wkt/write.hpp>

struct equal_turn
{
    equal_turn(std::string const& s) : turn_ptr(&s) {}
    
    template <typename T>
    bool operator()(T const& t) const
    {
        BOOST_ASSERT(turn_ptr && turn_ptr->size() == 3);
        return bg::method_char(t.method) == (*turn_ptr)[0]
            && bg::operation_char(t.operations[0].operation) == (*turn_ptr)[1]
            && bg::operation_char(t.operations[1].operation) == (*turn_ptr)[2];
    }

    const std::string * turn_ptr;
};

template <typename Geometry1, typename Geometry2>
void check_geometry(
    Geometry1 const& g1,
    Geometry2 const& g2,
    std::string const& wkt1,
    std::string const& wkt2,
    std::vector<std::string> const& expected)
{
    typedef bg::detail::overlay::turn_info
        <
            typename bg::point_type<Geometry2>::type,
            typename bg::detail::get_turns::turn_operation_type<Geometry1, Geometry2>::type
        > turn_info;
    typedef bg::detail::overlay::assign_null_policy assign_policy_t;
    typedef bg::detail::get_turns::no_interrupt_policy interrupt_policy_t;

    std::vector<turn_info> turns;
    interrupt_policy_t interrupt_policy;
    
    // Don't switch the geometries
    typedef bg::detail::get_turns::get_turn_info_type<Geometry1, Geometry2, assign_policy_t> turn_policy_t;
    bg::dispatch::get_turns
        <
            typename bg::tag<Geometry1>::type, typename bg::tag<Geometry2>::type,
            Geometry1, Geometry2, false, false,
            turn_policy_t
        >::apply(0, g1, 1, g2, bg::detail::no_rescale_policy(), turns, interrupt_policy);

    bool ok = expected.size() == turns.size();

    BOOST_CHECK_MESSAGE(ok,
        "get_turns: " << wkt1 << " and " << wkt2
        << " -> Expected turns #: " << expected.size() << " detected turns #: " << turns.size());

    BOOST_FOREACH(std::string const& s, expected)
    {
        typename std::vector<turn_info>::iterator
            it = std::find_if(turns.begin(), turns.end(), equal_turn(s));

        if ( it != turns.end() )
            turns.erase(it);
        else
        {
            BOOST_CHECK_MESSAGE(false,
                "get_turns: " << wkt1 << " and " << wkt2
                << " -> Expected turn: " << s << " not found");
        }
    }
}

template <typename Geometry1, typename Geometry2>
void test_geometry(std::string const& wkt1, std::string const& wkt2,
                   std::vector<std::string> const& expected)
{
    Geometry1 geometry1;
    Geometry2 geometry2;
    bg::read_wkt(wkt1, geometry1);
    bg::read_wkt(wkt2, geometry2);
    check_geometry(geometry1, geometry2, wkt1, wkt2, expected);
}

template <typename G1, typename G2>
void test_geometry(std::string const& wkt1, std::string const& wkt2,
                   std::string const& ex0)
{
    std::vector<std::string> expected;
    expected.push_back(ex0);
    test_geometry<G1, G2>(wkt1, wkt2, expected);
}

template <typename G1, typename G2>
void test_geometry(std::string const& wkt1, std::string const& wkt2,
    std::string const& ex0, std::string const& ex1)
{
    std::vector<std::string> expected;
    expected.push_back(ex0);
    expected.push_back(ex1);
    test_geometry<G1, G2>(wkt1, wkt2, expected);
}

template <typename G1, typename G2>
void test_geometry(std::string const& wkt1, std::string const& wkt2,
    std::string const& ex0, std::string const& ex1, std::string const& ex2)
{
    std::vector<std::string> expected;
    expected.push_back(ex0);
    expected.push_back(ex1);
    expected.push_back(ex2);
    test_geometry<G1, G2>(wkt1, wkt2, expected);
}

struct expected_pusher
{
    expected_pusher & operator()(std::string const& ex)
    {
        vec.push_back(ex);
        return *this;
    }
    std::vector<std::string> vec;
};

expected_pusher expected(std::string const& ex)
{
    expected_pusher res;
    return res(ex);
}

#endif // BOOST_GEOMETRY_TEST_ALGORITHMS_OVERLAY_TEST_GET_TURNS_HPP
