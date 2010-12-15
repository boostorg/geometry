// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2010, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_ADD_TO_CONTAINMENT_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_ADD_TO_CONTAINMENT_HPP


#include <boost/mpl/assert.hpp>
#include <boost/range.hpp>
#include <boost/typeof/typeof.hpp>


#include <boost/geometry/core/tags.hpp>
#include <boost/geometry/core/exterior_ring.hpp>
#include <boost/geometry/core/interior_rings.hpp>
#include <boost/geometry/algorithms/detail/ring_identifier.hpp>

#include <boost/geometry/algorithms/convert.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace overlay
{


template <typename Tag, typename Geometry>
struct add_to_containment
{
    BOOST_MPL_ASSERT_MSG
        (
            false, NOT_OR_NOT_YET_IMPLEMENTED_FOR_THIS_GEOMETRY_TYPE
            , (types<Geometry>)
        );
};

template <typename Ring>
struct add_to_containment<ring_tag, Ring>
{
    template <typename ContainmentContainer, typename Map>
    static inline void apply(ContainmentContainer& container,
            ring_identifier const& id, Ring const& ring, Map const& map,
            bool dissolve)
    {
        typedef typename range_value<ContainmentContainer>::type prop;
        bool found = map.find(id) != map.end();
        if (! dissolve || ! found)
        {
            // For dissolve, do not add intersected rings
            container.push_back(prop(id, ring, found));
        }
    }
};

template <typename Box>
struct add_to_containment<box_tag, Box>
{
    template <typename ContainmentContainer, typename Map>
    static inline void apply(ContainmentContainer& container,
            ring_identifier const& id, Box const& box, Map const& map,
            bool dissolve)
    {
        typedef typename range_value<ContainmentContainer>::type prop;
        bool found = map.find(id) != map.end();
        if (! dissolve || ! found)
        {
            container.push_back(prop(id, box, found));
        }
    }
};


template <typename Polygon>
struct add_to_containment<polygon_tag, Polygon>
{
    template <typename ContainmentContainer, typename Map>
    static inline void apply(ContainmentContainer& container,
            ring_identifier const& id, Polygon const& polygon, Map const& map,
            bool dissolve)
    {
        // Add exterior ring and interior rings
        ring_identifier copy = id;

        typedef add_to_containment
            <
                ring_tag,
                typename ring_type<Polygon>::type
            > policy;

        policy::apply(container, copy, exterior_ring(polygon), map, dissolve);
        copy.ring_index = 0;

        typename interior_return_type<Polygon const>::type rings
                    = interior_rings(polygon);
        for (BOOST_AUTO(it, boost::begin(rings)); it != boost::end(rings);
            ++it, ++copy.ring_index)
        {
            policy::apply(container, copy, *it, map, dissolve);
        }
    }
};


}} // namespace detail::overlay
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_ADD_TO_CONTAINMENT_HPP
