// Boost.Geometry

// Copyright (c) 2021, Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_STRATEGIES_SIMPLIFY_GEOGRAPHIC_HPP
#define BOOST_GEOMETRY_STRATEGIES_SIMPLIFY_GEOGRAPHIC_HPP


#include <boost/geometry/strategies/detail.hpp>
#include <boost/geometry/strategies/simplify/services.hpp>

#include <boost/geometry/strategies/agnostic/simplify_douglas_peucker.hpp>
#include <boost/geometry/strategies/geographic/distance_cross_track.hpp>
#include <boost/geometry/strategies/spherical/point_in_point.hpp>


namespace boost { namespace geometry
{

namespace strategies { namespace simplify
{

template
<
    typename FormulaPolicy = strategy::andoyer,
    typename Spheroid = srs::spheroid<double>,
    typename CalculationType = void
>
class geographic
    : public strategies::detail::geographic_base<Spheroid>
{
    using base_t = strategies::detail::geographic_base<Spheroid>;

public:
    geographic() = default;

    explicit geographic(Spheroid const& spheroid)
        : base_t(spheroid)
    {}

    template <typename Geometry1, typename Geometry2>
    auto distance(Geometry1 const&, Geometry2 const&,
                  std::enable_if_t
                  <
                        util::is_pointlike<Geometry1>::value
                            && util::is_segmental<Geometry2>::value
                     || util::is_segmental<Geometry1>::value
                            && util::is_pointlike<Geometry2>::value
                     || util::is_segmental<Geometry1>::value
                            && util::is_segmental<Geometry2>::value
                  > * = nullptr) const
    {
        return strategy::distance::geographic_cross_track
            <
                FormulaPolicy, Spheroid, CalculationType
            >(base_t::m_spheroid);
    }

    template <typename Geometry1, typename Geometry2>
    static auto relate(Geometry1 const&, Geometry2 const&,
                       std::enable_if_t
                            <
                                util::is_pointlike<Geometry1>::value
                             && util::is_pointlike<Geometry2>::value
                            > * = nullptr)
    {
        return strategy::within::spherical_point_point();
    }
};


namespace services
{

template <typename Geometry>
struct default_strategy<Geometry, geographic_tag>
{
    using type = strategies::simplify::geographic<>;
};


template <typename P, typename FP, typename S, typename CT>
struct strategy_converter
    <
        strategy::simplify::douglas_peucker
            <
                P,
                strategy::distance::geographic_cross_track<FP, S, CT>
            >
    >
{
    template <typename Strategy>
    static auto get(Strategy const& )
    {
        return strategies::simplify::geographic<FP, S, CT>();
    }
};


} // namespace services

}} // namespace strategies::simplify

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_SIMPLIFY_GEOGRAPHIC_HPP
