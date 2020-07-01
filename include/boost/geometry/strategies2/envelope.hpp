// Boost.Geometry

// Copyright (c) 2020, Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_STRATEGIES2_ENVELOPE_HPP
#define BOOST_GEOMETRY_STRATEGIES2_ENVELOPE_HPP


#include <type_traits>

#include <boost/geometry/strategies/envelope.hpp>
#include <boost/geometry/strategies/cartesian/envelope.hpp>
#include <boost/geometry/strategies/cartesian/envelope_box.hpp>
#include <boost/geometry/strategies/cartesian/envelope_point.hpp>
#include <boost/geometry/strategies/cartesian/envelope_multipoint.hpp>
#include <boost/geometry/strategies/cartesian/envelope_segment.hpp>
#include <boost/geometry/strategies/geographic/envelope.hpp>
#include <boost/geometry/strategies/geographic/envelope_segment.hpp>
#include <boost/geometry/strategies/spherical/envelope.hpp>
#include <boost/geometry/strategies/spherical/envelope_box.hpp>
#include <boost/geometry/strategies/spherical/envelope_point.hpp>
#include <boost/geometry/strategies/spherical/envelope_multipoint.hpp>
#include <boost/geometry/strategies/spherical/envelope_segment.hpp>


#include <boost/geometry/strategies2/base.hpp>


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


template
<
    typename CalculationType = void
>
class spherical : strategies::detail::spherical_base<void>
{
    using base_t = strategies::detail::spherical_base<void>;

public:
    template <typename Geometry, typename Box>
    static auto envelope(Geometry const&, Box const&,
                         typename strategies::detail::enable_if_point<Geometry>::type * = nullptr)
    {
        return strategy::envelope::spherical_point();
    }

    template <typename Geometry, typename Box>
    static auto envelope(Geometry const&, Box const&,
                         typename strategies::detail::enable_if_multi_point<Geometry>::type * = nullptr)
    {
        return strategy::envelope::spherical_multipoint();
    }

    template <typename Geometry, typename Box>
    static auto envelope(Geometry const&, Box const&,
                         typename strategies::detail::enable_if_box<Geometry>::type * = nullptr)
    {
        return strategy::envelope::spherical_box();
    }

    template <typename Geometry, typename Box>
    static auto envelope(Geometry const&, Box const&,
                         typename strategies::detail::enable_if_segment<Geometry>::type * = nullptr)
    {
        return strategy::envelope::spherical_segment<CalculationType>();
    }

    template <typename Geometry, typename Box>
    static auto envelope(Geometry const&, Box const&,
                         typename strategies::detail::enable_if_non_trivial_linear_or_polygonal<Geometry>::type * = nullptr)
    {
        return strategy::envelope::spherical<CalculationType>();
    }

    template <typename Box, typename Geometry>
    static auto expand(Box const&, Geometry const&,
                       typename strategies::detail::enable_if_point<Geometry>::type * = nullptr)
    {
        return strategy::expand::spherical_point();
    }

    // TEMP
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

    template <typename Geometry, typename Box>
    static auto envelope(Geometry const&, Box const&,
                         typename strategies::detail::enable_if_point<Geometry>::type * = nullptr)
    {
        return strategy::envelope::spherical_point();
    }

    template <typename Geometry, typename Box>
    static auto envelope(Geometry const&, Box const&,
                         typename strategies::detail::enable_if_multi_point<Geometry>::type * = nullptr)
    {
        return strategy::envelope::spherical_multipoint();
    }

    template <typename Geometry, typename Box>
    static auto envelope(Geometry const&, Box const&,
                         typename strategies::detail::enable_if_box<Geometry>::type * = nullptr)
    {
        return strategy::envelope::spherical_box();
    }

    template <typename Geometry, typename Box>
    auto envelope(Geometry const&, Box const&,
                  typename detail::enable_if_segment<Geometry>::type * = nullptr) const
    {
        return strategy::envelope::geographic_segment
            <
                FormulaPolicy, Spheroid, CalculationType
            >(base_t::m_spheroid);
    }

    template <typename Geometry, typename Box>
    auto envelope(Geometry const&, Box const&,
                  typename detail::enable_if_non_trivial_linear_or_polygonal<Geometry>::type * = nullptr) const
    {
        return strategy::envelope::geographic
            <
                FormulaPolicy, Spheroid, CalculationType
            >(base_t::m_spheroid);
    }

    template <typename Box, typename Geometry>
    static auto expand(Box const&, Geometry const&,
                       typename strategies::detail::enable_if_point<Geometry>::type * = nullptr)
    {
        return strategy::expand::spherical_point();
    }

    // TEMP
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

template <typename Geometry, typename Box>
struct default_strategy<Geometry, Box, cartesian_tag>
{
    using type = strategies::envelope::cartesian<>;
};

template <typename Geometry, typename Box>
struct default_strategy<Geometry, Box, spherical_tag>
{
    using type = strategies::envelope::spherical<>;
};

template <typename Geometry, typename Box>
struct default_strategy<Geometry, Box, spherical_equatorial_tag>
{
    using type = strategies::envelope::spherical<>;
};

template <typename Geometry, typename Box>
struct default_strategy<Geometry, Box, spherical_polar_tag>
{
    using type = strategies::envelope::spherical<>;
};

template <typename Geometry, typename Box>
struct default_strategy<Geometry, Box, geographic_tag>
{
    using type = strategies::envelope::geographic<>;
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

template <>
struct strategy_converter<strategy::envelope::spherical_point>
{
    static auto get(strategy::envelope::spherical_point const& )
    {
        return strategies::envelope::spherical<>();
    }
};

template <>
struct strategy_converter<strategy::envelope::spherical_multipoint>
{
    static auto get(strategy::envelope::spherical_multipoint const&)
    {
        return strategies::envelope::spherical<>();
    }
};

template <>
struct strategy_converter<strategy::envelope::spherical_box>
{
    static auto get(strategy::envelope::spherical_box const& )
    {
        return strategies::envelope::spherical<>();
    }
};

template <typename CT>
struct strategy_converter<strategy::envelope::spherical_segment<CT> >
{
    static auto get(strategy::envelope::spherical_segment<CT> const&)
    {
        return strategies::envelope::spherical<CT>();
    }
};

template <typename CT>
struct strategy_converter<strategy::envelope::spherical<CT> >
{
    static auto get(strategy::envelope::spherical<CT> const&)
    {
        return strategies::envelope::spherical<CT>();
    }
};

template <typename FP, typename S, typename CT>
struct strategy_converter<strategy::envelope::geographic_segment<FP, S, CT> >
{
    static auto get(strategy::envelope::geographic_segment<FP, S, CT> const& s)
    {
        return strategies::envelope::geographic<FP, S, CT>(s.model());
    }
};

template <typename FP, typename S, typename CT>
struct strategy_converter<strategy::envelope::geographic<FP, S, CT> >
{
    static auto get(strategy::envelope::geographic<FP, S, CT> const& s)
    {
        return strategies::envelope::geographic<FP, S, CT>(s.model());
    }
};

} // namespace services

}} // namespace strategies::envelope

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES2_ENVELOPE_HPP
