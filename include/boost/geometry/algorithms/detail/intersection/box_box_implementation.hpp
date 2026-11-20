// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2015 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2015-2024.
// Modifications copyright (c) 2015-2024, Oracle and/or its affiliates.
// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_INTERSECTION_BOX_BOX_IMPLEMENTATION_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_INTERSECTION_BOX_BOX_IMPLEMENTATION_HPP


#include <boost/geometry/strategies/detail/box_intersection.hpp>


namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace intersection
{

template <std::size_t Dimension, std::size_t DimensionCount>
struct intersection_box_box
{
    template
    <
        typename Box1, typename Box2,
        typename BoxOut,
        typename Strategy
    >
    static inline bool apply(Box1 const& box1,
            Box2 const& box2,
            BoxOut& box_out,
            Strategy const&)
    {
        return strategies::detail::box_intersection_detail::intersection_loop
            <Dimension, DimensionCount>::apply(box1, box2, box_out);
    }
};


}} // namespace detail::intersection
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_INTERSECTION_BOX_BOX_IMPLEMENTATION_HPP
