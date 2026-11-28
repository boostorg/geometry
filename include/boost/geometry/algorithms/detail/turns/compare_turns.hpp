// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014-2015, Oracle and/or its affiliates.

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle


#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_TURNS_COMPARE_TURNS_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_TURNS_COMPARE_TURNS_HPP

#include <cstddef>
#include <functional>
#include <type_traits>

#include <boost/geometry/strategies/default_strategy.hpp>
#include <boost/geometry/strategies/detail/topology.hpp>
#include <boost/geometry/algorithms/detail/overlay/turn_info.hpp>
#include <boost/geometry/algorithms/detail/overlay/copy_segment_point.hpp>
#include <boost/geometry/algorithms/detail/overlay/do_reverse.hpp>
#include <boost/geometry/algorithms/detail/sub_range.hpp>


namespace boost { namespace geometry
{

namespace detail { namespace turns
{

// TURNS SORTING AND SEARCHING

template <bool Reverse1, bool Reverse2, typename Endpoint, typename Geometry1,
          typename Geometry2, typename Point, typename Strategy>
int compare_on_segment(Geometry1 const& a, Geometry2 const& b, segment_identifier const& id,
                       Point const& left, Point const& right, Strategy const& strategy)
{
    Endpoint first, last;
    geometry::copy_segment_points<Reverse1, Reverse2>(a, b, id, first, last);
    return strategy.segment_position().apply(first, last, left, right);
}

template <std::size_t OpId, typename Turn, typename Geometry, typename Strategy>
int compare_positions(Turn const& left, Turn const& right, Geometry const& geometry, Strategy const& strategy)
{
    auto const& id = left.operations[OpId].seg_id;
    auto const& source = detail::sub_range(geometry, id);
    using range_type = std::remove_reference_t<decltype(source)>;
    detail::closed_clockwise_view<range_type, geometry::closure<range_type>::value,
        geometry::point_order<Geometry>::value> view(source);
    return strategy.segment_position().apply(range::at(view, id.segment_index),
        range::at(view, id.segment_index + 1), left.point, right.point);
}

// sort turns by G1 - source_index == 0 by:
// seg_id -> fraction -> other_id -> operation
template
<
    typename IdLess = std::less<signed_size_type>,
    int N = 0, int U = 1, int I = 2, int B = 3, int C = 4, int O = 0,
    std::size_t OpId = 0,
    typename Strategy = default_strategy
>
struct less_seg_fraction_other_op
{
    static_assert(OpId < 2, "Only 0 and 1 are valid OpIds.");
    static const std::size_t other_op_id = (OpId + 1) % 2;

    explicit less_seg_fraction_other_op(Strategy const& strategy = Strategy())
        : m_strategy(strategy)
    {}

    template <typename Op>
    static inline int order_op(Op const& op)
    {
        switch (op.operation)
        {
        case detail::overlay::operation_none : return N;
        case detail::overlay::operation_union : return U;
        case detail::overlay::operation_intersection : return I;
        case detail::overlay::operation_blocked : return B;
        case detail::overlay::operation_continue : return C;
        case detail::overlay::operation_opposite : return O;
        }
        return -1;
    }

    template <typename Op>
    static inline bool use_operation(Op const& left, Op const& right)
    {
        return order_op(left) < order_op(right);
    }

    template <typename Turn>
    static inline bool use_other_id(Turn const& left, Turn const& right)
    {
        segment_identifier const& left_other_seg_id = left.operations[other_op_id].seg_id;
        segment_identifier const& right_other_seg_id = right.operations[other_op_id].seg_id;

        if ( left_other_seg_id.multi_index != right_other_seg_id.multi_index )
        {
            return left_other_seg_id.multi_index < right_other_seg_id.multi_index;
        }
        if ( left_other_seg_id.ring_index != right_other_seg_id.ring_index )
        {
            return left_other_seg_id.ring_index != right_other_seg_id.ring_index;
        }
        if ( left_other_seg_id.segment_index != right_other_seg_id.segment_index )
        {
            return IdLess()(left_other_seg_id.segment_index,
                            right_other_seg_id.segment_index);
        }
        return use_operation(left.operations[OpId], right.operations[OpId]);
    }

    template <typename Turn>
    inline bool use_fraction(Turn const& left, Turn const& right) const
    {
        return
            strategies::detail::get_topology(m_strategy).same_segment_position(
                left.operations[OpId].fraction, right.operations[OpId].fraction)
            ?
            use_other_id(left, right)
            :
            (left.operations[OpId].fraction < right.operations[OpId].fraction)
            ;
    }

    template <typename Turn>
    inline bool operator()(Turn const& left, Turn const& right) const
    {
        segment_identifier const& sl = left.operations[OpId].seg_id;
        segment_identifier const& sr = right.operations[OpId].seg_id;

        return sl < sr || ( sl == sr && use_fraction(left, right) );
    }

    template <typename Turn, typename Geometry>
    bool use_position(Turn const& left, Turn const& right, Geometry const&, std::true_type) const
    { return use_fraction(left, right); }

    template <typename Turn, typename Geometry>
    bool use_position(Turn const& left, Turn const& right, Geometry const& geometry, std::false_type) const
    {
        int const order = geometry::detail::turns::compare_positions<OpId>(left, right, geometry, m_strategy);
        return order == 0 ? use_other_id(left, right) : order < 0;
    }

    template <typename Geometry>
    auto on_geometry(Geometry const& geometry) const
    {
        auto const comparator = *this;
        return [comparator, &geometry](auto const& left, auto const& right)
        {
            auto const& sl = left.operations[OpId].seg_id;
            auto const& sr = right.operations[OpId].seg_id;
            using turn = typename std::decay<decltype(left)>::type;
            return sl < sr || (sl == sr && comparator.use_position(left, right, geometry,
                overlay::stores_fraction<typename turn::segment_ratio_type>()));
        };
    }

private:
    Strategy m_strategy;
};

template <typename Strategy>
inline auto make_less_seg_fraction_other_op(Strategy const& strategy)
{
    return less_seg_fraction_other_op<std::less<signed_size_type>, 0, 1, 2, 3, 4, 0, 0, Strategy>(strategy);
}

template <typename Strategy, typename Geometry>
inline auto make_less_seg_fraction_other_op(Strategy const& strategy, Geometry const& geometry)
{
    return make_less_seg_fraction_other_op(strategy).on_geometry(geometry);
}





}} // namespace detail::turns

}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_TURNS_COMPARE_TURNS_HPP
