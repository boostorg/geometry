// Boost.Geometry

// Copyright (c) 2021, Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_STRATEGIES_SIMPLIFY_CARTESIAN_HPP
#define BOOST_GEOMETRY_STRATEGIES_SIMPLIFY_CARTESIAN_HPP


#include <boost/geometry/strategies/agnostic/simplify_douglas_peucker.hpp>
#include <boost/geometry/strategies/cartesian/distance_projected_point.hpp>
#include <boost/geometry/strategies/cartesian/distance_pythagoras.hpp>
#include <boost/geometry/strategies/cartesian/point_in_point.hpp>

#include <boost/geometry/strategies/detail.hpp>
#include <boost/geometry/strategies/simplify/services.hpp>

#include <boost/geometry/util/type_traits.hpp>


namespace boost { namespace geometry
{

namespace strategies { namespace simplify
{

template <typename CalculationType = void>
struct cartesian
    : public strategies::detail::cartesian_base
{
    template <typename Geometry1, typename Geometry2>
    static auto distance(Geometry1 const&, Geometry2 const&,
                         std::enable_if_t
                            <
                                util::is_pointlike<Geometry1>::value
                                    && util::is_segmental<Geometry2>::value
                             || util::is_segmental<Geometry1>::value
                                    && util::is_pointlike<Geometry2>::value
                             || util::is_segmental<Geometry1>::value
                                    && util::is_segmental<Geometry2>::value
                            > * = nullptr)
    {
        return strategy::distance::projected_point
            <
                CalculationType,
                strategy::distance::pythagoras<CalculationType>
            >();
    }

    template <typename Geometry1, typename Geometry2>
    static auto relate(Geometry1 const&, Geometry2 const&,
                       std::enable_if_t
                            <
                                util::is_pointlike<Geometry1>::value
                             && util::is_pointlike<Geometry2>::value
                            > * = nullptr)
    {
        return strategy::within::cartesian_point_point();
    }
};


namespace services
{

template <typename Geometry>
struct default_strategy<Geometry, cartesian_tag>
{
    using type = strategies::simplify::cartesian<>;
};


template <typename P, typename CT, typename S>
struct strategy_converter
    <
        strategy::simplify::douglas_peucker
            <
                P,
                strategy::distance::projected_point<CT, S>
            >
    >
{
    template <typename Strategy>
    static auto get(Strategy const& )
    {
        return strategies::simplify::cartesian<CT>();
    }
};


} // namespace services

}} // namespace strategies::simplify

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_SIMPLIFY_CARTESIAN_HPP
