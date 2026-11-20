// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2021 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2024.
// Modifications copyright (c) 2024 Oracle and/or its affiliates.
// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_GET_CLUSTERS_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_GET_CLUSTERS_HPP

#include <algorithm>
#include <map>
#include <type_traits>
#include <vector>

#include <boost/geometry/algorithms/detail/overlay/cluster_info.hpp>
#include <boost/geometry/algorithms/detail/overlay/get_ring.hpp>
#include <boost/range/value_type.hpp>
#include <boost/geometry/strategies/default_strategy.hpp>
#include <boost/geometry/strategies/detail/topology.hpp>
#include <boost/geometry/strategies/relate/intersection_types.hpp>

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace overlay
{

template <typename Point>
struct turn_with_point
{
    std::size_t turn_index;
    Point pnt;
};

template <typename Cluster, typename Point>
struct cluster_with_point
{
    Cluster cluster;
    Point pnt;
};

template <typename Compare>
struct exact_cluster_builder
{
    template <typename Turns, typename Clusters, typename Strategy>
    static bool apply(Turns const& turns, Clusters& clusters, Strategy const& strategy)
    {
        using cluster_type = typename Clusters::mapped_type;
        auto const& topology = strategies::detail::get_topology(strategy);

        std::vector<std::size_t> turn_indices;
        std::size_t turn_index = 0;
        for (auto const& turn : turns)
        {
            if (! turn.discarded)
            {
                turn_indices.push_back(turn_index);
            }
            ++turn_index;
        }
        std::sort(turn_indices.begin(), turn_indices.end(), [&](auto a, auto b)
        {
            return Compare::apply(turns[a].point, turns[b].point);
        });

        // Certified ordering puts each exact equality class in one contiguous
        // run. Every run of two or more points is still a cluster; colocation
        // subsequently unifies its point representation as before.
        signed_size_type cluster_id = 1;
        for (auto first = turn_indices.begin(); first != turn_indices.end(); )
        {
            auto last = first + 1;
            while (last != turn_indices.end()
                && topology.cluster_equal(turns[*first].point, turns[*last].point))
            {
                ++last;
            }
            if (last - first > 1)
            {
                cluster_type cluster;
                for (auto it = first; it != last; ++it)
                {
                    cluster.turn_indices.insert(*it);
                }
                clusters[cluster_id++] = cluster;
            }
            first = last;
        }
        return true;
    }
};

template <>
struct exact_cluster_builder<void>
{
    template <typename Turns, typename Clusters, typename Strategy>
    static bool apply(Turns const&, Clusters&, Strategy const&)
    {
        return false;
    }
};

// Use a sweep algorithm to detect clusters
template
<
    typename Turns,
    typename Clusters,
    typename Strategy
>
inline void get_clusters(Turns& turns, Clusters& clusters, Strategy const& strategy)
{
    using exact_compare = typename std::conditional
        <
            strategies::relate::services::exact_predicates<Strategy>::value,
            typename strategies::relate::services::exact_cluster_compare<Strategy>::type,
            void
        >::type;
    if (exact_cluster_builder<exact_compare>::apply(turns, clusters, strategy))
    {
        return;
    }

    using turn_type = typename boost::range_value<Turns>::type;
    using cluster_type = typename Clusters::mapped_type;
    using point_type = typename turn_type::point_type;

    auto const& topology = strategies::detail::get_topology(strategy);

    std::vector<turn_with_point<point_type>> points;
    std::size_t turn_index = 0;
    for (auto const& turn : turns)
    {
        if (! turn.discarded)
        {
            points.push_back({turn_index, turn.point});
        }
        turn_index++;
    }

    // Sort the points from top to bottom
    std::sort(points.begin(), points.end(), [&](auto const& e1, auto const& e2)
    {
       return topology.compare_y(e1.pnt, e2.pnt) > 0;
    });

    // The output vector will be sorted from bottom too
    std::vector<cluster_with_point<cluster_type, point_type>> clustered_points;

    // Compare points with each other. Performance is O(n log(n)) because of the sorting.
    for (auto it1 = points.begin(); it1 != points.end(); ++it1)
    {
        // Inner loop, iterates until it exceeds coordinates in y-direction
        for (auto it2 = it1 + 1; it2 != points.end(); ++it2)
        {
            if (topology.cluster_window_exceeded(it1->pnt, it2->pnt))
            {
                // Points at this y-coordinate or below cannot be equal
                break;
            }
            if (topology.cluster_equal(it1->pnt, it2->pnt))
            {
                std::size_t cindex = 0;

                // Most recent clusters (with this y-value) are at the bottom
                // therefore we can stop as soon as the y-value is out of reach (TODO)
                bool found = false;
                for (auto cit = clustered_points.begin();
                     cit != clustered_points.end(); ++cit)
                {
                    found = topology.cluster_equal(cit->pnt, it1->pnt);
                    if (found)
                    {
                        break;
                    }
                    cindex++;
                }

                // Add new cluster
                if (! found)
                {
                   cindex = clustered_points.size();
                   cluster_type newcluster;
                   clustered_points.push_back({newcluster, it1->pnt});
                }
                clustered_points[cindex].cluster.turn_indices.insert(it1->turn_index);
                clustered_points[cindex].cluster.turn_indices.insert(it2->turn_index);
            }
        }
    }

    // Convert to map
    signed_size_type cluster_id = 1;
    for (auto& trace : clustered_points)
    {
        clusters[cluster_id++] = trace.cluster;
    }
}

// Deprecated: pass the umbrella strategy so custom topology policies are honored.
template <typename Turns, typename Clusters>
inline void get_clusters(Turns& turns, Clusters& clusters)
{
    get_clusters(turns, clusters, default_strategy());
}

}} // namespace detail::overlay
#endif //DOXYGEN_NO_DETAIL


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_GET_CLUSTERS_HPP
