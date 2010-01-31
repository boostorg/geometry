// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_CONVEX_HULL_HPP
#define BOOST_GEOMETRY_STRATEGIES_CONVEX_HULL_HPP

#include <boost/geometry/strategies/tags.hpp>


namespace boost { namespace geometry
{




/*!
    \brief Traits class binding a convex hull calculation strategy to a coordinate system
    \ingroup convex_hull
    \tparam Tag tag of coordinate system
    \tparam Geometry the geometry type (hull operates internally per hull over geometry)
    \tparam Point point-type of output points
*/
template <typename Tag, typename Geometry, typename Point>
struct strategy_convex_hull
{
    typedef strategy::not_implemented type;
};


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_CONVEX_HULL_HPP
