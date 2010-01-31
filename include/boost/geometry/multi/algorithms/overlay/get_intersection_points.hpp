// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_MULTI_ALGORITHMS_GET_INTERSECTION_POINTS_HPP
#define BOOST_GEOMETRY_MULTI_ALGORITHMS_GET_INTERSECTION_POINTS_HPP

#include <boost/geometry/multi/core/is_multi.hpp>

#include <boost/geometry/multi/algorithms/distance.hpp>
#include <boost/geometry/multi/algorithms/get_section.hpp>
#include <boost/geometry/multi/algorithms/sectionalize.hpp>

#include <boost/geometry/multi/iterators/range_type.hpp>

#include <boost/geometry/algorithms/overlay/get_intersection_points.hpp>



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
    typename Strategy
>
struct get_intersection_points
    <
        MultiTag1, MultiTag2,
        true, true,
        MultiGeometry1, MultiGeometry2,
        IntersectionPoints,
        Strategy
    >
    : detail::get_intersection_points::get_ips_generic
        <
            MultiGeometry1,
            MultiGeometry2,
            IntersectionPoints,
            Strategy
        >
{};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH



}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_MULTI_ALGORITHMS_GET_INTERSECTION_POINTS_HPP
