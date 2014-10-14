// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_DISJOINT_MULTIPOINT_GEOMETRY_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_DISJOINT_MULTIPOINT_GEOMETRY_HPP

#include <algorithm>

#include <boost/assert.hpp>
#include <boost/range.hpp>

#include <boost/geometry/algorithms/detail/check_iterator_range.hpp>
#include <boost/geometry/algorithms/detail/disjoint/multirange_geometry.hpp>
#include <boost/geometry/algorithms/detail/disjoint/point_point.hpp>
#include <boost/geometry/algorithms/detail/disjoint/point_geometry.hpp>
#include <boost/geometry/algorithms/detail/relate/less.hpp>

#include <boost/geometry/algorithms/dispatch/disjoint.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace disjoint
{


template<typename MultiPoint1, typename MultiPoint2>
struct multipoint_multipoint
{
    static inline bool apply(MultiPoint1 const& multipoint1,
                             MultiPoint2 const& multipoint2)
    {
        BOOST_ASSERT( boost::size(multipoint1) <= boost::size(multipoint2) );

        typedef typename boost::range_value<MultiPoint1>::type point1_type;

        std::vector<point1_type> points1(boost::begin(multipoint1),
                                         boost::end(multipoint1));
        std::sort(points1.begin(), points1.end(), detail::relate::less());

        
        typedef typename boost::range_iterator
            <
                MultiPoint2 const
            >::type iterator2;

        for (iterator2 it = boost::begin(multipoint2);
             it != boost::end(multipoint2); ++it)
        {
            if ( std::binary_search(points1.begin(), points1.end(),
                                    *it, detail::relate::less()) )
            {
                return false;
            }
        }

        return true;
    }
};


}} // namespace detail::disjoint
#endif // DOXYGEN_NO_DETAIL




#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


template<typename Point, typename MultiPoint, std::size_t DimensionCount>
struct disjoint
    <
        Point, MultiPoint, DimensionCount, point_tag, multi_point_tag, false
    > : detail::disjoint::multirange_constant_size_geometry<MultiPoint, Point>
{};


template<typename MultiPoint, typename Segment, std::size_t DimensionCount>
struct disjoint
    <
        MultiPoint, Segment, DimensionCount, multi_point_tag, segment_tag, false
    > : detail::disjoint::multirange_constant_size_geometry<MultiPoint, Segment>
{};


template<typename MultiPoint, typename Box, std::size_t DimensionCount>
struct disjoint
    <
        MultiPoint, Box, DimensionCount, multi_point_tag, box_tag, false
    > : detail::disjoint::multirange_constant_size_geometry<MultiPoint, Box>
{};


template<typename MultiPoint1, typename MultiPoint2, std::size_t DimensionCount>
struct disjoint
    <
        MultiPoint1, MultiPoint2, DimensionCount,
        multi_point_tag, multi_point_tag, false
    >
{
    static inline bool apply(MultiPoint1 const& multipoint1,
                             MultiPoint2 const& multipoint2)
    {
        if ( boost::size(multipoint2) < boost::size(multipoint1) )
        {
            return detail::disjoint::multipoint_multipoint
                <
                    MultiPoint2, MultiPoint1
                >::apply(multipoint2, multipoint1);
        } 

        return detail::disjoint::multipoint_multipoint
            <
                MultiPoint1, MultiPoint2
            >::apply(multipoint1, multipoint2);
   }
};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


}} // namespace boost::geometry



#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_DISJOINT_MULTIPOINT_GEOMETRY_HPP
