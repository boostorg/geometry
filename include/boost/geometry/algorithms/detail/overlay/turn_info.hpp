// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_TURN_INFO_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_TURN_INFO_HPP

#include <boost/array.hpp>


namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace overlay {


enum operation_type
{
    operation_none,
    operation_union,
    operation_intersection,
    operation_blocked,
    operation_continue
};


enum method_type
{
    method_none,
    method_disjoint,
    method_crosses,
    method_touch,
    method_touch_interior,
    method_collinear,
    method_equal,
    method_error
};


/*!
    \brief Turn operation: operation
    \details Information necessary for traversal phase (a phase
        of the overlay process). The information is gathered during the
        get_turns (segment intersection) phase.
        The class is to be included in the turn_info class, either direct
        or a derived or similar class with more (e.g. enrichment) information.
 */
struct turn_operation
{
    operation_type operation;

    turn_operation()
        : operation(operation_none)
    {}
};


/*!
    \brief Turn information: intersection point, method, and turn information
    \details Information necessary for traversal phase (a phase
        of the overlay process). The information is gathered during the
        get_turns (segment intersection) phase.
    \tparam Point point type of intersection point
    \tparam Operation gives classes opportunity to add additional info
    \tparam Container gives classes opportunity to define how operations are stored
 */
template
<
    typename Point,
    typename Operation = turn_operation,
    typename Container = boost::array<Operation, 2>
>
struct turn_info
{
    typedef Point point_type;
    typedef Operation operation_type;
    typedef Container container_type;

    Point point;
    method_type method;
    bool ignore;
    bool rejected;

    Container operations;

    turn_info()
        : method(method_none)
        , ignore(false)
        , rejected(false)
    {}

};



}} // namespace detail::overlay
#endif //DOXYGEN_NO_DETAIL


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_TURN_INFO_HPP
