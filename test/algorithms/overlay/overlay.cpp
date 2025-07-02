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
#define BOOST_GEOMETRY_DEBUG_IDENTIFIER
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

    template <typename Turns, typename Cluster, typename Connections>
    inline void visit_cluster_connections(bg::signed_size_type cluster_id,
            Turns const& turns, Cluster const& cluster, Connections const& connections)
    {
        using point_type = typename Turns::value_type::point_type;
        using ls_type = bg::model::linestring<point_type>;

        auto id_as_string = [](auto const& id)
        {
            std::stringstream out;
            out << id;
            return out.str();
        };

        if (cluster.turn_indices.empty())
        {
            return;
        }

        auto const& turn_point = turns[*cluster.turn_indices.begin()].point;

        for (auto const& item : connections)
        {
            auto const& key = item.key;
            auto const& connection = item.properties;
            ls_type const ls{turn_point, connection.point};
            m_writer.feature(ls);
            m_writer.add_property("type", "cluster");
            m_writer.add_property("cluster_id", cluster_id);
            m_writer.add_property("direction", std::string(key.connection ==
                    bg::detail::overlay::connection_type::incoming ? "in" : "out"));
            m_writer.add_property("position_code", connection.position_code);
            m_writer.add_property("rank", connection.rank);
            m_writer.add_property("count_left", connection.zone_count_left);
            m_writer.add_property("count_right", connection.zone_count_right);
            m_writer.add_property("seg_id", id_as_string(key.seg_id));
            m_writer.add_property("ring_id", id_as_string(bg::detail::overlay::ring_id_by_seg_id(key.seg_id)));
        }
    }

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
            auto const& op0 = turn.operations[0];
            auto const& op1 = turn.operations[1];

            auto label_component = [&]()
            {
                std::ostringstream out;
                auto const& c0 = op0.enriched.component_id;
                auto const& c1 = op1.enriched.component_id;
                if (c0 < 0 && c1 < 0) out << "-";
                else if (c0 == c1) out << c0;
                else if (c0 < 0) out << c1;
                else if (c1 < 0) out << c0;
                else out << c0 << " | " << c1;
                return out.str();
            };
            auto label_operation_ids = [&turn](int op_index)
            {
                std::ostringstream out;
                out << bg::operation_char(turn.operations[op_index].operation)
                    << ": " << turn.operations[op_index].seg_id
                    << " v:" << turn.operations[op_index].enriched.travels_to_vertex_index
                    << "|t:" << turn.operations[op_index].enriched.travels_to_ip_index;
                return out.str();
            };
            auto label_operations = [&]()
            {
                std::ostringstream out;
                out << bg::operation_char(op0.operation)
                    << bg::operation_char(op1.operation);
                return out.str();
            };
            auto label_travel = [&]()
            {
                std::ostringstream out;
                out << op0.enriched.travels_to_ip_index
                    << "|" << op1.enriched.travels_to_ip_index;
                return out.str();
            };

            m_writer.feature(turn.point);
            m_writer.add_property("index", index);
            m_writer.add_property("method", bg::method_char(turn.method));
            m_writer.add_property("travels_to", label_travel());
            m_writer.add_property("cluster_id", turn.cluster_id);
            m_writer.add_property("discarded", turn.discarded);
            m_writer.add_property("self_turn", bg::detail::overlay::is_self_turn<bg::overlay_union>(turn));
            m_writer.add_property("component", label_component());
            m_writer.add_property("operations", label_operations());

            m_writer.add_property("operation_0", label_operation_ids(0));
            m_writer.add_property("count_left_0", op0.enriched.count_left);
            m_writer.add_property("count_right_0", op0.enriched.count_right);
            m_writer.add_property("ahead_distance_0", op0.enriched.ahead_distance_of_side_change);
            m_writer.add_property("ahead_side_0", op0.enriched.ahead_side);

            m_writer.add_property("operation_1", label_operation_ids(1));
            m_writer.add_property("count_left_1", op1.enriched.count_left);
            m_writer.add_property("count_right_1", op1.enriched.count_right);
            m_writer.add_property("ahead_distance_1", op1.enriched.ahead_distance_of_side_change);
            m_writer.add_property("ahead_side_1", op1.enriched.ahead_side);

        }
    }

    boost::geometry::geojson_writer& m_writer;

};
#endif

