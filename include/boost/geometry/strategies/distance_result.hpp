// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_DISTANCE_RESULT_HPP
#define BOOST_GEOMETRY_STRATEGIES_DISTANCE_RESULT_HPP


#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/core/point_type.hpp>
#include <boost/geometry/strategies/distance.hpp>


namespace boost { namespace geometry
{

/*!
    \brief Meta-function defining return type of distance function
    \ingroup distance
    \note The strategy defines the return-type (so this situation is different
        from length, where distance is sqr/sqrt, but length always squared)

 */
template <typename Geometry1, typename Geometry2>
struct distance_result
{
    typedef typename default_distance_strategy
        <
            typename point_type<Geometry1>::type,
            typename point_type<Geometry2>::type
        >::type strategy_type;

    typedef typename strategy_type::return_type type;
};


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_STRATEGIES_DISTANCE_RESULT_HPP
