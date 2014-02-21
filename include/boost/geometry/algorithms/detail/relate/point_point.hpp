// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2013, 2014.
// Modifications copyright (c) 2013, 2014, Oracle and/or its affiliates.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_POINT_POINT_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_POINT_POINT_HPP

#include <boost/geometry/algorithms/detail/disjoint/point_point.hpp>
#include <boost/geometry/algorithms/detail/within/point_in_geometry.hpp>

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace relate {

template <typename Point1, typename Point2>
struct point_point
{
    static const bool interruption_enabled = false;

    template <typename Result>
    static inline void apply(Point1 const& point1, Point2 const& point2, Result & result)
    {
        bool equal = detail::equals::equals_point_point(point1, point2);
        if ( equal )
        {
            set<interior, interior, '0'>(result);
        }
        else
        {
            set<interior, exterior, '0'>(result);
            set<exterior, interior, '0'>(result);
        }

        set<exterior, exterior, result_dimension<Point1>::value>(result);
    }
};

template <typename Point, typename MultiPoint>
std::pair<bool, bool> point_multipoint_check(Point const& point, MultiPoint const& multi_point)
{
    bool found_inside = false;
    bool found_outside = false;

    // point_in_geometry could be used here but why iterate over MultiPoint twice?
    // we must search for a point in the exterior because all points in MultiPoint can be equal

    typedef typename boost::range_value<MultiPoint>::type point_type;
    typedef typename boost::range_iterator<MultiPoint const>::type iterator;
    iterator it = boost::begin(multi_point);
    iterator last = boost::end(multi_point);
    for ( ; it != last ; ++it )
    {
        bool ii = detail::equals::equals_point_point(point, *it);

        if ( ii )
            found_inside = true;
        else
            found_outside = true;

        if ( found_inside && found_outside )
            break;
    }

    return std::make_pair(found_inside, found_outside);
}

template <typename Point, typename MultiPoint, bool Transpose = false>
struct point_multipoint
{
    static const bool interruption_enabled = false;

    template <typename Result>
    static inline void apply(Point const& point, MultiPoint const& multi_point, Result & result)
    {
        std::pair<bool, bool> rel = point_multipoint_check(point, multi_point);

        if ( rel.first ) // some point of MP is equal to P
        {
            set<interior, interior, '0', Transpose>(result);
            
        }
        else
        {
            set<interior, exterior, '0', Transpose>(result);
            set<exterior, interior, '0', Transpose>(result);
        }

        if ( rel.second ) // a point of MP was found outside P
        {
            set<exterior, interior, '0', Transpose>(result);
        }

        set<exterior, exterior, result_dimension<Point>::value, Transpose>(result);
    }
};

template <typename MultiPoint, typename Point>
struct multipoint_point
{
    static const bool interruption_enabled = false;

    template <typename Result>
    static inline void apply(MultiPoint const& multi_point, Point const& point, Result & result)
    {
        point_multipoint<Point, MultiPoint, true>::apply(point, multi_point, result);
    }
};

//template <typename MultiPoint1, typename MultiPoint2>
//struct multipoint_multipoint
//{
//    static const bool interruption_enabled = false;
//
//    template <typename Result>
//    static inline void apply(MultiPoint1 const& multi_point1, MultiPoint2 const& multi_point2, Result & result)
//    {
//        if ( boost::size(multi_point1) < boost::size(multi_point2) )
//        {
//            apply<false>(multi_point1, multi_point2, result);
//        }
//        else
//        {
//            apply<true>(multi_point2, multi_point1, result);
//        }
//    }
//
//    template <bool Transpose,
//              typename SortedMultiPoint,
//              typename IteratedMultiPoint,
//              typename Result>
//    void apply(SortedMultiPoint const& sorted_mpt,
//               IteratedMultiPoint const& iterated_mpt,
//               Result & result)
//    {
//        // sort points from the 1 MPt
//        typedef typename geometry::point_type<SortedMultiPoint>::type point_type;
//        std::vector<point_type> points;
//        std::sort(points.begin(), point.end(), geometry::less<point_type>());
//
//        // for each point in the second MPt
//        typedef typename boost::range_iterator<IteratedMultiPoint const>::iterator iterator;
//        for ( iterator it = boost::begin(iterated_mpt) ;
//              it != boost::end(iterated_mpt) ; ++it )
//        {
//
//// TODO: FOR THIS TO WORK, WE NEED geometry::less<> WHICH CAN TAKE 2 DIFFERENT POINT TYPES!
////     
//            bool found = binary_search (points.begin(), point.end(), *it, less);
//        }
//    }
//};

}} // namespace detail::relate
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_POINT_POINT_HPP
