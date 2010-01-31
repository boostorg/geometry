// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_DISTANCE_HPP
#define BOOST_GEOMETRY_STRATEGIES_DISTANCE_HPP


#include <boost/geometry/strategies/tags.hpp>


namespace boost { namespace geometry
{


/*!
    \brief Traits class binding a distance strategy to a (possibly two) coordinate system(s)
    \ingroup distance
    \tparam T1 tag of coordinate system of first point type
    \tparam T2 tag of coordinate system of second point type
    \tparam P1 first point-type
    \tparam P2 second point-type
*/
template <typename T1, typename T2, typename P1, typename P2>
struct strategy_distance
{
    typedef strategy::not_implemented type;
};

/*!
    \brief Traits class binding a distance-to-segment strategy to a (possibly two) coordinate system(s)
    \ingroup distance
    \tparam CsTag1 tag of coordinate system of point type
    \tparam CsTag2 tag of coordinate system of segment type, usually same as CsTag1
    \tparam Point point-type
    \tparam Segment segment-type
*/
template <typename CsTag1, typename CsTag2, typename Point, typename Segment>
struct strategy_distance_segment
{
    typedef strategy::not_implemented type;
};




}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_DISTANCE_HPP
