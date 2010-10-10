// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_MULTI_ALGORITHMS_INTERSECTION_HPP
#define BOOST_GEOMETRY_MULTI_ALGORITHMS_INTERSECTION_HPP


#include <boost/geometry/algorithms/intersection.hpp>
#include <boost/geometry/multi/algorithms/detail/overlay/assemble.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


template
<
    typename Polygon, typename MultiPolygon,
    typename OutputIterator,
    typename PolygonOut,
    typename Strategy
>
struct intersection_inserter
    <
        polygon_tag, multi_polygon_tag, polygon_tag,
        Polygon, MultiPolygon,
        OutputIterator, PolygonOut,
        Strategy
    > : detail::overlay::overlay
        <Polygon, MultiPolygon, OutputIterator, PolygonOut, -1, Strategy>
{};


template
<
    typename MultiPolygon1, typename MultiPolygon2,
    typename OutputIterator,
    typename PolygonOut,
    typename Strategy
>
struct intersection_inserter
    <
        multi_polygon_tag, multi_polygon_tag, polygon_tag,
        MultiPolygon1, MultiPolygon2,
        OutputIterator, PolygonOut,
        Strategy
    > : detail::overlay::overlay
        <MultiPolygon1, MultiPolygon2, OutputIterator, PolygonOut, -1, Strategy>
{};


} // namespace dispatch
#endif

}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_MULTI_ALGORITHMS_INTERSECTION_HPP

