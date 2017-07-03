// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014-2015, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_IS_VALID_IS_ACCEPTABLE_TURN_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_IS_VALID_IS_ACCEPTABLE_TURN_HPP

#include <boost/range.hpp>

#include <boost/geometry/core/point_order.hpp>
#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tags.hpp>

#include <boost/geometry/algorithms/detail/overlay/turn_info.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace is_valid
{


template
<
    typename Geometry,
    order_selector Order = geometry::point_order<Geometry>::value,
    typename Tag = typename tag<Geometry>::type
>
struct acceptable_operation
{};

template <typename Polygon>
struct acceptable_operation<Polygon, counterclockwise, polygon_tag>
{
    static const detail::overlay::operation_type value =
        detail::overlay::operation_union;
};

template <typename Polygon>
struct acceptable_operation<Polygon, clockwise, polygon_tag>
{
    static const detail::overlay::operation_type value =
        detail::overlay::operation_intersection;
};

template <typename MultiPolygon>
struct acceptable_operation<MultiPolygon, counterclockwise, multi_polygon_tag>
{
    static const detail::overlay::operation_type value =
        detail::overlay::operation_intersection;
};

template <typename MultiPolygon>
struct acceptable_operation<MultiPolygon, clockwise, multi_polygon_tag>
{
    static const detail::overlay::operation_type value =
        detail::overlay::operation_union;
};




template <typename Geometry, typename Tag = typename tag<Geometry>::type>
struct is_acceptable_turn
{};

template <typename Ring>
struct is_acceptable_turn<Ring, ring_tag>
{
    template <typename Turn>
    static inline bool apply(Turn const&)
    {
        return false;
    }
};

template <typename Polygon>
class is_acceptable_turn<Polygon, polygon_tag>
{
protected:
    template <typename Turn, typename Method, typename Operation>
    static inline bool check_turn(Turn const& turn,
                                  Method method,
                                  Operation operation)
    {
        return turn.method == method
            && turn.operations[0].operation == operation
            && turn.operations[1].operation == operation;
    }


public:
    template <typename Turn>
    static inline bool apply(Turn const& turn)
    {
        using namespace detail::overlay;

        if ( turn.operations[0].seg_id.ring_index
             == turn.operations[1].seg_id.ring_index )
        {
            return false;
        }

        operation_type const op = acceptable_operation<Polygon>::value;

        return check_turn(turn, method_touch_interior, op)
            || check_turn(turn, method_touch, op)
            ;
    }
};

template <typename MultiPolygon>
class is_acceptable_turn<MultiPolygon, multi_polygon_tag>
    : is_acceptable_turn<typename boost::range_value<MultiPolygon>::type>
{
private:
    typedef typename boost::range_value<MultiPolygon>::type polygon;
    typedef is_acceptable_turn<polygon> base;

    template <typename Operation>
    static inline
    bool check_int_ext(Operation const& op1,
        detail::overlay::operation_type optype1,
        Operation const& op2,
        detail::overlay::operation_type optype2)
    {
        // u/i is acceptable for touch of interior ring with another exterior ring
        // (but only if there is a colocated uu-turn of its exterior, TODO)
        return op1.seg_id.ring_index == -1
            && op2.seg_id.ring_index >= 0
            && op1.operation == optype1
            && op2.operation == optype2;
    }

    template <typename Operation>
    static inline
    bool check_int_int(Operation const& op1,
        Operation const& op2,
        detail::overlay::operation_type optype)
    {
        // i/i is acceptable for touching interior/interior rings
        return op1.seg_id.ring_index >= 0
            && op2.seg_id.ring_index >= 0
            && op1.operation == optype
            && op2.operation == optype;
    }

public:
    template <typename Turn>
    static inline bool apply(Turn const& turn)
    {
        using namespace detail::overlay;

        if ( turn.operations[0].seg_id.multi_index
             == turn.operations[1].seg_id.multi_index )
        {
            return base::apply(turn);
        }

        operation_type const op = acceptable_operation<MultiPolygon>::value;
        if ( base::check_turn(turn, method_touch_interior, op)
          || base::check_turn(turn, method_touch, op))
        {
            return true;
        }

        if (turn.method != method_touch)
        {
            return false;
        }

        operation_type const reverse_op
                = op == operation_union
                ? operation_intersection
                : operation_union;

        if ( check_int_int(turn.operations[0], turn.operations[1], reverse_op)
          || check_int_ext(turn.operations[0], reverse_op,
                           turn.operations[1], op)
          || check_int_ext(turn.operations[1], reverse_op,
                           turn.operations[0], op))
        {
            return true;
        }

        return false;
    }
};   


}} // namespace detail::is_valid
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_IS_VALID_IS_ACCEPTABLE_TURN_HPP
