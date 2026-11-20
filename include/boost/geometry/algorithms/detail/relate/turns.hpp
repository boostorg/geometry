// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2015 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2013-2024.
// Modifications copyright (c) 2013-2024 Oracle and/or its affiliates.
// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle
// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_TURNS_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_TURNS_HPP

#include <type_traits>
#include <boost/geometry/algorithms/detail/overlay/copy_segment_point.hpp>
#include <boost/geometry/algorithms/detail/sub_range.hpp>

#include <boost/geometry/algorithms/detail/overlay/do_reverse.hpp>
#include <boost/geometry/algorithms/detail/overlay/get_turns.hpp>
#include <boost/geometry/algorithms/detail/overlay/get_turn_info.hpp>

#include <boost/geometry/geometries/helper_geometry.hpp>

#include <boost/geometry/strategies/cartesian/point_in_point.hpp>
#include <boost/geometry/strategies/detail/topology.hpp>
#include <boost/geometry/strategies/relate/intersection_types.hpp>
#include <boost/geometry/strategies/spherical/point_in_point.hpp>
#include <boost/geometry/strategies/distance.hpp>


namespace boost { namespace geometry {

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace relate { namespace turns {

template <bool IncludeDegenerate = false>
struct assign_policy
    : overlay::assign_null_policy
{
    static bool const include_degenerate = IncludeDegenerate;
};

// turn retriever, calling get_turns

template
<
    typename Geometry1,
    typename Geometry2,
    typename GetTurnPolicy = detail::get_turns::get_turn_info_type
        <
            Geometry1, Geometry2, assign_policy<>
        >
>
struct get_turns
{
    using turn_point_type = typename helper_geometry
        <
            geometry::point_type_t<Geometry1>
        >::type;

    template
    <
        typename Strategy
    >
    struct turn_info_type
    {
        using point_type = typename strategies::relate::services::intersection_point_type
            <
                Strategy, turn_point_type
            >::type;
        using ratio_type = typename strategies::relate::services::segment_ratio_type
            <
                Strategy, point_type
            >::type;
        using type = overlay::turn_info
            <
                point_type,
                ratio_type,
                typename detail::get_turns::turn_operation_type
                    <
                        Geometry1, Geometry2, point_type, ratio_type
                    >::type
            >;
    };

    template <typename Turns, typename InterruptPolicy, typename Strategy>
    static inline void apply(Turns & turns,
                             Geometry1 const& geometry1,
                             Geometry2 const& geometry2,
                             InterruptPolicy & interrupt_policy,
                             Strategy const& strategy)
    {
        static const bool reverse1 = detail::overlay::do_reverse
            <
                geometry::point_order<Geometry1>::value
            >::value;

        static const bool reverse2 = detail::overlay::do_reverse
            <
                geometry::point_order<Geometry2>::value
            >::value;

        dispatch::get_turns
            <
                geometry::tag_t<Geometry1>,
                geometry::tag_t<Geometry2>,
                Geometry1,
                Geometry2,
                reverse1,
                reverse2,
                GetTurnPolicy
            >::apply(0, geometry1, 1, geometry2,
                     strategy,
                     turns, interrupt_policy);
    }
};

// TURNS SORTING AND SEARCHING

template <int N = 0, int U = 1, int I = 2, int B = 3, int C = 4, int O = 0>
struct op_to_int
{
    template <typename Operation>
    inline int operator()(Operation const& op) const
    {
        switch(op.operation)
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
};

template <std::size_t OpId, typename OpToInt>
struct less_op_xxx_linear
{
    template <typename Turn>
    inline bool operator()(Turn const& left, Turn const& right) const
    {
        static OpToInt op_to_int;
        return op_to_int(left.operations[OpId]) < op_to_int(right.operations[OpId]);
    }
};

template <std::size_t OpId>
struct less_op_linear_linear
    : less_op_xxx_linear< OpId, op_to_int<0,2,3,1,4,0> > // xuic
{};

template <std::size_t OpId>
struct less_op_linear_areal_single
{
    template <typename Turn>
    inline bool operator()(Turn const& left, Turn const& right) const
    {
        static const std::size_t other_op_id = (OpId + 1) % 2;
        static turns::op_to_int<0,2,3,1,4,0> op_to_int_xuic;
        static turns::op_to_int<0,3,2,1,4,0> op_to_int_xiuc;

        segment_identifier const& left_other_seg_id = left.operations[other_op_id].seg_id;
        segment_identifier const& right_other_seg_id = right.operations[other_op_id].seg_id;

        typedef typename Turn::turn_operation_type operation_type;
        operation_type const& left_operation = left.operations[OpId];
        operation_type const& right_operation = right.operations[OpId];

        if ( left_other_seg_id.multi_index == right_other_seg_id.multi_index
          && left_other_seg_id.ring_index == right_other_seg_id.ring_index )
        {
            return op_to_int_xuic(left_operation)
                 < op_to_int_xuic(right_operation);
        }
        else
        {
            return op_to_int_xiuc(left_operation)
                 < op_to_int_xiuc(right_operation);
        }
    }
};

template <std::size_t OpId>
struct less_op_areal_linear
    : less_op_xxx_linear< OpId, op_to_int<0,1,0,0,2,0> >
{};

template <std::size_t OpId>
struct less_op_areal_areal
{
    template <typename Turn>
    inline bool operator()(Turn const& left, Turn const& right) const
    {
        static const std::size_t other_op_id = (OpId + 1) % 2;
        static op_to_int<0, 1, 2, 3, 4, 0> op_to_int_uixc;

        segment_identifier const& left_other_seg_id = left.operations[other_op_id].seg_id;
        segment_identifier const& right_other_seg_id = right.operations[other_op_id].seg_id;

        typedef typename Turn::turn_operation_type operation_type;
        operation_type const& left_operation = left.operations[OpId];
        operation_type const& right_operation = right.operations[OpId];

        if ( left_other_seg_id.multi_index == right_other_seg_id.multi_index )
        {
            if ( left_other_seg_id.ring_index == right_other_seg_id.ring_index )
            {
                return op_to_int_uixc(left_operation) < op_to_int_uixc(right_operation);
            }
            else
            {
                return left_other_seg_id.ring_index < right_other_seg_id.ring_index;
            }
        }
        else
        {
            return left_other_seg_id.multi_index < right_other_seg_id.multi_index;
        }
    }
};

template <std::size_t OpId>
struct less_other_multi_index
{
    static const std::size_t other_op_id = (OpId + 1) % 2;

    template <typename Turn>
    inline bool operator()(Turn const& left, Turn const& right) const
    {
        return left.operations[other_op_id].seg_id.multi_index
             < right.operations[other_op_id].seg_id.multi_index;
    }
};

// sort turns by G1 - source_index == 0 by:
// seg_id -> distance and coordinates -> operation
template <std::size_t OpId, typename LessOp, typename Strategy>
struct less
{
    static_assert(OpId < 2, "Only 0 and 1 are valid OpIds.");

    explicit less(Strategy const& strategy = Strategy())
        : m_strategy(strategy)
    {}

    template <typename Turn>
    inline bool use_fraction(Turn const& left, Turn const& right) const
    {
        static LessOp less_op;

        return strategies::detail::get_topology(m_strategy).same_segment_position(
                    left.operations[OpId].fraction,
                    right.operations[OpId].fraction)
                && m_strategy.relate(left.point, right.point).apply(left.point, right.point)
             ?
             less_op(left, right)
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
        auto const& source = detail::sub_range(geometry, left.operations[OpId].seg_id);
        using range_type = typename std::remove_reference<decltype(source)>::type;
        detail::closed_clockwise_view<range_type, geometry::closure<range_type>::value,
            geometry::point_order<Geometry>::value> view(source);
        auto const index = left.operations[OpId].seg_id.segment_index;
        int const order = m_strategy.segment_position().apply(range::at(view, index), range::at(view, index + 1),
                                                               left.point, right.point);
        return order == 0 ? LessOp()(left, right) : order < 0;
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

}}} // namespace detail::relate::turns
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_TURNS_HPP