template <typename Geometry, bg::overlay_type OverlayType, bool SymDiff = false>
void test_overlay(std::string const& caseid,
        std::string const& wkt1, std::string const& wkt2,
        double expected_area,
        std::size_t expected_clip_count,
        std::size_t expected_hole_count)
{
    std::cout << caseid << std::endl;
    Geometry g1;
    bg::read_wkt(wkt1, g1);

    Geometry g2;
    bg::read_wkt(wkt2, g2);

    bg::correct(g1);
    bg::correct(g2);

    if (! bg::is_valid(g1))
    {
        std::cerr << "WARNING: Invalid input 1: " << caseid << std::endl;
    }
    if (! bg::is_valid(g2))
    {
        std::cerr << "WARNING: Invalid input 2: " << caseid << std::endl;
    }

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

    if (SymDiff)
    {
        bg::sym_difference(g1, g2, result);
    }
    else
    {
        overlay::apply(g1, g2, std::back_inserter(result), strategy, visitor);
    }

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

// TEMP
#define TEST_DIFFERENCE_S(caseid, area, clips, holes) (test_overlay<multi_polygon, bg::overlay_difference, true>) \
    ( #caseid "_diff_s", caseid[0], caseid[1], area, clips, holes)

static std::string issue_893_multi[2] =
{
    "MULTIPOLYGON(((-9133.3885344331684 3976.3162451998137, -6748.2449169873034 -5735.0734557728138, 12359.886942916415 -1042.0645095456412, 5126.3084924076147 2226.9708710750697, -1604.5619839035633 573.85084904357439, -9133.3885344331684 3976.3162451998137)))",
    "MULTIPOLYGON(((-3228.4265340177531 1307.7159344890201, -4500.2645033380131 1882.4913860267370, -4294.7752070657516 1045.8178117890784, -3228.4265340177531 1307.7159344890201)))"
};

static std::string issue_1100_multi[2] =
{
    "MULTIPOLYGON(((1.5300545419548890819e-16 2101,1 2101,1 2100,1.1102230246251565404e-16 2100,1.5300545419548890819e-16 2101)))",
    "MULTIPOLYGON(((-0.19761611601674899941 2101,0 2100.6135231738085167,0 2100,-0.5 2100,-0.5 2101,-0.19761611601674899941 2101)))"
};

static std::string issue_1363_multi[2] =
{
    "MULTIPOLYGON(((2.0611606968426476882 0.61095000000000010409,2.046160696842648008 0.62595000000000000639,2.0311606968426478836 0.6409499999999999087,1.9486606968426476438 0.73094999999999987761,1.9261606968426476794 0.76094999999999990425,1.9336606968426472974 0.78344999999999986873,2.0161606968426477593 0.85844999999999993534,2.0236606968426480435 0.8584499999999997133,2.0461606968426475639 0.90344999999999986429,2.0911606968426479369 0.88844999999999973994,2.098660696842647333 0.8734499999999996156,2.1136606968426479014 0.86594999999999977547,2.1286606968426480258 0.85094999999999976215,2.1436606968426472619 0.83594999999999985985,2.143660696842647706 0.62594999999999989537,2.0836606968426476527 0.62594999999999989537,2.0611606968426476882 0.61095000000000010409)))",
    "MULTIPOLYGON(((2.0461606968426484521 0.90344999999999986429,2.001160696842647635 0.91095000000000003748,1.8511606968426477238 0.91094999999999992646,1.813660696842647635 0.91844999999999998863,1.813660696842647635 0.9409499999999999531,1.8211606968426479192 1.1059499999999999886,1.8286606968426479813 1.263449999999999962,1.9636606968426479902 1.263449999999999962,2.0461606968426484521 1.2559499999999998998,2.0536606968426478481 1.2409499999999999975,2.1286606968426480258 1.2409499999999999975,2.1286606968426480258 1.1059499999999999886,2.1211606968426477415 0.92594999999999982876,2.1136606968426479014 0.89594999999999980211,2.091160696842648381 0.88844999999999996199,2.0461606968426484521 0.90344999999999986429)))"
};

static std::string buffer_rt_g_multi[2] =
    {
        "MULTIPOLYGON(((2.0 8.0,2.0 9.0,2.0 10.0,3.0 10.0,4.0 10.0,6.4142135623730958 10.0,4.7071067811865479 8.2928932188134521,3.7071067811865475 7.2928932188134521,2.0 5.5857864376269051,2.0 8.0)))",
        "MULTIPOLYGON(((0.0 6.0,0.0 7.0,0.0 8.0,1.0 8.0,2.0 8.0,4.4142135623730958 8.0,2.7071067811865475 6.2928932188134521,1.7071067811865475 5.2928932188134521,-0.0 3.5857864376269042,0.0 6.0)))"
    };

static std::string ggl_list_20190307_matthieu_1_multi[2] =
    {
        "MULTIPOLYGON(((-1.00000010731 -0.713619134602,-1.00000012822 -0.493922219801,-0.598172925227 0.100631982002,-1.00000012886 -0.0624283708015,-1.00000011994 0.0862738908136,-0.440262107798 0.31341400405,-0.360828341246 0.292948255722,-0.357275641893 0.210997365241,-0.970143533681 -0.695818118925,-1.00000010731 -0.713619134602)))",
        "MULTIPOLYGON(((-0.999999965066 -0.493921978401,-0.909999987372 -0.360755621066,-0.909999996424 -0.91000000872,0.91000000872 -0.909999996424,0.909999996424 0.91000000872,-0.909999996424 0.91000000872,-0.909999911756 -0.0259065349961,-0.999999867625 -0.0624282647935,-1 1,1 1,1 -1,-1 -1,-0.999999965066 -0.493921978401)))"
    };

static std::string buffer_rt_a_multi[2] =
    {
        "MULTIPOLYGON(((1 7,1 8,1.0012 8.04907,1.00482 8.09802,1.01082 8.14673,1.01921 8.19509,1.02997 8.24298,1.04306 8.29028,1.05846 8.33689,1.07612 8.38268,1.09601 8.42756,1.11808 8.4714,1.14227 8.5141,1.16853 8.55557,1.19679 8.5957,1.22699 8.63439,1.25905 8.67156,1.29289 8.70711,1.32844 8.74095,1.36561 8.77301,1.4043 8.80321,1.44443 8.83147,1.4859 8.85773,1.5286 8.88192,1.57244 8.90399,1.61732 8.92388,1.66311 8.94154,1.70972 8.95694,1.75702 8.97003,1.80491 8.98079,1.85327 8.98918,1.90198 8.99518,1.95093 8.9988,2 9,3 9,3.04907 8.9988,3.09802 8.99518,3.14673 8.98918,3.19509 8.98079,3.24298 8.97003,3.29028 8.95694,3.33689 8.94154,3.38268 8.92388,3.42756 8.90399,3.4714 8.88192,3.5141 8.85773,3.55557 8.83147,3.5957 8.80321,3.63439 8.77301,3.67156 8.74095,3.70711 8.70711,3.74095 8.67156,3.77301 8.63439,3.80321 8.5957,3.83147 8.55557,3.85773 8.5141,3.88192 8.4714,3.90399 8.42756,3.92388 8.38268,3.94154 8.33689,3.95694 8.29028,3.97003 8.24298,3.98079 8.19509,3.98918 8.14673,3.99518 8.09802,3.9988 8.04907,4 8,4 7,3.9988 6.95093,3.99518 6.90198,3.98918 6.85327,3.98079 6.80491,3.97003 6.75702,3.95694 6.70972,3.94154 6.66311,3.92388 6.61732,3.90399 6.57244,3.88192 6.5286,3.85773 6.4859,3.83147 6.44443,3.80321 6.4043,3.77301 6.36561,3.74095 6.32844,3.70711 6.29289,3.67156 6.25905,3.63439 6.22699,3.5957 6.19679,3.55557 6.16853,3.5141 6.14227,3.4714 6.11808,3.42756 6.09601,3.38268 6.07612,3.33689 6.05846,3.29028 6.04306,3.24298 6.02997,3.19509 6.01921,3.14673 6.01082,3.09802 6.00482,3.04907 6.0012,3 6,2 6,1.95093 6.0012,1.90198 6.00482,1.85327 6.01082,1.80491 6.01921,1.75702 6.02997,1.70972 6.04306,1.66311 6.05846,1.61732 6.07612,1.57244 6.09601,1.5286 6.11808,1.4859 6.14227,1.44443 6.16853,1.4043 6.19679,1.36561 6.22699,1.32844 6.25905,1.29289 6.29289,1.25905 6.32844,1.22699 6.36561,1.19679 6.4043,1.16853 6.44443,1.14227 6.4859,1.11808 6.5286,1.09601 6.57244,1.07612 6.61732,1.05846 6.66311,1.04306 6.70972,1.02997 6.75702,1.01921 6.80491,1.01082 6.85327,1.00482 6.90198,1.0012 6.95093,1 7)))",
        "MULTIPOLYGON(((3 6,4 6,4.04907 5.9988,4.09802 5.99518,4.14673 5.98918,4.19509 5.98079,4.24298 5.97003,4.29028 5.95694,4.33689 5.94154,4.38268 5.92388,4.42756 5.90399,4.4714 5.88192,4.5141 5.85773,4.55557 5.83147,4.5957 5.80321,4.63439 5.77301,4.67156 5.74095,4.70711 5.70711,4.74095 5.67156,4.77301 5.63439,4.80321 5.5957,4.83147 5.55557,4.85773 5.5141,4.88192 5.4714,4.90399 5.42756,4.92388 5.38268,4.94154 5.33689,4.95694 5.29028,4.97003 5.24298,4.98079 5.19509,4.98918 5.14673,4.99518 5.09802,4.9988 5.04907,5 5,5 4,4.9988 3.95093,4.99518 3.90198,4.98918 3.85327,4.98079 3.80491,4.97003 3.75702,4.95694 3.70972,4.94154 3.66311,4.92388 3.61732,4.90399 3.57244,4.88192 3.5286,4.85773 3.4859,4.83147 3.44443,4.80321 3.4043,4.77301 3.36561,4.74095 3.32844,4.70711 3.29289,4.67156 3.25905,4.63439 3.22699,4.5957 3.19679,4.55557 3.16853,4.5141 3.14227,4.4714 3.11808,4.42756 3.09601,4.38268 3.07612,4.33689 3.05846,4.29028 3.04306,4.24298 3.02997,4.19509 3.01921,4.14673 3.01082,4.09802 3.00482,4.04907 3.0012,4 3,3 3,3 3,2 3,1.95093 3.0012,1.90198 3.00482,1.85327 3.01082,1.80491 3.01921,1.75702 3.02997,1.70972 3.04306,1.66311 3.05846,1.61732 3.07612,1.57244 3.09601,1.5286 3.11808,1.4859 3.14227,1.44443 3.16853,1.4043 3.19679,1.36561 3.22699,1.32844 3.25905,1.29289 3.29289,1.25905 3.32844,1.22699 3.36561,1.19679 3.4043,1.16853 3.44443,1.14227 3.4859,1.11808 3.5286,1.09601 3.57244,1.07612 3.61732,1.05846 3.66311,1.04306 3.70972,1.02997 3.75702,1.01921 3.80491,1.01082 3.85327,1.00482 3.90198,1.0012 3.95093,1 4,1 5,1.0012 5.04907,1.00482 5.09802,1.01082 5.14673,1.01921 5.19509,1.02997 5.24298,1.04306 5.29028,1.05846 5.33689,1.07612 5.38268,1.09601 5.42756,1.11808 5.4714,1.14227 5.5141,1.16853 5.55557,1.19679 5.5957,1.22699 5.63439,1.25905 5.67156,1.29289 5.70711,1.32844 5.74095,1.36561 5.77301,1.4043 5.80321,1.44443 5.83147,1.4859 5.85773,1.5286 5.88192,1.57244 5.90399,1.61732 5.92388,1.66311 5.94154,1.70972 5.95694,1.75702 5.97003,1.80491 5.98079,1.85327 5.98918,1.90198 5.99518,1.95093 5.9988,2 6,3 6,3 6,3 6,3 6,3 6,3 6,3 6,3 6,3 6,3 6,3 6,3 6,3 6,3 6,3 6,3 6,3 6,3 6,3 6,3 6,3 6,3 6,3 6,3 6,3 6,3 6,3 6,3 6,3 6,3 6,3 6,3 6,3 6)))"
    };

// Two colocations of interior/exterior ring
static std::string case_80s_multi[2] = {
        "MULTIPOLYGON(((0 5,5 10,10 5,5 0,0 5),(10 5,4 6,5 4,10 5)))",
        "MULTIPOLYGON(((10 0,10 10,20 10,20 0,10 0),(10 5,15 3,18 8,10 5)))"
    };



template <typename T, bool Clockwise>
void test_all()
{
    using point_type = bg::model::point<T, 2, bg::cs::cartesian>;
    using polygon = bg::model::polygon<point_type, Clockwise>;
    using multi_polygon = bg::model::multi_polygon<polygon>;

    // TEST_DIFFERENCE_A(ggl_list_20190307_matthieu_1_multi, 0.18461532, 2, 0);
    // TEST_DIFFERENCE_B(ggl_list_20190307_matthieu_1_multi, 0.617978, 2, 0);
    // TEST_DIFFERENCE_S(ggl_list_20190307_matthieu_1_multi, 0.18461532 + 0.617978, 4, 0);
    // TEST_INTERSECTION(case_recursive_boxes_54, 10.0, 3, 1); // works (using select_edge)

    // TEST_INTERSECTION(case_recursive_boxes_95, 36.25, 30, 0);
    // TEST_INTERSECTION(case_recursive_boxes_99, 99, 7, 0);

    // TEST_INTERSECTION(case_bitset_3, 16.0, 1, 0);
    // TEST_INTERSECTION(case_149_multi, 48.0, 2, 2); // instruction
// 

    // TEST_DIFFERENCE_B(issue_893_multi, 97213916.0, 1, 1); // needs is_traverse


    TEST_UNION(case_134_multi, 66.0, 1, 2);
    TEST_UNION(case_76_multi, 8.0, 5, 0);
    TEST_UNION(case_150_multi, 21.0, 2, 0); // uu
    TEST_UNION(case_151_multi, 14.0, 4, 0); // 2 uu
    TEST_UNION_WITH(buffer_rt_a_multi, 1, 0, 19.2806668, 1, 0);
    TEST_UNION(case_80s_multi, 129.0, 2, 2);
    TEST_INTERSECTION(case_108_multi, 7.5, 7, 0);
    TEST_UNION(case_recursive_boxes_12, 6.0, 6, 0);
    TEST_UNION(case_149_multi, 115.0, 2, 0); // instruction
    TEST_UNION(ggl_list_20140212_sybren, 0.002471626, 2, 0);
    TEST_UNION_WITH(issue_1100_multi, 1, 0, 1.46181, 1, 0); // fixed by ux/uu blocking
    TEST_UNION(case_recursive_boxes_32, 5.75, 2, 0);
    TEST_INTERSECTION(case_148_multi, 4.0, 4, 1);
    TEST_INTERSECTION_WITH(case_recursive_boxes_99, 2, 3, 8.0, 5, 1);
    TEST_INTERSECTION(case_recursive_boxes_79, 9.0, 5, 1); // fixed (fixing self-turns in clusters)
    TEST_INTERSECTION(case_recursive_boxes_98, 4, 7, 0); // fixed by skip_count
    TEST_INTERSECTION(case_recursive_boxes_96, 37.25, 34, 0); // fixed by handling of cc / via target
    TEST_INTERSECTION_WITH(case_recursive_boxes_91, 0, 1, 27.5, 29, 0); // requires specific selection of operation_continue
    TEST_INTERSECTION(case_recursive_boxes_92, 30.5, 32, 0);
    TEST_INTERSECTION(case_recursive_boxes_66, 16.0, 4, 1);
    TEST_INTERSECTION(case_recursive_boxes_97, 30.25, 25, 0); // fixed by handling of cc / via target
    TEST_INTERSECTION_WITH(case_recursive_boxes_92, 1, 0, 30.5, 32, 0);
    TEST_INTERSECTION(case_recursive_boxes_83, 10.25, 5, 0); // Fixed (using count_right)
    TEST_UNION(case_recursive_boxes_56,  7.75, 5, 1);
    TEST_DIFFERENCE_S(case_multi_simplex, 5.58 + 2.58, 5 + 4, 0);
    TEST_UNION(case_101_multi, 22.25, 1, 3); // Convenient start case for union with holes
    TEST_UNION(buffer_rt_g_multi, 16.571, 1, 0); // FIXED by remaining + union
    TEST_UNION(issue_1100_multi, 1.46181, 1, 0);
    TEST_INTERSECTION_WITH(case_recursive_boxes_91, 6, 7, 8.75, 10, 0); // requires specific selection of operation_continue
    TEST_INTERSECTION_WITH(case_recursive_boxes_91, 2, 3, 27.5, 29, 0);
    TEST_INTERSECTION_WITH(case_recursive_boxes_91, 4, 5, 4.0, 5, 0);
    TEST_UNION(case_multi_simplex, 14.58, 1, 0);
    TEST_UNION(case_recursive_boxes_3, 56.5, 17, 6); // still complex
    TEST_UNION(case_recursive_boxes_4, 96.75, 1, 2);

    // Contains 4 clusters, one of which having 4 turns
    TEST_UNION(case_recursive_boxes_7, 7.0, 2, 0);

    TEST_UNION(case_recursive_boxes_13, 10.25, 3, 0);
    TEST_DIFFERENCE_S(case_multi_simplex, 5.58 + 2.58, 5 + 4, 0);
    TEST_DIFFERENCE_A(case_recursive_boxes_18, 1.0, 2, 0);
    TEST_DIFFERENCE_B(case_recursive_boxes_18, 1.5, 1, 0);
    TEST_INTERSECTION(case_101_multi, 4.75, 4, 0);
    TEST_INTERSECTION(case_125_multi, 2.1, 3, 1); // Fixed with correct visit info
    TEST_INTERSECTION(case_124_multi, 2.0625, 2, 1);
    TEST_INTERSECTION(case_recursive_boxes_4, 67.0, 13, 8);
    TEST_INTERSECTION(case_128_multi, 75.5, 2, 3); // FIXED

    TEST_INTERSECTION(case_recursive_boxes_79, 9.0, 5, 1); // fixed (fixing self-turns in clusters)
    TEST_INTERSECTION(case_recursive_boxes_77, 3.5, 5, 0); // fixed (using count_right)
    TEST_INTERSECTION(case_recursive_boxes_54, 10.0, 3, 1); // works (using select_edge)

    TEST_INTERSECTION(case_recursive_boxes_1, 47.0, 10, 0); // works (allow on same segment)
    TEST_INTERSECTION(case_recursive_boxes_89, 1.5, 2, 0);
    TEST_INTERSECTION(case_recursive_boxes_90, 1.0, 2, 0);
    TEST_INTERSECTION(case_recursive_boxes_35, 20.0, 2, 6);
    TEST_INTERSECTION(case_recursive_boxes_42, 95.0, 1, 4);
    TEST_INTERSECTION(case_recursive_boxes_48, 1.0, 1, 0);

    TEST_INTERSECTION(case_130_multi, 39.0, 2, 2);

    TEST_INTERSECTION(case_64_multi, 1.0, 1, 0);
    TEST_INTERSECTION(case_78_multi, 22.0, 1, 2);
    TEST_INTERSECTION(case_72_multi, 2.85, 3, 0);
    TEST_INTERSECTION_WITH(case_72_multi, 1, 2, 6.15, 3, 1);

    TEST_INTERSECTION(case_multi_simplex, 6.42, 2, 0);
    TEST_INTERSECTION(case_multi_diagonal, 650.0, 1, 0);
    TEST_INTERSECTION(case_multi_2, 5.9, 3, 0);
    TEST_INTERSECTION(case_multi_3, 20.0, 1, 0);

    return;




    TEST_DIFFERENCE_A(case_multi_simplex, 5.58, 5, 0);
    TEST_DIFFERENCE_B(case_multi_simplex, 2.58, 4, 0);

    // Contains 5 clusters, needing immediate selection of next turn
    TEST_UNION_WITH(case_58_multi, 0, 3, 19.8333333, 2, 0);

    // Contains many clusters, needing to exclude u/u turns
    TEST_UNION(case_recursive_boxes_3, 56.5, 17, 6);


    // Contains 5 clusters, needing immediate selection of next turn
    TEST_UNION(case_89_multi, 6.0, 1, 0);

    // Needs ux/next_turn_index==-1 to be filtered out
    TEST_INTERSECTION(case_77_multi, 9.0, 5, 0);

    TEST_INTERSECTION(case_recursive_boxes_11, 1.0, 2, 0);
    TEST_INTERSECTION(case_recursive_boxes_30, 6.0, 4, 0);

    TEST_INTERSECTION_WITH(case_58_multi, 2, 6, 13.25, 1, 1);
    TEST_INTERSECTION_WITH(case_72_multi, 1, 2, 6.15, 3, 1);

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
    // test_integer<int, true>();
    test_all<double, true>();
    return 0;
 }
