// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2014 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2014 Adam Wulkiewicz, Lodz, Poland.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_SELECT_RINGS_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_SELECT_RINGS_HPP


#include <map>

#include <boost/range.hpp>

#include <boost/geometry/core/tags.hpp>

#include <boost/geometry/algorithms/area.hpp>
#include <boost/geometry/algorithms/within.hpp>
#include <boost/geometry/algorithms/detail/interior_iterator.hpp>
#include <boost/geometry/algorithms/detail/ring_identifier.hpp>
#include <boost/geometry/algorithms/detail/overlay/ring_properties.hpp>
#include <boost/geometry/algorithms/detail/overlay/overlay_type.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace overlay
{


namespace dispatch
{

    template <typename Tag, typename Geometry>
    struct select_rings
    {};

    template <typename Box>
    struct select_rings<box_tag, Box>
    {
        template <typename Geometry, typename RingPropertyMap>
        static inline void apply(Box const& box, Geometry const& ,
                ring_identifier const& id, RingPropertyMap& ring_properties,
                bool midpoint)
        {
            ring_properties[id] = typename RingPropertyMap::mapped_type(box, midpoint);
        }

        template <typename RingPropertyMap>
        static inline void apply(Box const& box,
                ring_identifier const& id, RingPropertyMap& ring_properties,
                bool midpoint)
        {
            ring_properties[id] = typename RingPropertyMap::mapped_type(box, midpoint);
        }
    };

    template <typename Ring>
    struct select_rings<ring_tag, Ring>
    {
        template <typename Geometry, typename RingPropertyMap>
        static inline void apply(Ring const& ring, Geometry const& ,
                    ring_identifier const& id, RingPropertyMap& ring_properties,
                    bool midpoint)
        {
            if (boost::size(ring) > 0)
            {
                ring_properties[id] = typename RingPropertyMap::mapped_type(ring, midpoint);
            }
        }

        template <typename RingPropertyMap>
        static inline void apply(Ring const& ring,
                    ring_identifier const& id, RingPropertyMap& ring_properties,
                    bool midpoint)
        {
            if (boost::size(ring) > 0)
            {
                ring_properties[id] = typename RingPropertyMap::mapped_type(ring, midpoint);
            }
        }
    };


    template <typename Polygon>
    struct select_rings<polygon_tag, Polygon>
    {
        template <typename Geometry, typename RingPropertyMap>
        static inline void apply(Polygon const& polygon, Geometry const& geometry,
                    ring_identifier id, RingPropertyMap& ring_properties,
                    bool midpoint)
        {
            typedef typename geometry::ring_type<Polygon>::type ring_type;
            typedef select_rings<ring_tag, ring_type> per_ring;

            per_ring::apply(exterior_ring(polygon), geometry, id, ring_properties, midpoint);

            typename interior_return_type<Polygon const>::type
                rings = interior_rings(polygon);
            for (typename detail::interior_iterator<Polygon const>::type
                    it = boost::begin(rings); it != boost::end(rings); ++it)
            {
                id.ring_index++;
                per_ring::apply(*it, geometry, id, ring_properties, midpoint);
            }
        }

        template <typename RingPropertyMap>
        static inline void apply(Polygon const& polygon,
                ring_identifier id, RingPropertyMap& ring_properties, bool midpoint)
        {
            typedef typename geometry::ring_type<Polygon>::type ring_type;
            typedef select_rings<ring_tag, ring_type> per_ring;

            per_ring::apply(exterior_ring(polygon), id, ring_properties, midpoint);

            typename interior_return_type<Polygon const>::type
                rings = interior_rings(polygon);
            for (typename detail::interior_iterator<Polygon const>::type
                    it = boost::begin(rings); it != boost::end(rings); ++it)
            {
                id.ring_index++;
                per_ring::apply(*it, id, ring_properties, midpoint);
            }
        }
    };

    template <typename Multi>
    struct select_rings<multi_polygon_tag, Multi>
    {
        template <typename Geometry, typename RingPropertyMap>
        static inline void apply(Multi const& multi, Geometry const& geometry,
                    ring_identifier id, RingPropertyMap& ring_properties, bool midpoint)
        {
            typedef typename boost::range_iterator
                <
                    Multi const
                >::type iterator_type;

            typedef select_rings<polygon_tag, typename boost::range_value<Multi>::type> per_polygon;

            id.multi_index = 0;
            for (iterator_type it = boost::begin(multi); it != boost::end(multi); ++it)
            {
                id.ring_index = -1;
                per_polygon::apply(*it, geometry, id, ring_properties, midpoint);
                id.multi_index++;
            }
        }
    };

} // namespace dispatch


template<overlay_type OverlayType>
struct decide
{};

template<>
struct decide<overlay_union>
{
    template <typename Code>
    static bool include(ring_identifier const& , Code const& code)
    {
        return code.within_code * -1 == 1;
    }

    template <typename Code>
    static bool reversed(ring_identifier const& , Code const& )
    {
        return false;
    }
};

template<>
struct decide<overlay_difference>
{
    template <typename Code>
    static bool include(ring_identifier const& id, Code const& code)
    {
        bool is_first = id.source_index == 0;
        return code.within_code * -1 * (is_first ? 1 : -1) == 1;
    }

    template <typename Code>
    static bool reversed(ring_identifier const& id, Code const& code)
    {
        return include(id, code) && id.source_index == 1;
    }
};

template<>
struct decide<overlay_intersection>
{
    template <typename Code>
    static bool include(ring_identifier const& , Code const& code)
    {
        return code.within_code * 1 == 1;
    }

    template <typename Code>
    static bool reversed(ring_identifier const& , Code const& )
    {
        return false;
    }
};


template
<
    overlay_type OverlayType,
    typename Geometry1,
    typename Geometry2,
    typename TurnCountMap,
    typename RingPropertyMap
>
inline void update_ring_selection(Geometry1 const& geometry1,
            Geometry2 const& geometry2,
            TurnCountMap const& turn_count_map,
            RingPropertyMap const& all_ring_properties,
            RingPropertyMap& selected_ring_properties)
{
    selected_ring_properties.clear();

    for (typename RingPropertyMap::const_iterator it = boost::begin(all_ring_properties);
        it != boost::end(all_ring_properties);
        ++it)
    {
        typename TurnCountMap::const_iterator tcit = turn_count_map.find(it->first);
        if (tcit == turn_count_map.end())
        {
            ring_identifier const id = it->first;
            typename RingPropertyMap::mapped_type properties = it->second; // Copy by value

            // Calculate the "within code" (previously this was done earlier but is
            // much efficienter here - it can be even more efficient doing it all at once,
            // using partition, TODO)
            // So though this is less elegant than before, it avoids many unused point-in-poly calculations
            switch(id.source_index)
            {
                case 0 :
                    properties.within_code
                        = geometry::within(properties.point, geometry2) ? 1 : -1;
                    break;
                case 1 :
                    properties.within_code
                        = geometry::within(properties.point, geometry1) ? 1 : -1;
                    break;
            }

            if (decide<OverlayType>::include(id, properties))
            {
                properties.reversed = decide<OverlayType>::reversed(id, properties);
                selected_ring_properties[id] = properties;
            }
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
    typename TurnCountMap,
    typename RingPropertyMap
>
inline void select_rings(Geometry1 const& geometry1, Geometry2 const& geometry2,
            TurnCountMap const& turn_count_per_ring,
            RingPropertyMap& selected_ring_properties, bool midpoint)
{
    typedef typename geometry::tag<Geometry1>::type tag1;
    typedef typename geometry::tag<Geometry2>::type tag2;

    RingPropertyMap all_ring_properties;
    dispatch::select_rings<tag1, Geometry1>::apply(geometry1, geometry2,
                ring_identifier(0, -1, -1), all_ring_properties, midpoint);
    dispatch::select_rings<tag2, Geometry2>::apply(geometry2, geometry1,
                ring_identifier(1, -1, -1), all_ring_properties, midpoint);

    update_ring_selection<OverlayType>(geometry1, geometry2, turn_count_per_ring,
                all_ring_properties, selected_ring_properties);
}

template
<
    overlay_type OverlayType,
    typename Geometry,
    typename TurnCountMap,
    typename RingPropertyMap
>
inline void select_rings(Geometry const& geometry,
            TurnCountMap const& turn_count_per_ring,
            RingPropertyMap& selected_ring_properties, bool midpoint)
{
    typedef typename geometry::tag<Geometry>::type tag;

    RingPropertyMap all_ring_properties;
    dispatch::select_rings<tag, Geometry>::apply(geometry,
                ring_identifier(0, -1, -1), all_ring_properties, midpoint);

    update_ring_selection<OverlayType>(geometry, geometry, turn_count_per_ring,
                all_ring_properties, selected_ring_properties);
}


}} // namespace detail::overlay
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_SELECT_RINGS_HPP
