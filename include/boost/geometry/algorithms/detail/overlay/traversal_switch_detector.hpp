// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2015-2016 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2023 Adam Wulkiewicz, Lodz, Poland.

// This file was modified by Oracle on 2018-2024.
// Modifications copyright (c) 2018-2024 Oracle and/or its affiliates.
// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_TRAVERSAL_SWITCH_DETECTOR_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_TRAVERSAL_SWITCH_DETECTOR_HPP

#include <boost/range/value_type.hpp>

#include <boost/geometry/algorithms/detail/ring_identifier.hpp>
#include <boost/geometry/algorithms/detail/overlay/copy_segments.hpp>
#include <boost/geometry/algorithms/detail/overlay/cluster_info.hpp>
#include <boost/geometry/algorithms/detail/overlay/is_self_turn.hpp>
#include <boost/geometry/algorithms/detail/overlay/turn_info.hpp>

#if defined(BOOST_GEOMETRY_DEBUG_TRAVERSAL_SWITCH_DETECTOR)
#include <boost/geometry/core/access.hpp>
#endif

#include <boost/geometry/util/constexpr.hpp>

#include <cstddef>
#include <map>

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace overlay
{

// The switch detector, the first phase in traversal, inspects UU and II turns.
// Suppose you have these two polygons in a union. There is one UU turn.
// +-------+
// |       |
// |   A   |
// |       |
// +-------U---------+       U = UU turn
//         |         |
//         |    B    |
//         |         |
//         +---------+
// It first assigns region ids, A gets id 1 and B gets id 2.
// Because of that, it should NOT switch sources in traversal at U.
// So coming from upper left, it follows A, and also at U it keeps following A.
// The result is two rings. (See for example testcase "case_31" or others.)
//
// But suppose you have two larger input polygons, partially overlapping:
// +-----------------+
// |                 |
// |   A   +-----T---C       I = interior in output
// |       |  I  | O |       O = overlap A & B (included in output)
// +-------U-----T---C       U = UU turn
//         |         |       T = normal turn (u/i)
//         |    B    |       C = collinear turn (c/c)
//         |         |
//         +---------+
// Rings A and B will be connected (by inspecting turn information)
// and there will be one region 1.
// Because of that, it will switch sources in traversal at U.
// So coming from lower right, it follows B but at U it will switch to A.
// Also for the generated interior ring, coming from the top via A it will at U
// switch to B and go to the right, generating I. (See for example "case_91")
// Switching using region_id is only relevant for UU or II turns.
// In all T turns it will follow "u" for union or "i" for intersection,
// and in C turns it will follow either direction (they are the same).
template
<
    operation_type TargetOperation,
    typename Geometry1,
    typename Geometry2,
    typename Turns,
    typename Clusters,
    typename Visitor
>
struct traversal_switch_detector
{
    using turn_type = typename boost::range_value<Turns>::type;
    using set_type= std::set<signed_size_type>;

    // Per ring, first turns are collected (in turn_indices), and later
    // a region_id is assigned
    struct merged_ring_properties
    {
        signed_size_type region_id = -1;
        set_type turn_indices;
    };

    // Per region, a set of properties is maintained, including its connections
    // to other regions
    struct region_properties
    {
        signed_size_type region_id = -1;
    };

    // Maps ring -> properties
    using merge_map = std::map<ring_identifier, merged_ring_properties>;

    // Maps region_id -> properties
    using region_connection_map = std::map<signed_size_type, region_properties>;

    inline traversal_switch_detector(Geometry1 const& geometry1,
            Geometry2 const& geometry2,
            Turns& turns, Clusters const& clusters,
            Visitor& visitor)
        : m_geometry1(geometry1)
        , m_geometry2(geometry2)
        , m_turns(turns)
        , m_clusters(clusters)
        , m_visitor(visitor)
    {
    }

    void assign_region_ids_to_enriched()
    {
        for (auto const& key_val : m_turns_per_ring)
        {
            ring_identifier const& ring_id = key_val.first;
            merged_ring_properties const& properties = key_val.second;

            for (auto turn_index : properties.turn_indices)
            {
                turn_type& turn = m_turns[turn_index];

                if (! acceptable(turn))
                {
                    // No assignment necessary
                    continue;
                }

                for (auto& op : turn.operations)
                {
                    if (ring_id_by_seg_id(op.seg_id) == ring_id)
                    {
                        op.enriched.region_id = properties.region_id;
                    }
                }
            }
        }
    }

    void assign_connected_regions()
    {
        for (std::size_t turn_index = 0; turn_index < m_turns.size(); ++turn_index)
        {
            turn_type const& turn = m_turns[turn_index];

            signed_size_type const unique_turn_id
                    = turn.is_clustered() ? -turn.cluster_id : turn_index;

            signed_size_type const& id0 = turn.operations[0].enriched.region_id;
            signed_size_type const& id1 = turn.operations[1].enriched.region_id;

            // Add region (by assigning) and add involved turns
            if (id0 != -1)
            {
                m_connected_regions[id0].region_id = id0;
            }
            if (id1 != -1 && id0 != id1)
            {
                m_connected_regions[id1].region_id = id1;
            }
        }
    }

    inline bool acceptable(turn_type const& turn) const
    {
        // Discarded turns don't connect rings to the same region
        // Also xx are not relevant
        // (otherwise discarded colocated uu turn could make a connection)
        return ! turn.discarded && ! turn.both(operation_blocked);
    }

    inline bool uu_or_ii(turn_type const& turn) const
    {
        return turn.both(operation_union) || turn.both(operation_intersection);
    }

    inline bool connects_same_region(turn_type const& turn) const
    {
        if (! acceptable(turn))
        {
            return false;
        }

        if (! turn.is_clustered())
        {
            // If it is a uu/ii-turn (non clustered), it is never same region
            return ! uu_or_ii(turn);
        }

        if BOOST_GEOMETRY_CONSTEXPR (TargetOperation == operation_union)
        {
            // It is a cluster, check zones
            // (assigned by sort_by_side/handle colocations) of both operations
            return turn.operations[0].enriched.zone
                    == turn.operations[1].enriched.zone;
        }
        else // else prevents unreachable code warning
        {
            // For an intersection, two regions connect if they are not ii
            // (ii-regions are isolated) or, in some cases, not iu (for example
            // when a multi-polygon is inside an interior ring and connecting it)
            return ! (turn.both(operation_intersection)
                      || turn.combination(operation_intersection, operation_union));
        }
    }

    void create_region(signed_size_type& new_region_id, ring_identifier const& ring_id,
                merged_ring_properties& properties, signed_size_type region_id = -1)
    {
        if (properties.region_id > 0)
        {
            // Already handled
            return;
        }

        // Assign new id if this is a new region
        if (region_id == -1)
        {
            region_id = new_region_id++;
        }

        // Assign this ring to specified region
        properties.region_id = region_id;

#if defined(BOOST_GEOMETRY_DEBUG_TRAVERSAL_SWITCH_DETECTOR)
        std::cout << " ADD " << ring_id << " TO REGION " << region_id << std::endl;
#endif

        // Find connecting rings, recursively
        for (auto turn_index : properties.turn_indices)
        {
            turn_type const& turn = m_turns[turn_index];
            if (! connects_same_region(turn))
            {
                // This is a non clustered uu/ii-turn, or a cluster connecting different 'zones'
                continue;
            }

            // Union: This turn connects two rings (interior connected), create the region
            // Intersection: This turn connects two rings, set same regions for these two rings
            for (auto const& op : turn.operations)
            {
                ring_identifier connected_ring_id = ring_id_by_seg_id(op.seg_id);
                if (connected_ring_id != ring_id)
                {
                    propagate_region(new_region_id, connected_ring_id, region_id);
                }
            }
        }
    }

    void propagate_region(signed_size_type& new_region_id,
            ring_identifier const& ring_id, signed_size_type region_id)
    {
        auto it = m_turns_per_ring.find(ring_id);
        if (it != m_turns_per_ring.end())
        {
            create_region(new_region_id, ring_id, it->second, region_id);
        }
    }

#if defined(BOOST_GEOMETRY_DEBUG_TRAVERSAL_SWITCH_DETECTOR)
    void debug_show_results()
   {
        for (auto const& kv : m_connected_regions)
        {
            auto const& prop = kv.second;
            std::cout << "REGION " << prop.region_id
                      << std::endl;
        }

        for (std::size_t turn_index = 0; turn_index < m_turns.size(); ++turn_index)
        {
            turn_type const& turn = m_turns[turn_index];

            if (uu_or_ii(turn) && ! turn.is_clustered())
            {
                std::cout << (turn.both(operation_union) ? "UU" : "II")
                          << " " << turn_index
                          << " (" << geometry::get<0>(turn.point)
                          << ", " << geometry::get<1>(turn.point) << ")"
                          << " -> [" << turn.operations[0].seg_id.source_index
                          << "/" << turn.operations[1].seg_id.source_index << "] "
                          << "(" << turn.operations[0].enriched.region_id
                          << ") / (" << turn.operations[1].enriched.region_id << ")"
                          << std::endl;
            }
        }

        for (auto const& key_val : m_clusters)
        {
            cluster_info const& cinfo = key_val.second;
            std::cout << "CL RESULT " << key_val.first
                         << " -> " << cinfo.open_count << std::endl;
        }
    }
#endif

    void iterate()
    {
#if defined(BOOST_GEOMETRY_DEBUG_TRAVERSAL_SWITCH_DETECTOR)
        std::cout << "BEGIN SWITCH DETECTOR (region_ids and isolation)" << std::endl;
#endif

        // Collect turns per ring
        m_turns_per_ring.clear();
        m_connected_regions.clear();

        for (std::size_t turn_index = 0; turn_index < m_turns.size(); ++turn_index)
        {
            turn_type const& turn = m_turns[turn_index];

            if BOOST_GEOMETRY_CONSTEXPR (TargetOperation == operation_intersection)
            {
                if (turn.discarded)
                {
                    // Discarded turn (union currently still needs it to determine regions)
                    continue;
                }
            }

            for (auto const& op : turn.operations)
            {
                m_turns_per_ring[ring_id_by_seg_id(op.seg_id)].turn_indices.insert(turn_index);
            }
        }

        // All rings having turns are in turns/ring map. Process them.
        {
            signed_size_type new_region_id = 1;
            for (auto& key_val : m_turns_per_ring)
            {
                create_region(new_region_id, key_val.first, key_val.second);
            }

            assign_region_ids_to_enriched();
            assign_connected_regions();
        }

#if defined(BOOST_GEOMETRY_DEBUG_TRAVERSAL_SWITCH_DETECTOR)
        std::cout << "END SWITCH DETECTOR" << std::endl;
        debug_show_results();
#endif

    }

private:

    Geometry1 const& m_geometry1;
    Geometry2 const& m_geometry2;
    Turns& m_turns;
    Clusters const& m_clusters;
    merge_map m_turns_per_ring;
    region_connection_map m_connected_regions;
    Visitor& m_visitor;
};

}} // namespace detail::overlay
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_TRAVERSAL_SWITCH_DETECTOR_HPP
