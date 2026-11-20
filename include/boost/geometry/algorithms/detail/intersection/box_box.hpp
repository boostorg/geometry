// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2014.
// Modifications copyright (c) 2014, Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_INTERSECTION_BOX_BOX_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_INTERSECTION_BOX_BOX_HPP


#include <boost/geometry/algorithms/detail/intersection/interface.hpp>
#include <boost/geometry/strategies/detail/box_intersection.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


template
<
    typename Box1, typename Box2, bool Reverse
>
struct intersection
    <
        Box1, Box2,
        box_tag, box_tag,
        Reverse
    >
{
    template <typename BoxOut, typename Strategy>
    static bool apply(Box1 const& box1, Box2 const& box2, BoxOut& out, Strategy const& strategy)
    {
        return strategies::detail::get_box_intersection(strategy).apply(box1, box2, out);
    }
};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_INTERSECTION_BOX_BOX_HPP
