// Boost.Geometry
//
// Copyright (c) 2025 Barend Gehrels, Amsterdam, the Netherlands.

// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// Official repository: https://github.com/boostorg/geometry
// Documentation: http://www.boost.org/libs/geometry

#ifndef BOOST_GEOMETRY_TEST_DISSOLVE_GEOJSON_VISITOR_HPP
#define BOOST_GEOMETRY_TEST_DISSOLVE_GEOJSON_VISITOR_HPP

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
            auto label_preferences = [&]()
            {
                std::ostringstream out;
                out << op0.preference_index
                    << "|" << op1.preference_index;
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
            m_writer.add_property("x", bg::get<0>(turn.point));
            m_writer.add_property("y", bg::get<1>(turn.point));
            m_writer.add_property("index", index);
            m_writer.add_property("method", bg::method_char(turn.method));
            m_writer.add_property("travels_to", label_travel());
            m_writer.add_property("cluster_id", turn.cluster_id);
            m_writer.add_property("discarded", turn.discarded);
            m_writer.add_property("self_turn", bg::detail::overlay::is_self_turn<bg::overlay_union>(turn));
            m_writer.add_property("component", label_component());
            m_writer.add_property("operations", label_operations());
            m_writer.add_property("preferences", label_preferences());

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
