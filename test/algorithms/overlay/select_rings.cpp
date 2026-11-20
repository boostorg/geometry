// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.
//
// This file was modified by Oracle on 2017-2021.
// Modifications copyright (c) 2017-2021 Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <geometry_test_common.hpp>

#include <algorithm>
#include <cstdint>
#include <initializer_list>

#include <algorithms/test_overlay.hpp>

#include <boost/geometry/algorithms/detail/overlay/select_rings.hpp>
#include <boost/geometry/algorithms/detail/overlay/assign_parents.hpp>
#include <boost/geometry/algorithms/correct.hpp>

#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>

#include <boost/geometry/io/wkt/read.hpp>

template
<
    typename Geometry1,
    typename Geometry2,
    bg::overlay_type OverlayType,
    typename RingId
>
void test_geometry(std::string const& wkt1, std::string const& wkt2,
                   std::initializer_list<RingId> const& expected_ids)
{
    typedef bg::detail::overlay::ring_properties
        <
            typename bg::point_type<Geometry1>::type,
            double
        > properties;

    Geometry1 geometry1;
    Geometry2 geometry2;

    bg::read_wkt(wkt1, geometry1);
    bg::read_wkt(wkt2, geometry2);

    typedef std::map<bg::ring_identifier, properties> map_type;
    map_type selected;
    std::map<bg::ring_identifier, bg::detail::overlay::ring_turn_info> empty;

    typedef typename bg::strategies::relate::services::default_strategy
        <
            Geometry1, Geometry2
        >::type strategy_type;

    bg::detail::overlay::select_rings<OverlayType>(geometry1, geometry2, empty, selected, strategy_type());

    BOOST_CHECK_EQUAL(selected.size(), expected_ids.size());

    if (selected.size() <= expected_ids.size())
    {
        auto eit = expected_ids.begin();
        for (auto it = selected.begin(); it != selected.end(); ++it, ++eit)
        {
            bg::ring_identifier const ring_id = it->first;
            BOOST_CHECK_EQUAL(ring_id.source_index, eit->source_index);
            BOOST_CHECK_EQUAL(ring_id.multi_index, eit->multi_index);
            BOOST_CHECK_EQUAL(ring_id.ring_index, eit->ring_index);
        }
    }
}




template <typename P>
void test_all()
{
    // Point in correct clockwise ring -> should return true
    typedef bg::ring_identifier rid;

    test_geometry<bg::model::polygon<P>, bg::model::polygon<P>, bg::overlay_union>(
        winded[0], winded[1],
            { rid(0,-1,-1),
              rid(0,-1, 0),
              rid(0,-1, 1),
              rid(0,-1, 3),
              rid(1,-1, 1),
              rid(1,-1, 2) });

    test_geometry<bg::model::polygon<P>, bg::model::polygon<P>, bg::overlay_intersection>(
            winded[0], winded[1],
                { rid(0,-1, 2),
                  rid(1,-1,-1),
                  rid(1,-1, 0),
                  rid(1,-1, 3), });
}




template <typename Point, bool Clockwise, bool Closed>
void test_boundary_vertices()
{
    using ring = bg::model::ring<Point, Clockwise, Closed>;
    using polygon = bg::model::polygon<Point, Clockwise, Closed>;
    using strategy = typename bg::strategies::relate::services::default_strategy
        <ring, polygon>::type;

    ring r;
    polygon containing, outside, coincident;
    bg::read_wkt("POLYGON((2 2,2 1,1 1,2 2))", r);
    bg::read_wkt("POLYGON((2 2,1 2,1 1,0 1,0 0,2 0,2 1,3 1,2 2))", containing);
    bg::read_wkt("POLYGON((2 0,4 0,4 2,3 3,1 3,0 2,0 1,1 1,1 2,2 2,3 2,3 1,2 1,2 0))", outside);
    bg::read_wkt("POLYGON((2 2,2 1,1 1,2 2))", coincident);
    bg::correct(r);
    bg::correct(containing);
    bg::correct(outside);
    bg::correct(coincident);
    for (int reversed = 0; reversed != 2; ++reversed)
    {
        // Both an exterior ring and a hole have to be classified by their edges.
        BOOST_CHECK_EQUAL(bg::detail::overlay::ring_in_geometry(
            r.front(), r, containing, strategy()), 1);
        BOOST_CHECK_EQUAL(bg::detail::overlay::ring_in_geometry(
            r.front(), r, outside, strategy()), 0);
        BOOST_CHECK_EQUAL(bg::detail::overlay::ring_in_geometry(
            r.front(), r, coincident, strategy()), 0);
        std::reverse(r.begin(), r.end());
    }
}

int test_main( int , char* [] )
{
    test_all<bg::model::d2::point_xy<double> >();
    test_boundary_vertices<bg::model::d2::point_xy<std::int_least64_t>, true, true>();
    test_boundary_vertices<bg::model::d2::point_xy<std::int_least64_t>, true, false>();
    test_boundary_vertices<bg::model::d2::point_xy<std::int_least64_t>, false, true>();
    test_boundary_vertices<bg::model::d2::point_xy<std::int_least64_t>, false, false>();
    test_boundary_vertices<bg::model::d2::point_xy<double>, true, true>();

    return 0;
}
