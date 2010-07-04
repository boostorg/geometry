// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_DISTANCE_HPP
#define BOOST_GEOMETRY_STRATEGIES_DISTANCE_HPP


#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/strategies/tags.hpp>


namespace boost { namespace geometry
{


/*!
    \brief Traits class binding a distance strategy to a (possibly two) coordinate system(s)
    \ingroup distance
    \tparam T1 tag of coordinate system of first point type
    \tparam T2 tag of coordinate system of second point type
    \tparam P1 first point-type
    \tparam P2 second point-type
*/
template <typename T1, typename T2, typename P1, typename P2>
struct strategy_distance
{
    typedef strategy::not_implemented type;
};

/*!
    \brief Traits class binding a distance-to-segment strategy to a (possibly two) coordinate system(s)
    \ingroup distance
    \tparam CsTag1 tag of coordinate system of point type
    \tparam CsTag2 tag of coordinate system of segment type, usually same as CsTag1
    \tparam Point point-type
    \tparam PointOfSegment segment-point-type
*/
template <typename CsTag1, typename CsTag2, typename Point, typename PointOfSegment>
struct strategy_distance_segment
{
    typedef strategy::not_implemented type;
};



// New functionality:
template 
<
    typename Point1, 
    typename Point2 = Point1,
    typename Tag1 = typename cs_tag<Point1>::type,
    typename Tag2 = typename cs_tag<Point2>::type
>
struct default_distance_strategy
{
    typedef typename strategy_distance<Tag1, Tag2, Point1, Point2>::type type;
};

template 
<
    typename Point, 
    typename PointOfSegment,
    typename Tag1 = typename cs_tag<Point>::type,
    typename Tag2 = typename cs_tag<PointOfSegment>::type
>
struct default_distance_strategy_segment
{
    typedef typename strategy_distance_segment<Tag1, Tag2, Point, PointOfSegment>::type type;
};



namespace strategy { namespace distance { namespace services 
{

template <typename Strategy> struct tag {};


/*!
    \brief Metafunction delivering a similar strategy with other input point types
*/
template 
<
    typename Strategy, 
    typename Point1, 
    typename Point2
> 
struct similar_type {};

template 
<
    typename Strategy, 
    typename Point1, 
    typename Point2
> 
struct get_similar {};

template <typename Strategy> struct comparable_type {};
template <typename Strategy> struct get_comparable {};

template <typename Strategy> struct result_from_distance {};


}}} // namespace strategy::distance::services


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_DISTANCE_HPP
