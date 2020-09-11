// Boost.Geometry

// Copyright (c) 2020, Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_STRATEGIES_ENVELOPE_CARTESIAN_HPP
#define BOOST_GEOMETRY_STRATEGIES_ENVELOPE_CARTESIAN_HPP


#include <type_traits>

#include <boost/geometry/strategy/cartesian/envelope.hpp>
#include <boost/geometry/strategy/cartesian/envelope_box.hpp>
#include <boost/geometry/strategy/cartesian/envelope_point.hpp>
#include <boost/geometry/strategy/cartesian/envelope_multipoint.hpp>
#include <boost/geometry/strategy/cartesian/envelope_segment.hpp>

#include <boost/geometry/strategy/cartesian/expand_box.hpp> // TEMP
#include <boost/geometry/strategy/cartesian/expand_point.hpp>
#include <boost/geometry/strategy/cartesian/expand_segment.hpp> // TEMP

#include <boost/geometry/strategies/detail.hpp>
#include <boost/geometry/strategies/envelope/services.hpp>


namespace boost { namespace geometry
{

namespace strategies { namespace envelope
{

template <typename CalculationType = void>
struct cartesian : strategies::detail::cartesian_base
{
    template <typename Geometry, typename Box>
    static auto envelope(Geometry const&, Box const&,
                         typename strategies::detail::enable_if_point<Geometry>::type * = nullptr)
    {
        return strategy::envelope::cartesian_point();
    }

    template <typename Geometry, typename Box>
    static auto envelope(Geometry const&, Box const&,
                         typename strategies::detail::enable_if_multi_point<Geometry>::type * = nullptr)
    {
        return strategy::envelope::cartesian_multipoint();
    }

    template <typename Geometry, typename Box>
    static auto envelope(Geometry const&, Box const&,
                         typename strategies::detail::enable_if_box<Geometry>::type * = nullptr)
    {
        return strategy::envelope::cartesian_box();
    }

    template <typename Geometry, typename Box>
    static auto envelope(Geometry const&, Box const&,
                         typename strategies::detail::enable_if_segment<Geometry>::type * = nullptr)
    {
        return strategy::envelope::cartesian_segment<CalculationType>();
    }

    template <typename Geometry, typename Box>
    static auto envelope(Geometry const&, Box const&,
                         typename strategies::detail::enable_if_non_trivial_linear_or_polygonal<Geometry>::type * = nullptr)
    {
        return strategy::envelope::cartesian<CalculationType>();
    }

    template <typename Box, typename Geometry>
    static auto expand(Box const&, Geometry const&,
                       typename strategies::detail::enable_if_point<Geometry>::type * = nullptr)
    {
        return strategy::expand::cartesian_point();
    }

    // TEMP
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


namespace services
{

template <typename Geometry, typename Box>
struct default_strategy<Geometry, Box, cartesian_tag>
{
    using type = strategies::envelope::cartesian<>;
};


template <>
struct strategy_converter<strategy::envelope::cartesian_point>
{
    static auto get(strategy::envelope::cartesian_point const& )
    {
        return strategies::envelope::cartesian<>();
    }
};

template <>
struct strategy_converter<strategy::envelope::cartesian_multipoint>
{
    static auto get(strategy::envelope::cartesian_multipoint const&)
    {
        return strategies::envelope::cartesian<>();
    }
};

template <>
struct strategy_converter<strategy::envelope::cartesian_box>
{
    static auto get(strategy::envelope::cartesian_box const& )
    {
        return strategies::envelope::cartesian<>();
    }
};

template <typename CT>
struct strategy_converter<strategy::envelope::cartesian_segment<CT> >
{
    static auto get(strategy::envelope::cartesian_segment<CT> const&)
    {
        return strategies::envelope::cartesian<CT>();
    }
};

template <typename CT>
struct strategy_converter<strategy::envelope::cartesian<CT> >
{
    static auto get(strategy::envelope::cartesian<CT> const&)
    {
        return strategies::envelope::cartesian<CT>();
    }
};


} // namespace services

}} // namespace strategies::envelope

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_ENVELOPE_CARTESIAN_HPP
