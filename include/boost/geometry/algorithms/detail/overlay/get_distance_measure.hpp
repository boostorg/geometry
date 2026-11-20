// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2019-2021 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2022.
// Modifications copyright (c) 2022 Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_GET_DISTANCE_MEASURE_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_GET_DISTANCE_MEASURE_HPP

#include <boost/geometry/strategies/detail/topology.hpp>

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail
{

template <typename T>
using distance_measure = strategies::detail::distance_measure<T>;

} // detail


namespace detail_dispatch
{

// Compatibility alias for the former internal dispatch implementation.
template <typename CalculationType, typename CsTag>
using get_distance_measure
    = strategies::detail::distance_measure_detail::side_measure<CalculationType, CsTag>;

} // namespace detail_dispatch

namespace detail
{

// Returns a (often very tiny) value to indicate its side, and distance,
// 0 (absolutely 0, not even an epsilon) means collinear. Like side,
// a negative means that p is to the right of p1-p2. And a positive value
// means that p is to the left of p1-p2.
template <typename SegmentPoint, typename Point, typename Strategies>
inline auto get_distance_measure(SegmentPoint const& p1, SegmentPoint const& p2, Point const& p,
                                 Strategies const& strategies)
{
    return strategies::detail::get_topology(strategies).distance_measure(p1, p2, p);
}

} // namespace detail
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_GET_DISTANCE_MEASURE_HPP
