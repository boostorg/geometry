// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_POINT_IN_POLY_HPP
#define BOOST_GEOMETRY_STRATEGIES_POINT_IN_POLY_HPP

#include <boost/geometry/strategies/tags.hpp>

namespace boost { namespace geometry
{


/*!
    \brief Traits class binding a within determination strategy to a coordinate system
    \ingroup within
    \tparam TagPoint tag of coordinate system of point-type
    \tparam TagSegment tag of coordinate system of segment-type
    \tparam Point point-type of input points
    \tparam PointOfSegment point-type of input segment-points
*/
template <typename TagPoint, typename TagSegment, typename Point, typename PointOfSegment>
struct strategy_within
{
    typedef strategy::not_implemented type;
};



}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_POINT_IN_POLY_HPP
