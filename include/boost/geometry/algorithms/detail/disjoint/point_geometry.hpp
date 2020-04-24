// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2015 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2015 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2015 Mateusz Loskot, London, UK.
// Copyright (c) 2013-2015 Adam Wulkiewicz, Lodz, Poland.

// This file was modified by Oracle on 2013, 2014, 2015.
// Modifications copyright (c) 2013-2015, Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle
// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_DISJOINT_POINT_GEOMETRY_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_DISJOINT_POINT_GEOMETRY_HPP

#include <boost/geometry/algorithms/covered_by.hpp>

#include <boost/geometry/algorithms/detail/disjoint/linear_linear.hpp>

#include <boost/geometry/algorithms/dispatch/disjoint.hpp>
#include <boost/geometry/algorithms/dispatch/disjoint_with_info.hpp>

namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace disjoint
{


struct reverse_covered_by
{
    template <typename Geometry1, typename Geometry2, typename Strategy>
    static inline bool apply(Geometry1 const& geometry1,
                             Geometry2 const& geometry2,
                             Strategy const& strategy)
    {
        return ! geometry::covered_by(geometry1, geometry2, strategy);
    }
};

template <typename Point>
struct reverse_covered_by_with_info
{
    typedef segment_intersection_points<Point> intersection_return_type;

    template <typename Geometry2, typename Strategy>
    static inline intersection_return_type
    apply(Point const& point,
          Geometry2 const& geometry2,
          Strategy const& strategy)
    {
        intersection_return_type res;
        if (geometry::covered_by(point, geometry2, strategy))
        {
            res.count = 1;
            res.intersections[0] = point;
            return res;
        }
        return res;
    }
};


}} // namespace detail::disjoint
#endif // DOXYGEN_NO_DETAIL




#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


template<typename Point, typename Linear, std::size_t DimensionCount>
struct disjoint<Point, Linear, DimensionCount, point_tag, linear_tag, false>
    : detail::disjoint::reverse_covered_by
{};


template <typename Point, typename Areal, std::size_t DimensionCount>
struct disjoint<Point, Areal, DimensionCount, point_tag, areal_tag, false>
    : detail::disjoint::reverse_covered_by
{};


template<typename Point, typename Segment, std::size_t DimensionCount>
struct disjoint<Point, Segment, DimensionCount, point_tag, segment_tag, false>
    : detail::disjoint::reverse_covered_by
{};


template<typename Point, typename Linear, std::size_t DimensionCount>
struct disjoint_with_info<Point, Linear, DimensionCount, point_tag, linear_tag, false>
    : detail::disjoint::reverse_covered_by_with_info<Point>
{};


template <typename Point, typename Areal, std::size_t DimensionCount>
struct disjoint_with_info<Point, Areal, DimensionCount, point_tag, areal_tag, false>
    : detail::disjoint::reverse_covered_by_with_info<Point>
{};


template<typename Point, typename Segment, std::size_t DimensionCount>
struct disjoint_with_info<Point, Segment, DimensionCount, point_tag, segment_tag, false>
    : detail::disjoint::reverse_covered_by_with_info<Point>
{};



} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


}} // namespace boost::geometry



#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_DISJOINT_POINT_GEOMETRY_HPP
