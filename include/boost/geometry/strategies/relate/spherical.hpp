// Boost.Geometry

// Copyright (c) 2020, Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_STRATEGIES_RELATE_SPHERICAL_HPP
#define BOOST_GEOMETRY_STRATEGIES_RELATE_SPHERICAL_HPP


// TEMP - move to strategy
#include <boost/geometry/strategies/agnostic/point_in_box_by_side.hpp>
#include <boost/geometry/strategies/cartesian/box_in_box.hpp>
#include <boost/geometry/strategies/spherical/intersection.hpp>
#include <boost/geometry/strategies/spherical/point_in_point.hpp>
#include <boost/geometry/strategies/spherical/point_in_poly_winding.hpp>
#include <boost/geometry/strategies/spherical/disjoint_box_box.hpp>

#include <boost/geometry/strategies/relate/services.hpp>
#include <boost/geometry/strategies/detail.hpp>

#include <boost/geometry/strategy/spherical/area.hpp>
#include <boost/geometry/strategy/spherical/envelope.hpp>
#include <boost/geometry/strategy/spherical/envelope_point.hpp>
#include <boost/geometry/strategy/spherical/envelope_multipoint.hpp>
#include <boost/geometry/strategy/spherical/envelope_segment.hpp>
#include <boost/geometry/strategy/spherical/expand_box.hpp>
#include <boost/geometry/strategy/spherical/expand_point.hpp>
#include <boost/geometry/strategy/spherical/expand_segment.hpp>

#include <boost/geometry/util/type_traits.hpp>


