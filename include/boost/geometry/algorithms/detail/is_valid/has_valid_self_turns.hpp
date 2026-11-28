// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014-2024, Oracle and/or its affiliates.
// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle
// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_IS_VALID_HAS_VALID_SELF_TURNS_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_IS_VALID_HAS_VALID_SELF_TURNS_HPP

#include <boost/geometry/strategies/detail/turn_container.hpp>

#include <vector>

#include <boost/core/ignore_unused.hpp>
#include <boost/range/empty.hpp>

#include <boost/geometry/algorithms/detail/is_valid/is_acceptable_turn.hpp>
#include <boost/geometry/algorithms/detail/overlay/get_turn_info.hpp>
#include <boost/geometry/algorithms/detail/overlay/turn_info.hpp>
#include <boost/geometry/algorithms/detail/overlay/self_turn_points.hpp>
#include <boost/geometry/algorithms/validity_failure_type.hpp>

#include <boost/geometry/core/assert.hpp>
#include <boost/geometry/core/point_type.hpp>

#include <boost/geometry/policies/predicate_based_interrupt_policy.hpp>
#include <boost/geometry/strategies/detail/turn_container.hpp>
#include <boost/geometry/strategies/relate/intersection_types.hpp>

namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace is_valid
{


template <typename Geometry, typename Strategy>
class has_valid_self_turns
{
    using point_type = typename strategies::relate::services::intersection_point_type
        <Strategy, point_type_t<Geometry>>::type;

public:
    using turn_type = detail::overlay::turn_info
        <
            point_type,
            typename strategies::relate::services::segment_ratio_type<Strategy, point_type>::type
        >;

    // returns true if all turns are valid
    template <typename Turns, typename VisitPolicy>
    static inline bool apply(Geometry const& geometry,
                             Turns& turns,
                             VisitPolicy& visitor,
                             Strategy const& strategy)
    {
        boost::ignore_unused(visitor);

        detail::overlay::stateless_predicate_based_interrupt_policy
            <
                is_acceptable_turn<Geometry>
            > interrupt_policy;

        // Calculate self-turns, skipping adjacent segments
        detail::self_get_turn_points::self_turns
            <
                false, detail::overlay::assign_null_policy
            >(geometry, strategy, turns, interrupt_policy,
              0, true);

        if (interrupt_policy.has_intersections)
        {
            BOOST_GEOMETRY_ASSERT(! boost::empty(turns));
            return visitor.template apply<failure_self_intersections>(turns);
        }
        else
        {
            return visitor.template apply<no_failure>();
        }
    }

    // returns true if all turns are valid
    template <typename VisitPolicy>
    static inline bool apply(Geometry const& geometry, VisitPolicy& visitor, Strategy const& strategy)
    {
        auto turns = strategies::detail::make_turn_container<std::vector<turn_type>>
            (strategy, geometry, geometry);
        return apply(geometry, turns, visitor, strategy);
    }
};


}} // namespace detail::is_valid
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_IS_VALID_HAS_VALID_SELF_TURNS_HPP
