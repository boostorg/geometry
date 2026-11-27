// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2014 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2014 Adam Wulkiewicz, Lodz, Poland.

// This file was modified by Oracle on 2017-2022.
// Modifications copyright (c) 2017-2022 Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_SELECT_RINGS_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_SELECT_RINGS_HPP


#include <map>

#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/range/size.hpp>

#include <boost/geometry/core/tag_cast.hpp>
#include <boost/geometry/core/tags.hpp>

#include <boost/geometry/algorithms/detail/covered_by/implementation.hpp>
#include <boost/geometry/algorithms/detail/overlay/get_ring.hpp>
#include <boost/geometry/algorithms/detail/overlay/range_in_geometry.hpp>
#include <boost/geometry/algorithms/detail/overlay/ring_properties.hpp>
#include <boost/geometry/algorithms/detail/overlay/overlay_type.hpp>
#include <boost/geometry/algorithms/detail/ring_identifier.hpp>

#include <boost/geometry/views/detail/boundary_view.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace overlay
{

struct ring_turn_info
{
    bool has_traversed_turn;
    bool has_blocked_turn;

    ring_turn_info()
        : has_traversed_turn(false)
        , has_blocked_turn(false)
    {}
};

namespace dispatch
{

    struct include_all_rings
    {
        bool operator()(ring_identifier const&) const { return true; }
    };

    template <typename Tag, typename Geometry>
    struct select_rings
    {};

    template <typename Box>
    struct select_rings<box_tag, Box>
    {
        template <typename RingPropertyMap, typename Strategy, typename Filter = include_all_rings>
        static inline void apply(Box const& box,
                ring_identifier const& id, RingPropertyMap& ring_properties,
                Strategy const& strategy, Filter const& include = {})
        {
            if (include(id))
            {
                ring_properties[id] = typename RingPropertyMap::mapped_type(box, strategy);
            }
        }
    };

    template <typename Ring>
    struct select_rings<ring_tag, Ring>
    {
        template <typename RingPropertyMap, typename Strategy, typename Filter = include_all_rings>
        static inline void apply(Ring const& ring,
                    ring_identifier const& id, RingPropertyMap& ring_properties,
                    Strategy const& strategy, Filter const& include = {})
        {
            if (boost::size(ring) > 0 && include(id))
            {
                ring_properties[id] = typename RingPropertyMap::mapped_type(ring, strategy);
            }
        }
    };


    template <typename Polygon>
    struct select_rings<polygon_tag, Polygon>
    {
        template <typename RingPropertyMap, typename Strategy, typename Filter = include_all_rings>
        static inline void apply(Polygon const& polygon,
                    ring_identifier id, RingPropertyMap& ring_properties,
                    Strategy const& strategy, Filter const& include = {})
        {
            using per_ring = select_rings<ring_tag, geometry::ring_type_t<Polygon>>;

            per_ring::apply(exterior_ring(polygon), id, ring_properties, strategy, include);

            auto const& rings = interior_rings(polygon);
            for (auto it = boost::begin(rings); it != boost::end(rings); ++it)
            {
                id.ring_index++;
                per_ring::apply(*it, id, ring_properties, strategy, include);
            }
        }
    };

    template <typename Multi>
    struct select_rings<multi_polygon_tag, Multi>
    {
        template <typename RingPropertyMap, typename Strategy, typename Filter = include_all_rings>
        static inline void apply(Multi const& multi,
                    ring_identifier id, RingPropertyMap& ring_properties,
                    Strategy const& strategy, Filter const& include = {})
        {
            using per_polygon = select_rings<polygon_tag, typename boost::range_value<Multi>::type>;

            id.multi_index = 0;
            for (auto it = boost::begin(multi); it != boost::end(multi); ++it)
            {
                id.ring_index = -1;
                per_polygon::apply(*it, id, ring_properties, strategy, include);
                id.multi_index++;
            }
        }
    };

} // namespace dispatch


template <overlay_type OverlayType>
bool select_ring_boundary(ring_identifier const& id, bool within_other, bool& reversed)
{
    // Only membership in the ring's own operand changes across its boundary.
    unsigned const own = id.source_index == 0 ? 1 : 2;
    unsigned const outside = within_other ? 3 ^ own : 0;
    reversed = overlay_includes<OverlayType>(outside);
    return reversed != overlay_includes<OverlayType>(outside | own);
}

template <typename Geometry1, typename Geometry2,
          typename Tag1 = geometry::tag_t<Geometry1>,
          typename Tag2 = typename tag_cast<geometry::tag_t<Geometry2>, polygonal_tag>::type>
struct ring_within_geometry
{
    template <typename Strategy>
    static inline bool apply(Geometry1 const&, Geometry2 const&, Strategy const&)
    {
        return false;
    }
};

template <typename Ring, typename Geometry>
struct ring_within_geometry<Ring, Geometry, ring_tag, polygonal_tag>
{
    template <typename Strategy>
    static inline bool apply(Ring const& ring, Geometry const& geometry,
                             Strategy const& strategy)
    {
        // A hole's orientation is opposite to its ring type's point order.
        // Classify its boundary as a line, not as a standalone areal geometry.
        return geometry::within(detail::boundary_view<Ring const>(ring), geometry, strategy);
    }
};

template <typename Point, typename Ring, typename Geometry, typename Strategy>
inline int ring_in_geometry(Point const& point, Ring const& ring,
                            Geometry const& geometry, Strategy const& strategy)
{
    int const code = range_in_geometry(point, ring, geometry, strategy);
    // All vertices may touch the boundary while the intervening edges lie inside.
    return code == 0 && ring_within_geometry<Ring, Geometry>::apply(ring, geometry, strategy)
         ? 1 : code;
}

template
<
    overlay_type OverlayType,
    typename Geometry1,
    typename Geometry2,
    typename TurnInfoMap,
    typename RingPropertyMap,
    typename Strategy
>
inline void update_ring_selection(Geometry1 const& geometry1,
            Geometry2 const& geometry2,
            TurnInfoMap const& turn_info_map,
            RingPropertyMap const& all_ring_properties,
            RingPropertyMap& selected_ring_properties,
            Strategy const& strategy,
            bool include_on_boundary = false)
{
    selected_ring_properties.clear();

    for (auto const& pair : all_ring_properties)
    {
        if constexpr (ring_properties_detail::is_topological<typename RingPropertyMap::mapped_type>::value)
        {
            // A degenerate ring has no representative point to classify.
            if (!pair.second.valid)
            {
                continue;
            }
        }
        ring_identifier const& id = pair.first;

        auto const tcit = turn_info_map.find(id);
        if (tcit != turn_info_map.end()
            && (tcit->second.has_traversed_turn || tcit->second.has_blocked_turn))
        {
            // This turn is traversed or blocked,
            // don't include the original ring
            continue;
        }

        // Check if the ring is within the other geometry, by using
        // a point on the ring.
        int code = 0;
        switch(id.source_index)
        {
            // within
            case 0 :
                code = ring_in_geometry(pair.second.point,
                                        get_ring<geometry::tag_t<Geometry1>>::apply(id, geometry1),
                                        geometry2, strategy);
                break;
            case 1 :
                code = ring_in_geometry(pair.second.point,
                                        get_ring<geometry::tag_t<Geometry2>>::apply(id, geometry2),
                                        geometry1, strategy);
                break;
        }

        // Code 0: the point is located on the boundary. Normally such a
        // situation is resolved by the turns (the rings are traversed).
        // But if the overlay produced no turns at all, and the inputs are (nearly)
        // coincident without any segment intersection being detected (such as
        // issue 1471), then a point on the boundary should be counted as being
        // within the other geometry.
        bool const within_other = include_on_boundary ? code >= 0 : code > 0;

        bool reversed;
        if (select_ring_boundary<OverlayType>(id, within_other, reversed))
        {
            selected_ring_properties[id] = pair.second;
            selected_ring_properties[id].reversed = reversed;
        }
    }
}


/*!
\brief The function select_rings select rings based on the overlay-type (union,intersection)
*/
template
<
    overlay_type OverlayType,
    typename Geometry1,
    typename Geometry2,
    typename RingTurnInfoMap,
    typename RingPropertyMap,
    typename Strategy
>
inline void select_rings(Geometry1 const& geometry1, Geometry2 const& geometry2,
            RingTurnInfoMap const& turn_info_per_ring,
            RingPropertyMap& selected_ring_properties,
            Strategy const& strategy,
            bool include_on_boundary = false)
{
    using tag1 = geometry::tag_t<Geometry1>;
    using tag2 = geometry::tag_t<Geometry2>;

    RingPropertyMap all_ring_properties;
    dispatch::select_rings<tag1, Geometry1>::apply(geometry1,
                ring_identifier(0, -1, -1), all_ring_properties,
                strategy);
    dispatch::select_rings<tag2, Geometry2>::apply(geometry2,
                ring_identifier(1, -1, -1), all_ring_properties,
                strategy);

    update_ring_selection<OverlayType>(geometry1, geometry2, turn_info_per_ring,
                all_ring_properties, selected_ring_properties,
                strategy, include_on_boundary);
}

template
<
    overlay_type OverlayType,
    typename Geometry,
    typename RingTurnInfoMap,
    typename RingPropertyMap,
    typename Strategy
>
inline void select_rings(Geometry const& geometry,
            RingTurnInfoMap const& turn_info_per_ring,
            RingPropertyMap& selected_ring_properties,
            Strategy const& strategy,
            bool include_on_boundary = false)
{
    RingPropertyMap all_ring_properties;
    dispatch::select_rings<geometry::tag_t<Geometry>, Geometry>::apply(geometry,
                ring_identifier(0, -1, -1), all_ring_properties,
                strategy);

    update_ring_selection<OverlayType>(geometry, geometry, turn_info_per_ring,
                all_ring_properties, selected_ring_properties,
                strategy, include_on_boundary);
}


}} // namespace detail::overlay
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_SELECT_RINGS_HPP
