// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2021 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_APPROXIMATELY_EQUALS_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_APPROXIMATELY_EQUALS_HPP

#include <boost/geometry/strategies/detail/topology.hpp>

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace overlay
{

// Compatibility aliases. New clustering code uses strategies::detail::get_topology.
template <typename T>
using common_approximately_equals_epsilon_multiplier
    = strategies::detail::topology_detail::common_approximately_equals_epsilon_multiplier<T>;

template <typename Point1, typename Point2, typename E>
inline bool approximately_equals(Point1 const& a, Point2 const& b,
                                 E const& epsilon_multiplier)
{
    return strategies::detail::topology_detail::approximately_equals(a, b, epsilon_multiplier);
}

}} // namespace detail::overlay
#endif //DOXYGEN_NO_DETAIL


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_APPROXIMATELY_EQUALS_HPP
