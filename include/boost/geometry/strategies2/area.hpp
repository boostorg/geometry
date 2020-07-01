// Boost.Geometry

// Copyright (c) 2020, Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_STRATEGIES2_AREA_HPP
#define BOOST_GEOMETRY_STRATEGIES2_AREA_HPP


#include <boost/geometry/strategies/area.hpp>
#include <boost/geometry/strategies/cartesian/area.hpp>
#include <boost/geometry/strategies/geographic/area.hpp>
#include <boost/geometry/strategies/spherical/area.hpp>

#include <boost/geometry/strategies2/base.hpp>


namespace boost { namespace geometry
{


namespace strategies { namespace area
{


template <typename CalculationType = void>
struct cartesian : strategies::detail::cartesian_base
{
    template <typename Geometry>
    static auto area(Geometry const&)
    {
        return strategy::area::cartesian<CalculationType>();
    }
};


template
<
    typename RadiusTypeOrSphere = double,
    typename CalculationType = void
>
class spherical : strategies::detail::spherical_base<RadiusTypeOrSphere>
{
    using base_t = strategies::detail::spherical_base<RadiusTypeOrSphere>;

public:
    spherical()
        : base_t()
    {}

    template <typename RadiusOrSphere>
    explicit spherical(RadiusOrSphere const& radius_or_sphere)
        : base_t(radius_or_sphere)
    {}

    template <typename Geometry>
    auto area(Geometry const&) const
    {
        return strategy::area::spherical
            <
                typename base_t::radius_type, CalculationType
            >(base_t::m_radius);
    }
};

template
<
    typename FormulaPolicy = strategy::andoyer,
    std::size_t SeriesOrder = strategy::default_order<FormulaPolicy>::value,
    typename Spheroid = srs::spheroid<double>,
    typename CalculationType = void
>
class geographic : strategies::detail::geographic_base<Spheroid>
{
    using base_t = strategies::detail::geographic_base<Spheroid>;

public:
    geographic()
        : base_t()
    {}

    explicit geographic(Spheroid const& spheroid)
        : base_t(spheroid)
    {}

    template <typename Geometry>
    auto area(Geometry const&) const
    {
        return strategy::area::geographic
            <
                FormulaPolicy, SeriesOrder, Spheroid, CalculationType
            >(base_t::m_spheroid);
    }
};


namespace services
{

template <typename Geometry>
struct default_strategy<Geometry, cartesian_tag>
{
    using type = strategies::area::cartesian<>;
};

template <typename Geometry>
struct default_strategy<Geometry, spherical_tag>
{
    using type = strategies::area::spherical<>;
};

template <typename Geometry>
struct default_strategy<Geometry, spherical_equatorial_tag>
{
    using type = strategies::area::spherical<>;
};

template <typename Geometry>
struct default_strategy<Geometry, spherical_polar_tag>
{
    using type = strategies::area::spherical<>;
};

template <typename Geometry>
struct default_strategy<Geometry, geographic_tag>
{
    using type = strategies::area::geographic<>;
};


template <typename CT>
struct strategy_converter<strategy::area::cartesian<CT> >
{
    static auto get(strategy::area::cartesian<CT> const& strategy)
    {
        return strategies::area::cartesian<CT>();
    }
};

template <typename R, typename CT>
struct strategy_converter<strategy::area::spherical<R, CT> >
{
    static auto get(strategy::area::spherical<R, CT> const& strategy)
    {
        return strategies::area::spherical<R, CT>(strategy.model());
    }
};

template <typename FP, std::size_t SO, typename S, typename CT>
struct strategy_converter<strategy::area::geographic<FP, SO, S, CT> >
{
    static auto get(strategy::area::geographic<FP, SO, S, CT> const& strategy)
    {
        return strategies::area::geographic<FP, SO, S, CT>(strategy.model());
    }
};

} // namespace services

}} // namespace strategies::area

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES2_AREA_HPP
