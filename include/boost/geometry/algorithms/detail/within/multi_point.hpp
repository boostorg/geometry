// Boost.Geometry

// Copyright (c) 2017 Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_WITHIN_MULTI_POINT_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_WITHIN_MULTI_POINT_HPP


#include <algorithm>
#include <vector>

#include <boost/range.hpp>

#include <boost/geometry/algorithms/detail/relate/less.hpp>


namespace boost { namespace geometry {

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace within {

struct multi_point_point
{
    template <typename MultiPoint, typename Point, typename Strategy> static inline
    bool apply(MultiPoint const& multi_point, Point const& point,
               Strategy const& strategy)
    {
        typedef typename boost::range_const_iterator<MultiPoint>::type iterator;
        for ( iterator it = boost::begin(multi_point) ; it != boost::end(multi_point) ; ++it )
        {
            if (! strategy.apply(*it, point))
            {
                return false;
            }
        }

        // all points of MultiPoint inside Point
        return true;
    }
};

// NOTE: currently the strategy is ignored, math::equals() is used inside relate::less
struct multi_point_multi_point
{
    template <typename MultiPoint1, typename MultiPoint2, typename Strategy> static inline
    bool apply(MultiPoint1 const& multi_point1, MultiPoint2 const& multi_point2,
               Strategy const& /*strategy*/)
    {
        typedef typename boost::range_value<MultiPoint2>::type point2_type;

        relate::less const less;

        std::vector<point2_type> points2(boost::begin(multi_point2), boost::end(multi_point2));
        std::sort(points2.begin(), points2.end(), less);

        typedef typename boost::range_const_iterator<MultiPoint1>::type iterator;
        for ( iterator it = boost::begin(multi_point1) ; it != boost::end(multi_point1) ; ++it )
        {
            if (! std::binary_search(points2.begin(), points2.end(), *it, less))
            {
                return false;
            }
        }

        // all points of MultiPoint1 inside MultiPoint2
        return true;
    }
};


}} // namespace detail::within
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_WITHIN_MULTI_POINT_HPP
