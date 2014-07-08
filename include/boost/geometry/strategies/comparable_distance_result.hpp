// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_STRATEGIES_COMPARABLE_DISTANCE_RESULT_HPP
#define BOOST_GEOMETRY_STRATEGIES_COMPARABLE_DISTANCE_RESULT_HPP

#include <boost/geometry/strategies/default_comparable_strategy.hpp>
#include <boost/geometry/strategies/default_strategy.hpp>
#include <boost/geometry/strategies/distance.hpp>
#include <boost/geometry/strategies/distance_result.hpp>

#include <boost/geometry/algorithms/detail/distance/default_strategies.hpp>


namespace boost { namespace geometry
{


/*!
\brief Meta-function defining return type of comparable_distance function
\ingroup distance
*/
template
<
    typename Geometry1,
    typename Geometry2 = Geometry1,
    typename Strategy = void
>
struct comparable_distance_result
    : distance_result
        <
            Geometry1,
            Geometry2,
            typename strategy::distance::services::comparable_type
                <
                    Strategy
                >::type
        >
{};

template <typename Geometry1, typename Geometry2>
struct comparable_distance_result<Geometry1, Geometry2, default_strategy>
    : distance_result<Geometry1, Geometry2, default_comparable_strategy>
{};

template <typename Geometry1, typename Geometry2>
struct comparable_distance_result
    <
        Geometry1, Geometry2, default_comparable_strategy
    > : distance_result<Geometry1, Geometry2, default_comparable_strategy>
{};

template <typename Geometry1, typename Geometry2>
struct comparable_distance_result<Geometry1, Geometry2, void>
    : distance_result<Geometry1, Geometry2, default_comparable_strategy>
{};


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_STRATEGIES_COMPARABLE_DISTANCE_RESULT_HPP
