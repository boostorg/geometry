// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2017 Adam Wulkiewicz, Lodz, Poland.

// This file was modified by Oracle on 2017-2023.
// Modifications copyright (c) 2017-2023 Oracle and/or its affiliates.
// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_ASSIGN_PARENTS_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_ASSIGN_PARENTS_HPP

#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>

#include <boost/geometry/core/coordinate_type.hpp>

#include <boost/geometry/algorithms/envelope.hpp>
#include <boost/geometry/algorithms/expand.hpp>
#include <boost/geometry/algorithms/detail/covered_by/implementation.hpp>
#include <boost/geometry/algorithms/detail/immediate_parent.hpp>
#include <boost/geometry/algorithms/detail/partition.hpp>
#include <boost/geometry/algorithms/detail/overlay/get_ring.hpp>
#include <boost/geometry/algorithms/detail/overlay/range_in_geometry.hpp>
#include <boost/geometry/algorithms/detail/overlay/ring_properties.hpp>
#include <boost/geometry/views/enumerate_view.hpp>

#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/strategies/relate/intersection_types.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace overlay
{



template
<
    typename Item,
    typename InnerGeometry,
    typename Geometry1, typename Geometry2,
    typename RingCollection,
    typename Strategy
>
inline bool within_selected_input(Item const& item2,
        InnerGeometry const& inner_geometry,
        ring_identifier const& outer_id,
        Geometry1 const& geometry1, Geometry2 const& geometry2,
        RingCollection const& collection,
        Strategy const& strategy)
{
    using tag1 = geometry::tag_t<Geometry1>;
    using tag2 = geometry::tag_t<Geometry2>;

    // NOTE: range_in_geometry first checks the item2.point and then
    // if this point is on boundary it checks points of inner_geometry
    // ring until a point inside/outside other geometry ring is found
    switch (outer_id.source_index)
    {
        // covered_by
        case 0 :
            return range_in_geometry(item2.point, inner_geometry,
                get_ring<tag1>::apply(outer_id, geometry1), strategy) >= 0;
        case 1 :
            return range_in_geometry(item2.point, inner_geometry,
                get_ring<tag2>::apply(outer_id, geometry2), strategy) >= 0;
        case 2 :
            return range_in_geometry(item2.point, inner_geometry,
                get_ring<void>::apply(outer_id, collection), strategy) >= 0;
    }
    return false;
}

template
<
    typename Item,
    typename Geometry1, typename Geometry2,
    typename RingCollection,
    typename Strategy
>
inline bool within_selected_input(Item const& item2,
        ring_identifier const& inner_id, ring_identifier const& outer_id,
        Geometry1 const& geometry1, Geometry2 const& geometry2,
        RingCollection const& collection,
        Strategy const& strategy)
{
    using tag1 = geometry::tag_t<Geometry1>;
    using tag2 = geometry::tag_t<Geometry2>;

    switch (inner_id.source_index)
    {
        case 0 :
            return within_selected_input(item2,
               get_ring<tag1>::apply(inner_id, geometry1),
               outer_id, geometry1, geometry2, collection, strategy);
        case 1 :
            return within_selected_input(item2,
               get_ring<tag2>::apply(inner_id, geometry2),
               outer_id, geometry1, geometry2, collection, strategy);
        case 2 :
            return within_selected_input(item2,
                get_ring<void>::apply(inner_id, collection),
                outer_id, geometry1, geometry2, collection, strategy);
    }
    return false;
}


using strategies::detail::classify_ring_measure;
using strategies::detail::ring_direction;

template <typename Point, typename AreaType, typename Box = model::box<Point>>
struct ring_info_helper
{
    ring_identifier id;
    AreaType real_area;
    AreaType abs_area;
    Box envelope;

    inline ring_info_helper()
        : real_area(0), abs_area(0)
    {}

    inline ring_info_helper(ring_identifier i, AreaType const& a)
        : id(i), real_area(a), abs_area(geometry::math::abs(a))
    {}
};


template <typename Strategy>
struct ring_info_helper_get_box
{
    ring_info_helper_get_box(Strategy const& strategy)
        : m_strategy(strategy)
    {}

    template <typename Box, typename InputItem>
    inline void apply(Box& total, InputItem const& item) const
    {
        assert_coordinate_type_equal(total, item.envelope);
        geometry::expand(total, item.envelope, m_strategy);
    }

    Strategy const& m_strategy;
};

template <typename Strategy>
struct ring_info_helper_overlaps_box
{
    ring_info_helper_overlaps_box(Strategy const& strategy)
        : m_strategy(strategy)
    {}

    template <typename Box, typename InputItem>
    inline bool apply(Box const& box, InputItem const& item) const
    {
        assert_coordinate_type_equal(box, item.envelope);
        return ! geometry::detail::disjoint::disjoint_box_box(
                    box, item.envelope, m_strategy);
    }

    Strategy const& m_strategy;
};

// Segments intersection Strategy
template
<
    typename Geometry1,
    typename Geometry2,
    typename Collection,
    typename RingMap,
    typename Strategy
>
struct assign_visitor
{
    using ring_info_type = typename RingMap::mapped_type;

    Geometry1 const& m_geometry1;
    Geometry2 const& m_geometry2;
    Collection const& m_collection;
    RingMap& m_ring_map;
    Strategy const& m_strategy;
    bool m_check_for_orientation;

    inline assign_visitor(Geometry1 const& g1, Geometry2 const& g2, Collection const& c,
                          RingMap& map, Strategy const& strategy, bool check)
        : m_geometry1(g1)
        , m_geometry2(g2)
        , m_collection(c)
        , m_ring_map(map)
        , m_strategy(strategy)
        , m_check_for_orientation(check)
    {}

    template <typename Item>
    inline bool apply(Item const& outer, Item const& inner, bool first = true)
    {
        if (first && outer.abs_area < inner.abs_area)
        {
            // Apply with reversed arguments
            apply(inner, outer, false);
            return true;
        }

        if (m_check_for_orientation
         || (classify_ring_measure(m_strategy, outer.real_area) == ring_direction::positive
          && classify_ring_measure(m_strategy, inner.real_area) == ring_direction::negative))
        {
            ring_info_type& inner_in_map = m_ring_map[inner.id];

            if (geometry::covered_by(inner_in_map.point, outer.envelope, m_strategy)
               && within_selected_input(inner_in_map, inner.id, outer.id,
                                        m_geometry1, m_geometry2, m_collection,
                                        m_strategy)
               )
            {
                // Assign a parent if there was no earlier parent, or the newly
                // found parent is smaller than the previous one
                if (inner_in_map.parent.source_index == -1
                    || outer.abs_area < inner_in_map.parent_area)
                {
                    inner_in_map.parent = outer.id;
                    inner_in_map.parent_area = outer.abs_area;
                }
            }
        }

        return true;
    }
};

template <overlay_type OverlayType, bool Topological, typename RingMap, typename Sign>
inline void finalize_ring_parents(RingMap& map, Sign const& sign)
{
    if (OverlayType == overlay_buffer || OverlayType == overlay_dissolve)
    {
        for (auto& pair : map)
        {
            auto& info = pair.second;
            if (Topological && info.discarded)
                continue;
            auto const direction = sign(info, false);
            if (direction == ring_direction::zero)
            {
                info.discarded = true;
            }
            else if (info.parent.source_index >= 0)
            {
                bool const parent_positive = sign(map.find(info.parent)->second, true) == ring_direction::positive;
                if ((direction == ring_direction::positive && parent_positive)
                    || (OverlayType == overlay_dissolve && direction != ring_direction::positive && !parent_positive))
                    info.discarded = true;
                if (direction == ring_direction::positive || info.discarded)
                    info.parent = ring_identifier();
            }
            else if (direction == ring_direction::negative)
            {
                info.reversed = !Topological || !info.reversed;
            }
        }
    }
    for (auto& pair : map)
    {
        auto const& info = pair.second;
        if (info.parent.source_index >= 0 && (!Topological || !info.discarded))
            map.find(info.parent)->second.children.push_back(pair.first);
    }
}

template
<
    overlay_type OverlayType,
    typename Geometry1, typename Geometry2,
    typename RingCollection,
    typename RingMap,
    typename Strategy
>
inline void assign_parents_by_area(Geometry1 const& geometry1,
            Geometry2 const& geometry2,
            RingCollection const& collection,
            RingMap& ring_map,
            Strategy const& strategy)
{
    static bool const is_buffer = OverlayType == overlay_buffer;
    static bool const is_dissolve = OverlayType == overlay_dissolve;
    static bool const check_for_orientation = is_buffer || is_dissolve;

    using tag1 = geometry::tag_t<Geometry1>;
    using tag2 = geometry::tag_t<Geometry2>;

    using ring_info_type = typename RingMap::mapped_type;
    using point_type = typename ring_info_type::point_type;
    using box_type = typename strategies::relate::services::box_type<Strategy, point_type>::type;
    using area_result_type = typename ring_info_type::area_type;

    {
        std::size_t count_total = ring_map.size();
        std::size_t count_positive = 0;
        std::size_t index_positive = 0; // only used if count_positive>0

        // Copy to vector (this might be obsolete, using the map directly)
        // The index in the map is also the index in the vector.
        using helper = ring_info_helper<point_type, area_result_type, box_type>;
        std::vector<helper> vector(count_total);

        for (auto const& enumerated : util::enumerate(ring_map))
        {
            auto const& ring_id = enumerated.value.first;
            auto const& info = enumerated.value.second;
            vector[enumerated.index] = helper(ring_id, info.get_area());
            helper& item = vector[enumerated.index];
            switch(ring_id.source_index)
            {
                case 0 :
                    geometry::envelope(get_ring<tag1>::apply(ring_id, geometry1),
                                       item.envelope, strategy);
                    break;
                case 1 :
                    geometry::envelope(get_ring<tag2>::apply(ring_id, geometry2),
                                       item.envelope, strategy);
                    break;
                case 2 :
                    geometry::envelope(get_ring<void>::apply(ring_id, collection),
                                       item.envelope, strategy);
                    break;
            }

            // Expand envelope slightly
            strategies::detail::expand_search_envelope(item.envelope, strategy);

            if (classify_ring_measure<false>(strategy, item.real_area) == ring_direction::positive)
            {
                count_positive++;
                index_positive = enumerated.index;
            }
        }

        if (! check_for_orientation)
        {
            if (count_positive == count_total)
            {
                // Optimization for only positive rings
                // -> no assignment of parents or reversal necessary, ready here.
                return;
            }

            if (count_positive == 1 && OverlayType == overlay_union)
            {
                // Optimization for one outer ring
                // -> assign this as parent to all others (all interior rings)
                // In unions, this is probably the most occuring case and gives
                //    a dramatic improvement (factor 5 for star_comb testcase)
                // In difference or other cases where interior rings might be
                // located outside the outer ring, this cannot be done
                ring_identifier id_of_positive = vector[index_positive].id;
                ring_info_type& outer = ring_map[id_of_positive];
                for (auto const& item : util::enumerate(vector))
                {
                    if (item.index != index_positive)
                    {
                        auto const id = item.value.id;
                        ring_info_type& inner = ring_map[id];
                        inner.parent = id_of_positive;
                        outer.children.push_back(id);
                    }
                }
                return;
            }
        }

        assign_visitor
            <
                Geometry1, Geometry2,
                RingCollection, RingMap,
                Strategy
            > visitor(geometry1, geometry2, collection, ring_map, strategy, check_for_orientation);

        geometry::partition
            <
                box_type
            >::apply(vector, visitor,
                     ring_info_helper_get_box<Strategy>(strategy),
                     ring_info_helper_overlaps_box<Strategy>(strategy));
    }

    finalize_ring_parents<OverlayType, false>(ring_map, [&](auto const& info, bool parent)
    {
        auto const area = parent ? info.area : info.get_area();
        return classify_ring_measure(strategy, area);
    });
}


template <overlay_type OverlayType, typename Geometry1, typename Geometry2,
      typename Collection, typename RingMap, typename Topology>
inline void assign_parents_by_containment(Geometry1 const& a, Geometry2 const& b,
    Collection const& collection, RingMap& map, Topology const& topology)
{
    if (map.size() == 1)
    {
        auto& info = map.begin()->second;
        if (info.valid && info.effective_direction() > 0)
        {
            info.children.clear();
            info.parent = ring_identifier();
            return;
        }
    }
    using properties = typename RingMap::mapped_type;
    struct selected_ring
    {
        ring_identifier id;
        properties* info;
        decltype(topology.boundary(ring_identifier(), a, b, collection, false)) boundary;
    };
    std::vector<selected_ring> rings;
    constexpr bool allow_certificate = OverlayType == overlay_union
        || OverlayType == overlay_intersection || OverlayType == overlay_difference;
    for (auto& pair : map)
    {
        auto& info = pair.second;
        info.children.clear();
        info.parent = ring_identifier();
        if (!info.valid || info.effective_direction() == 0) { info.discarded = true; continue; }
        rings.push_back({pair.first, &info, topology.boundary(
            pair.first, a, b, collection, allow_certificate)});
    }
    auto relation = [&](std::size_t inner, std::size_t outer)
    {
        return topology.classify(rings[inner].boundary, rings[outer].boundary,
                                 rings[inner].info->boundary_point);
    };
    // Coincident chains are not strict parents. Reduce their directed
    // multiplicities before choosing containers, so parent links cannot cycle.
    std::vector<std::size_t> group(rings.size());
    for (std::size_t i = 0; i < rings.size(); ++i) group[i] = i;
    auto root = [&](std::size_t i) { while (group[i] != i) i = group[i]; return i; };
    for (std::size_t i = 0; i < rings.size(); ++i)
        for (std::size_t j = i + 1; j < rings.size(); ++j)
            if (root(i) != root(j) && topology.coincident(
                    rings[i].boundary, rings[j].boundary,
                    rings[i].info->boundary_point, rings[j].info->boundary_point))
            {
                group[root(j)] = root(i);
            }
    std::vector<std::ptrdiff_t> net(rings.size(), 0);
    for (std::size_t i = 0; i < rings.size(); ++i) net[root(i)] += rings[i].info->effective_direction();
    std::vector<bool> kept(rings.size(), false);
    for (std::size_t i = 0; i < rings.size(); ++i)
    {
        auto const parent = root(i);
        auto const direction = rings[i].info->effective_direction();
        if (!kept[parent] && ((net[parent] > 0 && direction > 0) || (net[parent] < 0 && direction < 0)))
            kept[parent] = true;
        else rings[i].info->discarded = true;
    }
    constexpr bool check_orientation = OverlayType == overlay_buffer || OverlayType == overlay_dissolve;
    for (std::size_t inner = 0; inner < rings.size(); ++inner)
    {
        auto& info = *rings[inner].info;
        if (info.discarded || (!check_orientation && info.effective_direction() > 0)) continue;
        auto const parent = detail::immediate_parent(rings.size(), inner,
            [&](std::size_t child, std::size_t container)
            {
                auto const& candidate = *rings[container].info;
                return !candidate.discarded
                    && (check_orientation || candidate.effective_direction() > 0)
                    && relation(child, container) >= 0 && relation(container, child) < 0;
            });
        if (parent != rings.size()) info.parent = rings[parent].id;
    }
    finalize_ring_parents<OverlayType, true>(map, [](auto const& info, bool parent)
    {
        auto const sign = parent ? info.direction : info.effective_direction();
        return sign > 0 ? ring_direction::positive
             : sign < 0 ? ring_direction::negative : ring_direction::zero;
    });
}

template <overlay_type OverlayType, typename Geometry1, typename Geometry2,
          typename Collection, typename RingMap, typename Strategy>
inline void assign_parents_dispatch(Geometry1 const& a, Geometry2 const& b, Collection const& collection,
                                   RingMap& map, Strategy const& strategy, std::false_type)
{
    assign_parents_by_area<OverlayType>(a, b, collection, map, strategy);
}

template <overlay_type OverlayType, typename Geometry1, typename Geometry2,
          typename Collection, typename RingMap, typename Strategy>
inline void assign_parents_dispatch(Geometry1 const& a, Geometry2 const& b, Collection const& collection,
                                   RingMap& map, Strategy const& strategy, std::true_type)
{
    assign_parents_by_containment<OverlayType>(a, b, collection, map,
        strategy.ring_topology());
}

template <overlay_type OverlayType, typename Geometry1, typename Geometry2,
          typename Collection, typename RingMap, typename Strategy>
inline void assign_parents(Geometry1 const& a, Geometry2 const& b, Collection const& collection,
                          RingMap& map, Strategy const& strategy)
{
    assign_parents_dispatch<OverlayType>(a, b, collection, map, strategy,
        ring_properties_detail::is_topological<typename RingMap::mapped_type>());
}

// Version for one geometry (called by buffer/dissolve)
template
<
    overlay_type OverlayType,
    typename Geometry,
    typename RingCollection,
    typename RingMap,
    typename Strategy
>
inline void assign_parents(Geometry const& geometry,
            RingCollection const& collection,
            RingMap& ring_map,
            Strategy const& strategy)
{
    // Call it with an empty geometry as second geometry (source_id == 1)
    // (ring_map should be empty for source_id==1)
    Geometry empty;
    assign_parents<OverlayType>(geometry, empty,
            collection, ring_map, strategy);
}


}} // namespace detail::overlay
#endif // DOXYGEN_NO_DETAIL


}} // namespace geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_ASSIGN_PARENTS_HPP
