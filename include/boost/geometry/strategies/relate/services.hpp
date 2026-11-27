// Boost.Geometry

// Copyright (c) 2020, Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_STRATEGIES_RELATE_SERVICES_HPP
#define BOOST_GEOMETRY_STRATEGIES_RELATE_SERVICES_HPP


#include <type_traits>

#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/core/static_assert.hpp>


namespace boost { namespace geometry
{


namespace strategies { namespace relate {

namespace services
{

template
<
    typename Geometry1,
    typename Geometry2,
    typename CSTag1 = geometry::cs_tag_t<Geometry1>,
    typename CSTag2 = geometry::cs_tag_t<Geometry2>
>
struct default_strategy
{
    BOOST_GEOMETRY_STATIC_ASSERT_FALSE(
        "Not implemented for this Geometry's coordinate system.",
        Geometry1, Geometry2, CSTag1, CSTag2);
};

template <typename Strategy>
struct strategy_converter
{
    BOOST_GEOMETRY_STATIC_ASSERT_FALSE(
        "Not implemented for this Strategy.",
        Strategy);
};

// A strategy may supply a kernel that needs no public coordinate output.
template <typename Strategy>
struct predicate_strategy
{
    template <typename A, typename B>
    static Strategy const& get(Strategy const& strategy, A const&, B const&) { return strategy; }
};

// Opt in ranges of boundary rings addressed by segment_identifier::multi_index.
template <typename Tag>
struct indexed_ring_collection_tag : std::false_type {};

} // namespace services

}} // namespace strategies::relate


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_RELATE_SERVICES_HPP
