// Boost.Geometry

// Copyright (c) 2020, Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_EXTENSIONS_NSPHERE_STRATEGIES_RELATE_CARTESIAN_HPP
#define BOOST_GEOMETRY_EXTENSIONS_NSPHERE_STRATEGIES_RELATE_CARTESIAN_HPP


#include <boost/geometry/strategies/relate/cartesian.hpp>

#include <boost/geometry/extensions/nsphere/strategies/cartesian/disjoint.hpp>
#include <boost/geometry/extensions/nsphere/strategies/cartesian/nsphere_in_box.hpp>
#include <boost/geometry/extensions/nsphere/strategies/cartesian/point_in_nsphere.hpp>


namespace boost { namespace geometry
{

namespace strategies { namespace relate
{

template <typename CalculationType = void>
class cartesian_for_nsphere
    : public strategies::relate::cartesian<CalculationType>
{
public:

    // covered_by

    template <typename Geometry1, typename Geometry2>
    static auto covered_by(Geometry1 const&, Geometry2 const&,
                           std::enable_if_t
                                <
                                    util::is_pointlike<Geometry1>::value
                                 && std::is_same<nsphere_tag, typename tag<Geometry2>::type>::value
                                > * = nullptr)
    {
        return strategy::within::point_in_nsphere
            <
                Geometry1, Geometry2,
                strategy::within::point_nsphere_covered_by_comparable_distance
            >();
    }

    template <typename Geometry1, typename Geometry2>
    static auto covered_by(Geometry1 const&, Geometry2 const&,
                           std::enable_if_t
                                <
                                    std::is_same<nsphere_tag, typename tag<Geometry1>::type>::value
                                 && util::is_box<Geometry2>::value
                                > * = nullptr)
    {
        return strategy::within::nsphere_in_box
            <
                Geometry1, Geometry2,
                strategy::within::nsphere_covered_by_range
            >();
    }

    // within

    template <typename Geometry1, typename Geometry2>
    static auto within(Geometry1 const&, Geometry2 const&,
                       std::enable_if_t
                            <
                                util::is_pointlike<Geometry1>::value
                                && std::is_same<nsphere_tag, typename tag<Geometry2>::type>::value
                            > * = nullptr)
    {
        return strategy::within::point_in_nsphere
            <
                Geometry1, Geometry2,
                strategy::within::point_nsphere_within_comparable_distance
            >();
    }

    template <typename Geometry1, typename Geometry2>
    static auto within(Geometry1 const&, Geometry2 const&,
                       std::enable_if_t
                            <
                                std::is_same<nsphere_tag, typename tag<Geometry1>::type>::value
                                && util::is_box<Geometry2>::value
                            > * = nullptr)
    {
        return strategy::within::nsphere_in_box
            <
                Geometry1, Geometry2,
                strategy::within::nsphere_within_range
            >();
    }
};


namespace services
{

template <typename P, typename T, typename Geometry2>
struct default_strategy<model::nsphere<P, T>, Geometry2, cartesian_tag, cartesian_tag>
{
    using type = strategies::relate::cartesian_for_nsphere<>;
};

template <typename Geometry1, typename P, typename T>
struct default_strategy<Geometry1, model::nsphere<P, T>, cartesian_tag, cartesian_tag>
{
    using type = strategies::relate::cartesian_for_nsphere<>;
};

template <typename P1, typename T1, typename P2, typename T2>
struct default_strategy<model::nsphere<P1, T1>, model::nsphere<P2, T2>, cartesian_tag, cartesian_tag>
{
    using type = strategies::relate::cartesian_for_nsphere<>;
};


} // namespace services

}} // namespace strategies::relate

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_EXTENSIONS_NSPHERE_STRATEGIES_RELATE_CARTESIAN_HPP
