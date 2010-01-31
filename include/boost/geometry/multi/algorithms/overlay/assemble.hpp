// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_MULTI_ALGORITHMS_OVERLAY_ASSEMBLE_HPP
#define BOOST_GEOMETRY_MULTI_ALGORITHMS_OVERLAY_ASSEMBLE_HPP


#include <boost/assert.hpp>
#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>

#include <boost/geometry/algorithms/overlay/assemble.hpp>

#include <boost/geometry/multi/core/ring_type.hpp>

#include <boost/geometry/multi/algorithms/overlay/get_turns.hpp>
#include <boost/geometry/multi/algorithms/overlay/copy_segments.hpp>
#include <boost/geometry/multi/algorithms/overlay/copy_segment_point.hpp>
#include <boost/geometry/multi/algorithms/detail/point_on_border.hpp>
#include <boost/geometry/multi/algorithms/detail/sections/get_full_section.hpp>

#include <boost/geometry/multi/algorithms/envelope.hpp>
#include <boost/geometry/multi/algorithms/num_points.hpp>
#include <boost/geometry/multi/algorithms/within.hpp>


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
            Map const& map)
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
            >::apply(container, copy, *it, map);
        }
    }
};



template<>
struct get_ring<multi_polygon_tag>
{
    template<typename MultiPolygon>
    static inline typename ring_type<MultiPolygon>::type const& apply(
                ring_identifier const& id,
                MultiPolygon const& multi_polygon)
    {
        BOOST_ASSERT
            (
                id.multi_index >= 0
                && id.multi_index < boost::size(multi_polygon)
            );
        return get_ring<polygon_tag>::apply(id,
                    multi_polygon[id.multi_index]);
    }
};



}} // namespace detail::overlay
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry

#endif //GGL_ALGORITHMS_OVERLAY_ASSEMBLE_HPP
