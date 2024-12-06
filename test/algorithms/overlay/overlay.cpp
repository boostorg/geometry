// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2015-2024 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2017-2024.
// Modifications copyright (c) 2017-2024, Oracle and/or its affiliates.
// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#if defined(TEST_WITH_GEOJSON)
#define BOOST_GEOMETRY_DEBUG_SEGMENT_IDENTIFIER
#endif

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>

#include <geometry_test_common.hpp>
#include <algorithms/check_validity.hpp>

#include <boost/geometry/algorithms/correct.hpp>
#include <boost/geometry/algorithms/detail/overlay/debug_turn_info.hpp>
#include <boost/geometry/algorithms/detail/overlay/overlay.hpp>
#include <boost/geometry/algorithms/sym_difference.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/io/wkt/wkt.hpp>
#include <boost/geometry/views/enumerate_view.hpp>

#if defined(TEST_WITH_GEOJSON)
#include <boost/geometry/extensions/gis/io/geojson/geojson_writer.hpp>
#endif

#include "multi_overlay_cases.hpp"


#if defined(TEST_WITH_GEOJSON)
struct geojson_visitor : public boost::geometry::detail::overlay::overlay_null_visitor
{
    geojson_visitor(boost::geometry::geojson_writer& writer)
        : m_writer(writer)
    {}

    template <typename Turns>
    void visit_turns(int phase, Turns const& turns)
    {
        if (phase != 3)
        {
            return;
        }

        for (auto const& enumerated : boost::geometry::util::enumerate(turns))
        {
            auto index = enumerated.index;
            auto const& turn = enumerated.value;
            auto label_enriched = [&turn](int i)
            {
                auto const& op = turn.operations[i].enriched;
                std::ostringstream out;
                out //<< " l:" << op.count_left << " r:" << op.count_right
                    //<< " rank:" << op.rank
                    // << " z:" << op.zone
                    << " region:" << op.region_id
                    << (op.isolated ? " ISOLATED" : "");
                return out.str();
            };
            auto label_operation_ids = [&turn](int op_index)
            {
                std::ostringstream out;
                out << bg::operation_char(turn.operations[op_index].operation)
                    << ": " << turn.operations[op_index].seg_id
                    << " " << turn.operations[op_index].enriched.next_ip_index
                    << "|" << turn.operations[op_index].enriched.travels_to_ip_index;
                return out.str();
            };
            auto label_operations = [&turn]()
            {
                std::ostringstream out;
                out << bg::operation_char(turn.operations[0].operation)
                    << bg::operation_char(turn.operations[1].operation);
                return out.str();
            };
            auto label_travel = [&turn]()
            {
                std::ostringstream out;
                out << turn.operations[0].enriched.travels_to_ip_index
                    << "|" << turn.operations[1].enriched.travels_to_ip_index;
                return out.str();
            };

            m_writer.feature(turn.point);
            m_writer.add_property("index", index);
            m_writer.add_property("method", bg::method_char(turn.method));
            m_writer.add_property("operations", label_operations());
            m_writer.add_property("travels_to", label_travel());
            m_writer.add_property("cluster_id", turn.cluster_id);
            m_writer.add_property("discarded", turn.discarded);
            m_writer.add_property("has_colocated_both", turn.has_colocated_both);
            m_writer.add_property("self_turn", bg::detail::overlay::is_self_turn<bg::overlay_union>(turn));
            m_writer.add_property("operation_0", label_operation_ids(0));
            m_writer.add_property("operation_1", label_operation_ids(1));
            m_writer.add_property("enriched_0", label_enriched(0));
            m_writer.add_property("enriched_1", label_enriched(1));
        }
    }

    boost::geometry::geojson_writer& m_writer;

};
#endif

template <typename Geometry, bg::overlay_type OverlayType>
void test_overlay(std::string const& caseid,
        std::string const& wkt1, std::string const& wkt2,
        double expected_area,
        std::size_t expected_clip_count,
        std::size_t expected_hole_count)
{
    Geometry g1;
    bg::read_wkt(wkt1, g1);

    Geometry g2;
    bg::read_wkt(wkt2, g2);

    bg::correct(g1);
    bg::correct(g2);

#if defined(TEST_WITH_GEOJSON)
    std::ostringstream filename;
    // For QGis, it is usually convenient to always write to the same geojson file.
    filename << "/tmp/"
        // << caseid << "_"
        << "overlay.geojson";
    std::ofstream geojson_file(filename.str().c_str());

    boost::geometry::geojson_writer writer(geojson_file);
    writer.feature(g1);
    writer.add_property("type", "input1");
    writer.feature(g2);
    writer.add_property("type", "input2");
#endif


    using geometry_out = typename boost::range_value<Geometry>::type ;
    using overlay = bg::detail::overlay::overlay
        <
            Geometry, Geometry,
            bg::detail::overlay::do_reverse<bg::point_order<Geometry>::value>::value,
            OverlayType == bg::overlay_difference
            ? ! bg::detail::overlay::do_reverse<bg::point_order<Geometry>::value>::value
            : bg::detail::overlay::do_reverse<bg::point_order<Geometry>::value>::value,
            bg::detail::overlay::do_reverse<bg::point_order<Geometry>::value>::value,
            geometry_out,
            OverlayType
        >;

    using strategy_type = typename bg::strategies::relate::services::default_strategy
        <
            Geometry, Geometry
        >::type;

    strategy_type strategy;

#if defined(TEST_WITH_GEOJSON)
    geojson_visitor visitor(writer);
#else
    bg::detail::overlay::overlay_null_visitor visitor;
#endif

    Geometry result;
    overlay::apply(g1, g2, std::back_inserter(result), strategy, visitor);

    std::string message;
    bool const valid = check_validity<Geometry>::apply(result, caseid, g1, g2, message);
    BOOST_CHECK_MESSAGE(valid,
        "overlay: " << caseid << " not valid: " << message
        << " type: " << (type_for_assert_message<Geometry, Geometry>()));

    BOOST_CHECK_CLOSE(bg::area(result), expected_area, 0.001);
    BOOST_CHECK_MESSAGE((bg::num_interior_rings(result) == expected_hole_count),
                        caseid
                        << " hole count: detected: " << bg::num_interior_rings(result)
                        << " expected: "  << expected_hole_count);
    BOOST_CHECK_MESSAGE((result.size() == expected_clip_count),
                        caseid
                        << " clip count: detected: " << result.size()
                        << " expected: "  << expected_clip_count);

#if defined(TEST_WITH_GEOJSON)
    std::size_t result_index = 0;
    for (auto const& p : result)
    {
        writer.feature(p);
        writer.add_property("type", "result");
        writer.add_property("index", result_index++);
    }

    for (auto const& p : result)
    {
        for (const auto& ring : bg::interior_rings(p))
        {
            writer.feature(ring);
            writer.add_property("type", "hole");
            writer.add_property("index", result_index++);
        }
    }
#endif
}

