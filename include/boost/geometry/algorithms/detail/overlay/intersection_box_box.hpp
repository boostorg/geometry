// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2015 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2015.
// Modifications copyright (c) 2015, Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_INTERSECTION_BOX_BOX_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_INTERSECTION_BOX_BOX_HPP


#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/util/math.hpp>

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
        typename RobustPolicy,
        typename BoxOut,
        typename Strategy
    >
    static inline bool apply(Box1 const& box1,
            Box2 const& box2,
            RobustPolicy const& robust_policy,
            BoxOut& box_out,
            Strategy const& strategy)
    {
        typedef typename coordinate_type<BoxOut>::type ct;

        ct min1 = get<min_corner, Dimension>(box1);
        ct min2 = get<min_corner, Dimension>(box2);
        ct max1 = get<max_corner, Dimension>(box1);
        ct max2 = get<max_corner, Dimension>(box2);

        if (math::smaller(max1, min2) || math::smaller(max2, min1))
        {
            return false;
        }
        
        ct const res_min = min1 < min2 ? min2 : min1;
        ct const res_max = max1 > max2 ? max2 : max1;

        // Set dimensions of output coordinate
        // This condition is needed because math::smaller() compares values WRT
        // machine epsilon, so max may be lesser than min when compared with
        // raw operator<
        if (res_min <= res_max)
        {
            set<min_corner, Dimension>(box_out, res_min);
            set<max_corner, Dimension>(box_out, res_max);
        }
        else
        {
            set<min_corner, Dimension>(box_out, res_max);
            set<max_corner, Dimension>(box_out, res_min);
        }
        

        return intersection_box_box<Dimension + 1, DimensionCount>
               ::apply(box1, box2, robust_policy, box_out, strategy);
    }
};

template <std::size_t DimensionCount>
struct intersection_box_box<DimensionCount, DimensionCount>
{
    template
    <
        typename Box1, typename Box2,
        typename RobustPolicy,
        typename BoxOut,
        typename Strategy
    >
    static inline bool apply(Box1 const&, Box2 const&,
            RobustPolicy const&, BoxOut&, Strategy const&)
    {
        return true;
    }
};


}} // namespace detail::intersection
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_INTERSECTION_BOX_BOX_HPP