namespace boost { namespace geometry
{

namespace strategies { namespace relate
{


template <typename CalculationType = void>
class spherical
    : public strategies::detail::spherical_base<void>
{
    using base_t = strategies::detail::spherical_base<void>;

public:
    // area

    template <typename Geometry>
    auto area(Geometry const&) const
    {
        return strategy::area::spherical<double, CalculationType>(1.0);
    }

    // covered_by

    template <typename Geometry1, typename Geometry2>
    static auto covered_by(Geometry1 const&, Geometry2 const&,
                           std::enable_if_t
                                <
                                    util::is_pointlike<Geometry1>::value
                                 && util::is_box<Geometry2>::value
                                > * = nullptr)
    {
        return strategy::covered_by::spherical_point_box();
    }

    template <typename Geometry1, typename Geometry2>
    static auto covered_by(Geometry1 const&, Geometry2 const&,
                           std::enable_if_t
                            <
                                util::is_box<Geometry1>::value
                             && util::is_box<Geometry2>::value
                            > * = nullptr)
    {
        return strategy::covered_by::spherical_box_box();
    }

    // disjoint

    template <typename Geometry1, typename Geometry2>
    static auto disjoint(Geometry1 const&, Geometry2 const&,
                         std::enable_if_t
                            <
                                util::is_box<Geometry1>::value
                             && util::is_box<Geometry2>::value
                            > * = nullptr)
    {
        return strategy::disjoint::spherical_box_box();
    }

    template <typename Geometry1, typename Geometry2>
    static auto disjoint(Geometry1 const&, Geometry2 const&,
                         std::enable_if_t
                            <
                                util::is_segment<Geometry1>::value
                             && util::is_box<Geometry2>::value
                            > * = nullptr)
    {
        // NOTE: Inconsistent name.
        return strategy::disjoint::segment_box_spherical();
    }

    // envelope

    template <typename Geometry, typename Box>
    static auto envelope(Geometry const&, Box const&,
                         typename util::enable_if_point_t<Geometry> * = nullptr)
    {
        return strategy::envelope::spherical_point();
    }

    template <typename Geometry, typename Box>
    static auto envelope(Geometry const&, Box const&,
                         typename util::enable_if_multi_point_t<Geometry> * = nullptr)
    {
        return strategy::envelope::spherical_multipoint();
    }

    template <typename Geometry, typename Box>
    static auto envelope(Geometry const&, Box const&,
                         typename util::enable_if_box_t<Geometry> * = nullptr)
    {
        return strategy::envelope::spherical_box();
    }

    template <typename Geometry, typename Box>
    static auto envelope(Geometry const&, Box const&,
                         typename util::enable_if_segment_t<Geometry> * = nullptr)
    {
        return strategy::envelope::spherical_segment<CalculationType>();
    }

    template <typename Geometry, typename Box>
    static auto envelope(Geometry const&, Box const&,
                         typename util::enable_if_polysegmental_t<Geometry> * = nullptr)
    {
        return strategy::envelope::spherical<CalculationType>();
    }

    // expand

    template <typename Box, typename Geometry>
    static auto expand(Box const&, Geometry const&,
                       typename util::enable_if_point_t<Geometry> * = nullptr)
    {
        return strategy::expand::spherical_point();
    }

    template <typename Box, typename Geometry>
    static auto expand(Box const&, Geometry const&,
                       typename util::enable_if_box_t<Geometry> * = nullptr)
    {
        return strategy::expand::spherical_box();
    }

    template <typename Box, typename Geometry>
    static auto expand(Box const&, Geometry const&,
                       typename util::enable_if_segment_t<Geometry> * = nullptr)
    {
        return strategy::expand::spherical_segment<CalculationType>();
    }

    // relate

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

    template <typename Geometry1, typename Geometry2>
    static auto relate(Geometry1 const&, Geometry2 const&,
                       std::enable_if_t
                            <
                                util::is_pointlike<Geometry1>::value
                             && ( util::is_linear<Geometry2>::value
                               || util::is_polygonal<Geometry2>::value )
                            > * = nullptr)
    {
        return strategy::within::spherical_winding<void, void, CalculationType>();
    }

    //template <typename Geometry1, typename Geometry2>
    static auto relate(/*Geometry1 const&, Geometry2 const&,
                       std::enable_if_t
                            <
                                ( util::is_linear<Geometry1>::value
                               || util::is_polygonal<Geometry1>::value )
                             && ( util::is_linear<Geometry2>::value
                               || util::is_polygonal<Geometry2>::value )
                            > * = nullptr*/)
    {
        return strategy::intersection::spherical_segments<CalculationType>();
    }

    // side

    static auto side()
    {
        return strategy::side::spherical_side_formula<CalculationType>();
    }

    // within

    template <typename Geometry1, typename Geometry2>
    static auto within(Geometry1 const&, Geometry2 const&,
                       std::enable_if_t
                            <
                                util::is_pointlike<Geometry1>::value
                                && util::is_box<Geometry2>::value
                            > * = nullptr)
    {
        return strategy::within::spherical_point_box();
    }

    template <typename Geometry1, typename Geometry2>
    static auto within(Geometry1 const&, Geometry2 const&,
                       std::enable_if_t
                            <
                                util::is_box<Geometry1>::value
                             && util::is_box<Geometry2>::value
                            > * = nullptr)
    {
        return strategy::within::spherical_box_box();
    }
};


namespace services
{

template <typename Geometry1, typename Geometry2>
struct default_strategy<Geometry1, Geometry2, spherical_tag, spherical_tag>
{
    using type = strategies::relate::spherical<>;
};

template <typename Geometry1, typename Geometry2>
struct default_strategy<Geometry1, Geometry2, spherical_equatorial_tag, spherical_equatorial_tag>
{
    using type = strategies::relate::spherical<>;
};

template <typename Geometry1, typename Geometry2>
struct default_strategy<Geometry1, Geometry2, spherical_polar_tag, spherical_polar_tag>
{
    using type = strategies::relate::spherical<>;
};


template <>
struct strategy_converter<strategy::within::spherical_point_point>
{
    static auto get(strategy::within::spherical_point_point const& )
    {
        return strategies::relate::spherical<>();
    }
};

template <>
struct strategy_converter<strategy::within::spherical_point_box>
{
    static auto get(strategy::within::spherical_point_box const&)
    {
        return strategies::relate::spherical<>();
    }
};

template <>
struct strategy_converter<strategy::covered_by::spherical_point_box>
{
    static auto get(strategy::covered_by::spherical_point_box const&)
    {
        return strategies::relate::spherical<>();
    }
};

template <>
struct strategy_converter<strategy::covered_by::spherical_box_box>
{
    static auto get(strategy::covered_by::spherical_box_box const&)
    {
        return strategies::relate::spherical<>();
    }
};

template <>
struct strategy_converter<strategy::disjoint::spherical_box_box>
{
    static auto get(strategy::disjoint::spherical_box_box const&)
    {
        return strategies::relate::spherical<>();
    }
};

template <>
struct strategy_converter<strategy::disjoint::segment_box_spherical>
{
    static auto get(strategy::disjoint::segment_box_spherical const&)
    {
        return strategies::relate::spherical<>();
    }
};

template <>
struct strategy_converter<strategy::within::spherical_box_box>
{
    static auto get(strategy::within::spherical_box_box const&)
    {
        return strategies::relate::spherical<>();
    }
};

template <typename P1, typename P2, typename CalculationType>
struct strategy_converter<strategy::within::spherical_winding<P1, P2, CalculationType>>
{
    static auto get(strategy::within::spherical_winding<P1, P2, CalculationType> const& )
    {
        return strategies::relate::spherical<CalculationType>();
    }
};

template <typename CalculationType>
struct strategy_converter<strategy::intersection::spherical_segments<CalculationType>>
{
    static auto get(strategy::intersection::spherical_segments<CalculationType> const& )
    {
        return strategies::relate::spherical<CalculationType>();
    }
};

template <typename CalculationType>
struct strategy_converter<strategy::within::spherical_point_box_by_side<CalculationType>>
{
    struct altered_strategy
        : strategies::relate::spherical<CalculationType>
    {
        template <typename Geometry1, typename Geometry2>
        static auto covered_by(Geometry1 const&, Geometry2 const&,
                               std::enable_if_t
                                    <
                                        util::is_pointlike<Geometry1>::value
                                     && util::is_box<Geometry2>::value
                                    > * = nullptr)
        {
            return strategy::covered_by::spherical_point_box_by_side<CalculationType>();
        }

        template <typename Geometry1, typename Geometry2>
        static auto within(Geometry1 const&, Geometry2 const&,
                           std::enable_if_t
                                <
                                    util::is_pointlike<Geometry1>::value
                                    && util::is_box<Geometry2>::value
                                > * = nullptr)
        {
            return strategy::within::spherical_point_box_by_side<CalculationType>();
        }
    };

    static auto get(strategy::covered_by::spherical_point_box_by_side<CalculationType> const&)
    {
        return altered_strategy();
    }

    static auto get(strategy::within::spherical_point_box_by_side<CalculationType> const&)
    {
        return altered_strategy();
    }
};

template <typename CalculationType>
struct strategy_converter<strategy::covered_by::spherical_point_box_by_side<CalculationType>>
    : strategy_converter<strategy::within::spherical_point_box_by_side<CalculationType>>
{};

// TEMP used in distance segment/box
template <typename CalculationType>
struct strategy_converter<strategy::side::spherical_side_formula<CalculationType>>
{
    static auto get(strategy::side::spherical_side_formula<CalculationType> const& )
    {
        return strategies::relate::spherical<CalculationType>();
    }
};


} // namespace services

}} // namespace strategies::relate

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_RELATE_SPHERICAL_HPP
