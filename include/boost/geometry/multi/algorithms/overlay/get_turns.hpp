// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_MULTI_ALGORITHMS_GET_TURNS_HPP
#define BOOST_GEOMETRY_MULTI_ALGORITHMS_GET_TURNS_HPP

#include <boost/geometry/multi/core/is_multi.hpp>

#include <boost/geometry/multi/algorithms/distance.hpp>
#include <boost/geometry/multi/algorithms/get_section.hpp>
#include <boost/geometry/multi/algorithms/sectionalize.hpp>
#include <boost/geometry/multi/iterators/range_type.hpp>

#include <boost/geometry/algorithms/overlay/get_turns.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


template
<
    typename MultiTag1,
    typename MultiTag2,
    typename MultiGeometry1,
    typename MultiGeometry2,
    typename IntersectionPoints,
    typename Strategy,
    typename AssignPolicy
>
struct get_turns
    <
        MultiTag1, MultiTag2,
        true, true,
        MultiGeometry1, MultiGeometry2,
        IntersectionPoints,
        Strategy,
        AssignPolicy
    >
    : detail::get_turns::get_turns_generic
        <
            MultiGeometry1,
            MultiGeometry2,
            IntersectionPoints,
            Strategy,
            AssignPolicy
        >
{};


template
<
    typename SingleTag,
    typename MultiTag,
    typename SingleGeometry,
    typename MultiGeometry,
    typename IntersectionPoints,
    typename Strategy,
    typename AssignPolicy
>
struct get_turns
    <
        SingleTag, MultiTag,
        false, true,
        SingleGeometry, MultiGeometry,
        IntersectionPoints,
        Strategy,
        AssignPolicy
    >
    : detail::get_turns::get_turns_generic
        <
            SingleGeometry,
            MultiGeometry,
            IntersectionPoints,
            Strategy,
            AssignPolicy
        >
{};


// Version for multi/single, necessary for multi_polygon/ring
template
<
    typename MultiTag,
    typename SingleTag,
    typename MultiGeometry,
    typename SingleGeometry,
    typename IntersectionPoints,
    typename Strategy,
    typename AssignPolicy
>
struct get_turns
    <
        MultiTag, SingleTag,
        true, false,
        MultiGeometry, SingleGeometry,
        IntersectionPoints,
        Strategy,
        AssignPolicy
    >
    : detail::get_turns::get_turns_generic
        <
            MultiGeometry,
            SingleGeometry,
            IntersectionPoints,
            Strategy,
            AssignPolicy
        >
{};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_MULTI_ALGORITHMS_GET_TURNS_HPP
