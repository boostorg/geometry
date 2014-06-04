// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_MULTI_ALGORITHMS_DETAIL_OVERLAY_GET_TURNS_HPP
#define BOOST_GEOMETRY_MULTI_ALGORITHMS_DETAIL_OVERLAY_GET_TURNS_HPP

#include <boost/geometry/algorithms/detail/overlay/get_turns.hpp>

#include <boost/geometry/core/ring_type.hpp>
#include <boost/geometry/core/tags.hpp>
#include <boost/geometry/geometries/concepts/check.hpp>

#include <boost/geometry/views/detail/range_type.hpp>

#include <boost/geometry/multi/algorithms/detail/sections/range_by_section.hpp>
#include <boost/geometry/multi/algorithms/detail/sections/sectionalize.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace get_turns
{

template
<
    typename Multi, typename Box,
    bool Reverse, bool ReverseBox,
    typename TurnPolicy
>
struct get_turns_multi_polygon_cs
{
    template <typename RobustPolicy, typename Turns, typename InterruptPolicy>
    static inline void apply(
            int source_id1, Multi const& multi,
            int source_id2, Box const& box,
            RobustPolicy const& robust_policy,
            Turns& turns, InterruptPolicy& interrupt_policy)
    {
        typedef typename boost::range_iterator
            <
                Multi const
            >::type iterator_type;

        int i = 0;
        for (iterator_type it = boost::begin(multi);
             it != boost::end(multi);
             ++it, ++i)
        {
            // Call its single version
            get_turns_polygon_cs
                <
                    typename boost::range_value<Multi>::type, Box,
                    Reverse, ReverseBox,
                    TurnPolicy
                >::apply(source_id1, *it, source_id2, box,
                            robust_policy, turns, interrupt_policy, i);
        }
    }
};

}} // namespace detail::get_turns
#endif // DOXYGEN_NO_DETAIL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


template
<
    typename MultiPolygon,
    typename Box,
    bool ReverseMultiPolygon, bool ReverseBox,
    typename TurnPolicy
>
struct get_turns
    <
        multi_polygon_tag, box_tag,
        MultiPolygon, Box,
        ReverseMultiPolygon, ReverseBox,
        TurnPolicy
    >
    : detail::get_turns::get_turns_multi_polygon_cs
        <
            MultiPolygon, Box,
            ReverseMultiPolygon, ReverseBox,
            TurnPolicy
        >
{};

} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_MULTI_ALGORITHMS_DETAIL_OVERLAY_GET_TURNS_HPP
