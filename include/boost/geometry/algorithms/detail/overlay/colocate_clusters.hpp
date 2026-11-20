// Boost.Geometry

// Copyright (c) 2023 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_COLOCATE_CLUSTERS_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_COLOCATE_CLUSTERS_HPP

#include <boost/geometry/strategies/default_strategy.hpp>
#include <boost/geometry/strategies/detail/topology.hpp>

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace overlay
{

template <typename Clusters, typename Turns, typename Strategy>
inline void colocate_clusters(Clusters const& clusters, Turns& turns, Strategy const& strategy)
{
    auto const& topology = strategies::detail::get_topology(strategy);
    for (auto const& pair : clusters)
    {
        auto const& turn_indices = pair.second.turn_indices;
        if (turn_indices.size() < 2)
        {
            // Defensive check
            continue;
        }
        topology.colocate_cluster(turn_indices, turns);
    }
}

// Deprecated: pass the umbrella strategy so custom topology policies are honored.
template <typename Clusters, typename Turns>
inline void colocate_clusters(Clusters const& clusters, Turns& turns)
{
    colocate_clusters(clusters, turns, default_strategy());
}


}} // namespace detail::overlay
#endif //DOXYGEN_NO_DETAIL


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_COLOCATE_CLUSTERS_HPP
