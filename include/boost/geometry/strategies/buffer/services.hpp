// Boost.Geometry

// Copyright (c) 2021, Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_STRATEGIES_BUFFER_SERVICES_HPP
#define BOOST_GEOMETRY_STRATEGIES_BUFFER_SERVICES_HPP


#include <type_traits>

#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/core/static_assert.hpp>


namespace boost { namespace geometry
{

namespace strategy { namespace buffer
{
template <typename NumericType> class distance_symmetric;
template <typename NumericType> class distance_asymmetric;
}}

namespace strategies { namespace buffer
{

namespace services
{

template
<
    typename Geometry,
    typename CSTag = geometry::cs_tag_t<Geometry>
>
struct default_strategy
{
    BOOST_GEOMETRY_STATIC_ASSERT_FALSE(
        "Not implemented for this Geometry's coordinate systems.",
        Geometry, CSTag);
};

template <typename Distance>
struct is_constant_distance : std::false_type {};

template <typename T>
struct is_constant_distance<strategy::buffer::distance_symmetric<T>> : std::true_type {};

template <typename T>
struct is_constant_distance<strategy::buffer::distance_asymmetric<T>> : std::true_type {};


} // namespace services

namespace detail
{
template <typename Strategy>
auto generation_strategy(Strategy const& strategy, int) -> decltype(strategy.buffer_generation_strategy())
{ return strategy.buffer_generation_strategy(); }

template <typename Strategy>
Strategy const& generation_strategy(Strategy const& strategy, long)
{ return strategy; }

template <typename Rings, typename Strategy, typename Apply>
auto with_piece_strategy(Strategy const& strategy, Apply const& apply, int)
    -> decltype(strategy.template with_buffer_piece_strategy<Rings>(apply))
{ return strategy.template with_buffer_piece_strategy<Rings>(apply); }

template <typename Rings, typename Strategy, typename Apply>
void with_piece_strategy(Strategy const& strategy, Apply const& apply, long)
{ apply(strategy); }
}

}} // namespace strategies::buffer

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_BUFFER_SERVICES_HPP