#define TEST_INTERSECTION(caseid, area, clips, holes) (test_overlay<multi_polygon, bg::overlay_intersection>) \
    ( #caseid "_int", caseid[0], caseid[1], area, clips, holes)
#define TEST_UNION(caseid, area, clips, holes) (test_overlay<multi_polygon, bg::overlay_union>) \
    ( #caseid "_union", caseid[0], caseid[1], area, clips, holes)
#define TEST_DIFFERENCE_A(caseid, area, clips, holes) (test_overlay<multi_polygon, bg::overlay_difference>) \
    ( #caseid "_diff_a", caseid[0], caseid[1], area, clips, holes)
#define TEST_DIFFERENCE_B(caseid, area, clips, holes) (test_overlay<multi_polygon, bg::overlay_difference>) \
    ( #caseid "_diff_b", caseid[1], caseid[0], area, clips, holes)

#define TEST_INTERSECTION_WITH(caseid, index1, index2, area, clips, holes) (test_overlay<multi_polygon, bg::overlay_intersection>) \
    ( #caseid "_int_" #index1 "_" #index2, caseid[index1], caseid[index2], area, clips, holes)
#define TEST_UNION_WITH(caseid, index1, index2, area, clips, holes) (test_overlay<multi_polygon, bg::overlay_union>) \
    ( #caseid "_union" #index1 "_" #index2, caseid[index1], caseid[index2], area, clips, holes)

template <typename T, bool Clockwise>
void test_all()
{
    using point_type = bg::model::point<T, 2, bg::cs::cartesian>;
    using polygon = bg::model::polygon<point_type, Clockwise>;
    using multi_polygon = bg::model::multi_polygon<polygon>;

    TEST_UNION(case_multi_simplex, 14.58, 1, 0);
    TEST_INTERSECTION(case_multi_simplex, 6.42, 2, 0);

    TEST_DIFFERENCE_A(case_multi_simplex, 5.58, 5, 0);
    TEST_DIFFERENCE_B(case_multi_simplex, 2.58, 4, 0);

    // Contains 5 clusters, needing immediate selection of next turn
    TEST_UNION_WITH(case_58_multi, 0, 3, 19.8333333, 2, 0);

    // Contains many clusters, needing to exclude u/u turns
    TEST_UNION(case_recursive_boxes_3, 56.5, 17, 6);

    // Contains 4 clusters, one of which having 4 turns
    TEST_UNION(case_recursive_boxes_7, 7.0, 2, 0);

    // Contains 5 clusters, needing immediate selection of next turn
    TEST_UNION(case_89_multi, 6.0, 1, 0);

    // Needs ux/next_turn_index==-1 to be filtered out
    TEST_INTERSECTION(case_77_multi, 9.0, 5, 0);
    TEST_UNION(case_101_multi, 22.25, 1, 3);
    TEST_INTERSECTION(case_101_multi, 4.75, 4, 0);

    TEST_INTERSECTION(case_recursive_boxes_11, 1.0, 2, 0);
    TEST_INTERSECTION(case_recursive_boxes_30, 6.0, 4, 0);

    TEST_UNION(case_recursive_boxes_4, 96.75, 1, 2);
    TEST_INTERSECTION_WITH(case_58_multi, 2, 6, 13.25, 1, 1);
    TEST_INTERSECTION_WITH(case_72_multi, 1, 2, 6.15, 3, 1);
    TEST_UNION(case_recursive_boxes_12, 6.0, 6, 0);
    TEST_UNION(case_recursive_boxes_13, 10.25, 3, 0);

    TEST_INTERSECTION(issue_930, 8.3333333, 1, 0);
}

template <typename T, bool Clockwise>
void test_integer()
{
    using point_type = bg::model::point<T, 2, bg::cs::cartesian>;
    using polygon = bg::model::polygon<point_type, Clockwise>;
    using multi_polygon = bg::model::multi_polygon<polygon>;

    TEST_INTERSECTION(issue_930, 10, 1, 0);
}

int test_main(int, char* [])
{
    test_integer<int, true>();
    test_all<double, true>();
    return 0;
 }
