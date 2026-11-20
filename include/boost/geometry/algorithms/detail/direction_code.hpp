// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2015-2020 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2015-2020.
// Modifications copyright (c) 2015-2020 Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_DIRECTION_CODE_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_DIRECTION_CODE_HPP

#include <boost/geometry/strategies/detail/direction_code.hpp>

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail
{

template <typename CSTag>
using direction_code_impl = strategies::detail::direction_code_impl<CSTag>;

// Gives sense of direction for point p, collinear w.r.t. segment (a,b)
// Returns -1 if p goes backward w.r.t (a,b), so goes from b in direction of a
// Returns 1 if p goes forward, so extends (a,b)
// Returns 0 if p is equal with b, or if (a,b) is degenerate
// Note that it does not do any collinearity test, that should be done before
// In some cases the "segment" consists of different source points, and therefore
// their types might differ.
template <typename CSTag, typename PointSegmentA, typename PointSegmentB, typename Point2>
inline int direction_code(PointSegmentA const& segment_a, PointSegmentB const& segment_b,
                          Point2 const& p)
{
    return direction_code_impl<CSTag>::apply(segment_a, segment_b, p);
}

template <typename PointSegmentA, typename PointSegmentB, typename Point, typename Strategy>
inline int direction_code(PointSegmentA const& segment_a, PointSegmentB const& segment_b,
                          Point const& p, Strategy const& strategy)
{
    return strategies::detail::direction_code(segment_a, segment_b, p, strategy);
}

} // namespace detail
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_DIRECTION_CODE_HPP
