// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_AREA_RESULT_HPP
#define BOOST_GEOMETRY_STRATEGIES_AREA_RESULT_HPP


#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/strategies/area.hpp>
#include <boost/geometry/util/select_most_precise.hpp>


namespace boost { namespace geometry
{

/*!
    \brief Meta-function defining return type of area function
    \ingroup area
    \note The strategy defines the return-type (so this situation is different
        from length, where distance is sqr/sqrt, but length always squared)

 */

// TODO: rename to "default_area_result" or services::default_result


template <typename Geometry>
struct area_result
{
    typedef typename point_type<Geometry>::type point_type;
    typedef typename strategy::area::services::default_strategy
        <
            typename cs_tag<point_type>::type,
            point_type
        >::type strategy_type;

    typedef typename strategy_type::return_type type;
};


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_STRATEGIES_AREA_RESULT_HPP
