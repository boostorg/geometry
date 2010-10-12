// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_MULTI_ALGORITHMS_DETAIL_OVERLAY_ADD_TO_CONTAINMENT_HPP
#define BOOST_GEOMETRY_MULTI_ALGORITHMS_DETAIL_OVERLAY_ADD_TO_CONTAINMENT_HPP


#include <boost/range.hpp>

#include <boost/geometry/algorithms/detail/ring_identifier.hpp>
#include <boost/geometry/algorithms/detail/overlay/add_to_containment.hpp>
#include <boost/geometry/multi/core/tags.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace overlay
{


template <typename MultiPolygon>
struct add_to_containment<multi_polygon_tag, MultiPolygon>
{
    template <typename ContainmentContainer, typename Map>
    static inline void apply(ContainmentContainer& container,
            ring_identifier const& id, MultiPolygon const& multi_polygon,
            Map const& map, bool dissolve)
    {
        ring_identifier copy = id;
        copy.multi_index = 0;

        // Add all rings with the updated index
        for (typename boost::range_iterator<MultiPolygon const>::type it
                = boost::begin(multi_polygon);
            it != boost::end(multi_polygon);
            ++it, ++copy.multi_index)
        {
            add_to_containment
            <
                polygon_tag,
                typename boost::range_value<MultiPolygon>::type
            >::apply(container, copy, *it, map, dissolve);
        }
    }
};


}} // namespace detail::overlay
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_MULTI_ALGORITHMS_DETAIL_OVERLAY_ADD_TO_CONTAINMENT_HPP
