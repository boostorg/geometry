// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_SIDE_HPP
#define BOOST_GEOMETRY_STRATEGIES_SIDE_HPP


#include <boost/geometry/strategies/tags.hpp>


namespace boost { namespace geometry
{


/*!
\brief Traits class binding a side determination strategy to a coordinate system
\ingroup util
\tparam Tag tag of coordinate system of point-type
\tparam CalculationType \tparam_calculation
*/
template <typename Tag, typename CalculationType = void>
struct strategy_side
{
    typedef strategy::not_implemented type;
};


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_SIDE_HPP
