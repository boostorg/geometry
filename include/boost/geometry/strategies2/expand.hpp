// Boost.Geometry

// Copyright (c) 2020, Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_STRATEGIES2_EXPAND_HPP
#define BOOST_GEOMETRY_STRATEGIES2_EXPAND_HPP


#include <type_traits>

#include <boost/geometry/strategies/expand.hpp>
#include <boost/geometry/strategies/cartesian/expand_box.hpp>
#include <boost/geometry/strategies/cartesian/expand_point.hpp>
#include <boost/geometry/strategies/cartesian/expand_segment.hpp>
#include <boost/geometry/strategies/geographic/expand_segment.hpp>
#include <boost/geometry/strategies/spherical/expand_box.hpp>
#include <boost/geometry/strategies/spherical/expand_point.hpp>
#include <boost/geometry/strategies/spherical/expand_segment.hpp>


#include <boost/geometry/strategies2/base.hpp>


namespace boost { namespace geometry
{


namespace strategies { namespace expand
{


template <typename CalculationType = void>
struct cartesian : strategies::detail::cartesian_base
{
    template <typename Box, typename Geometry>
    static auto expand(Box const&, Geometry const&,
                       typename strategies::detail::enable_if_point<Geometry>::type * = nullptr)
    {
        return strategy::expand::cartesian_point();
    }

    template <typename Box, typename Geometry>
    static auto expand(Box const&, Geometry const&,
                       typename strategies::detail::enable_if_box<Geometry>::type * = nullptr)
    {
        return strategy::expand::cartesian_box();
    }

    template <typename Box, typename Geometry>
    static auto expand(Box const&, Geometry const&,
                       typename strategies::detail::enable_if_segment<Geometry>::type * = nullptr)
    {
        return strategy::expand::cartesian_segment();
    }
};


template
<
    typename CalculationType = void
>
class spherical : strategies::detail::spherical_base<void>
{
    using base_t = strategies::detail::spherical_base<void>;

public:
    template <typename Box, typename Geometry>
    static auto expand(Box const&, Geometry const&,
                       typename strategies::detail::enable_if_point<Geometry>::type * = nullptr)
    {
        return strategy::expand::spherical_point();
    }

    template <typename Box, typename Geometry>
    static auto expand(Box const&, Geometry const&,
                       typename strategies::detail::enable_if_box<Geometry>::type * = nullptr)
    {
        return strategy::expand::spherical_box();
    }

    template <typename Box, typename Geometry>
    static auto expand(Box const&, Geometry const&,
                       typename strategies::detail::enable_if_segment<Geometry>::type * = nullptr)
    {
        return strategy::expand::spherical_segment<CalculationType>();
    }
};

template
<
    typename FormulaPolicy = strategy::andoyer,
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

    template <typename Box, typename Geometry>
    static auto expand(Box const&, Geometry const&,
                       typename strategies::detail::enable_if_point<Geometry>::type * = nullptr)
    {
        return strategy::expand::spherical_point();
    }

    template <typename Box, typename Geometry>
    static auto expand(Box const&, Geometry const&,
                       typename strategies::detail::enable_if_box<Geometry>::type * = nullptr)
    {
        return strategy::expand::spherical_box();
    }

    template <typename Box, typename Geometry>
    auto expand(Box const&, Geometry const&,
                typename detail::enable_if_segment<Geometry>::type * = nullptr) const
    {
        return strategy::expand::geographic_segment
            <
                FormulaPolicy, Spheroid, CalculationType
            >(base_t::m_spheroid);
    }
};


namespace services
{

template <typename Box, typename Geometry>
struct default_strategy<Box, Geometry, cartesian_tag>
{
    using type = strategies::expand::cartesian<>;
};

template <typename Box, typename Geometry>
struct default_strategy<Box, Geometry, spherical_tag>
{
    using type = strategies::expand::spherical<>;
};

template <typename Box, typename Geometry>
struct default_strategy<Box, Geometry, spherical_equatorial_tag>
{
    using type = strategies::expand::spherical<>;
};

template <typename Box, typename Geometry>
struct default_strategy<Box, Geometry, spherical_polar_tag>
{
    using type = strategies::expand::spherical<>;
};

template <typename Box, typename Geometry>
struct default_strategy<Box, Geometry, geographic_tag>
{
    using type = strategies::expand::geographic<>;
};


template <>
struct strategy_converter<strategy::expand::cartesian_point>
{
    static auto get(strategy::expand::cartesian_point const& )
    {
        return strategies::expand::cartesian<>();
    }
};

template <>
struct strategy_converter<strategy::expand::cartesian_box>
{
    static auto get(strategy::expand::cartesian_box const& )
    {
        return strategies::expand::cartesian<>();
    }
};

template <>
struct strategy_converter<strategy::expand::cartesian_segment>
{
    static auto get(strategy::expand::cartesian_segment const&)
    {
        return strategies::expand::cartesian<>();
    }
};

template <>
struct strategy_converter<strategy::expand::spherical_point>
{
    static auto get(strategy::expand::spherical_point const& )
    {
        return strategies::expand::spherical<>();
    }
};

template <>
struct strategy_converter<strategy::expand::spherical_box>
{
    static auto get(strategy::expand::spherical_box const& )
    {
        return strategies::expand::spherical<>();
    }
};

template <typename CT>
struct strategy_converter<strategy::expand::spherical_segment<CT> >
{
    static auto get(strategy::expand::spherical_segment<CT> const&)
    {
        return strategies::expand::spherical<CT>();
    }
};

template <typename FP, typename S, typename CT>
struct strategy_converter<strategy::expand::geographic_segment<FP, S, CT> >
{
    static auto get(strategy::expand::geographic_segment<FP, S, CT> const& s)
    {
        return strategies::expand::geographic<FP, S, CT>(s.model());
    }
};


} // namespace services

}} // namespace strategies::envelope

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES2_EXPAND_HPP
