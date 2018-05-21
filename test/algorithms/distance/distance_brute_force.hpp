// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2014, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_TEST_DISTANCE_BRUTE_FORCE_HPP
#define BOOST_GEOMETRY_TEST_DISTANCE_BRUTE_FORCE_HPP

#include <iterator>

#include <boost/mpl/assert.hpp>
#include <boost/mpl/or.hpp>
#include <boost/range.hpp>

#include <boost/geometry/core/reverse_dispatch.hpp>
#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tag_cast.hpp>
#include <boost/geometry/core/tags.hpp>

#include <boost/geometry/iterators/segment_iterator.hpp>

#include <boost/geometry/algorithms/distance.hpp>
#include <boost/geometry/algorithms/intersects.hpp>
#include <boost/geometry/algorithms/not_implemented.hpp>


namespace boost { namespace geometry
{

namespace unit_test
{

namespace detail { namespace distance_brute_force
{

struct distance_from_bg
{
    template <typename G>
    struct use_distance_from_bg
    {
        typedef typename boost::mpl::or_
            <
                boost::is_same<typename tag<G>::type, point_tag>,
                typename boost::mpl::or_
                    <
                        boost::is_same<typename tag<G>::type, segment_tag>,
                        boost::is_same<typename tag<G>::type, box_tag>
                    >::type
            >::type type;
    };

    template <typename Geometry1, typename Geometry2, typename Strategy>
    static inline
    typename distance_result<Geometry1, Geometry2, Strategy>::type
    apply(Geometry1 const& geometry1,
          Geometry2 const& geometry2,
          Strategy const& strategy)
    {
        BOOST_MPL_ASSERT((typename use_distance_from_bg<Geometry1>::type));
        BOOST_MPL_ASSERT((typename use_distance_from_bg<Geometry2>::type));

        return geometry::distance(geometry1, geometry2, strategy);
    }
};


template <typename Geometry1, typename Geometry2, typename Strategy>
inline
typename distance_result<Geometry1, Geometry2, Strategy>::type
bg_distance(Geometry1 const& geometry1,
            Geometry2 const& geometry2,
            Strategy const& strategy)
{
    return distance_from_bg::apply(geometry1, geometry2, strategy);
}


template <typename Policy>
struct one_to_many
{
    template <typename Geometry, typename Iterator, typename Strategy>
    static inline typename distance_result
        <
            Geometry,
            typename std::iterator_traits<Iterator>::value_type,
            Strategy
        >::type
    apply(Geometry const& geometry, Iterator begin, Iterator end,
          Strategy const& strategy)
    {
        typedef typename distance_result
        <
            Geometry,
            typename std::iterator_traits<Iterator>::value_type,
            Strategy
        >::type distance_type;

        bool first = true;
        distance_type d_min(0);
        for (Iterator it = begin; it != end; ++it, first = false)
        {
            distance_type d = Policy::apply(geometry, *it, strategy);

            if ( first || d < d_min )
            {
                d_min = d;
            }
        }
        return d_min;
    }
};



}} // namespace detail::distance_brute_force


namespace dispatch
{

template
<
    typename Geometry1,
    typename Geometry2,
    typename Strategy,
    typename Tag1 = typename tag_cast
        <
            typename tag<Geometry1>::type,
            segment_tag,
            linear_tag
        >::type,
    typename Tag2 = typename tag_cast
        <
            typename tag<Geometry2>::type,
            segment_tag,
            linear_tag
        >::type,
    bool Reverse = reverse_dispatch<Geometry1, Geometry2>::type::value
>
struct distance_brute_force
    : not_implemented<Geometry1, Geometry2>
{};

template
<
    typename Geometry1,
    typename Geometry2,
    typename Strategy,
    typename Tag1,
    typename Tag2
>
struct distance_brute_force<Geometry1, Geometry2, Strategy, Tag1, Tag2, true>
{
    static inline typename distance_result<Geometry1, Geometry2, Strategy>::type
    apply(Geometry1 const& geometry1,
          Geometry2 const& geometry2,
          Strategy const& strategy)
    {
        return distance_brute_force
            <
                Geometry2, Geometry1, Strategy
            >::apply(geometry2, geometry1, strategy);
    }
};

//===================================================================

template
<
    typename Point1,
    typename Point2,
    typename Strategy
>
struct distance_brute_force
<
    Point1, Point2, Strategy,
    point_tag, point_tag, false
> : detail::distance_brute_force::distance_from_bg
{};


template
<
    typename Point,
    typename Segment,
    typename Strategy
>
struct distance_brute_force
<
    Point, Segment, Strategy,
    point_tag, segment_tag, false
> : detail::distance_brute_force::distance_from_bg
{};

template
<
    typename Point,
    typename Linear,
    typename Strategy
>
struct distance_brute_force
<
    Point, Linear, Strategy,
    point_tag, linear_tag, false
>
{
    typedef typename distance_result
        <
            Point, Linear, Strategy
        >::type distance_type;

    static inline distance_type apply(Point const& point,
                                      Linear const& linear,
                                      Strategy const& strategy)
    {
        return detail::distance_brute_force::one_to_many
            <
                detail::distance_brute_force::distance_from_bg
            >::apply(point,
                     geometry::segments_begin(linear),
                     geometry::segments_end(linear),
                     strategy);
    }
};

template
<
    typename Point,
    typename Ring,
    typename Strategy
>
struct distance_brute_force
<
    Point, Ring, Strategy,
    point_tag, ring_tag, false
>
{
    typedef typename distance_result
        <
            Point, Ring, Strategy
        >::type distance_type;

    static inline distance_type apply(Point const& point,
                                      Ring const& ring,
                                      Strategy const& strategy)
    {
        return detail::distance_brute_force::one_to_many
            <
                distance_brute_force
                    <
                        Point,
                        typename std::iterator_traits
                            <
                                segment_iterator<Ring const>
                            >::value_type,
                        Strategy
                    >
            >::apply(point,
                     geometry::segments_begin(ring),
                     geometry::segments_end(ring),
                     strategy);
    }
};

//TODO iterate over exterior rings
template
<
    typename Point,
    typename Polygon,
    typename Strategy
>
struct distance_brute_force
<
    Point, Polygon, Strategy,
    point_tag, polygon_tag, false
>
{
    typedef typename distance_result
        <
            Point, Polygon, Strategy
        >::type distance_type;

    static inline distance_type apply(Point const& point,
                                      Polygon const& polygon,
                                      Strategy const& strategy)
    {
        return detail::distance_brute_force::one_to_many
            <
                distance_brute_force
                    <
                        Point,
                        typename std::iterator_traits
                            <
                                segment_iterator<Polygon const>
                            >::value_type,
                        Strategy
                    >
            >::apply(point,
                     geometry::segments_begin(polygon),
                     geometry::segments_end(polygon),
                     strategy);
    }
};

template
<
    typename Point,
    typename Box,
    typename Strategy
>
struct distance_brute_force
<
    Point, Box, Strategy,
    point_tag, box_tag, false
> : detail::distance_brute_force::distance_from_bg
{};

template
<
    typename Point,
    typename MultiPoint,
    typename Strategy
>
struct distance_brute_force
<
    Point, MultiPoint, Strategy,
    point_tag, multi_point_tag, false
>
{
    typedef typename distance_result
        <
            Point, MultiPoint, Strategy
        >::type distance_type;

    static inline distance_type apply(Point const& p,
                                      MultiPoint const& mp,
                                      Strategy const& strategy)
    {
        return detail::distance_brute_force::one_to_many
            <
                detail::distance_brute_force::distance_from_bg
            >::apply(p, boost::begin(mp), boost::end(mp), strategy);
    }
};

template
<
    typename Point,
    typename MultiPolygon,
    typename Strategy
>
struct distance_brute_force
<
    Point, MultiPolygon, Strategy,
    point_tag, multi_polygon_tag, false
>
{
    typedef typename distance_result
        <
            Point, MultiPolygon, Strategy
        >::type distance_type;

    static inline distance_type apply(Point const& p,
                                      MultiPolygon const& mp,
                                      Strategy const& strategy)
    {
        return detail::distance_brute_force::one_to_many
            <
                distance_brute_force
                <
                    Point,
                    typename boost::range_value<MultiPolygon>::type,
                    Strategy
                >
            >::apply(p, boost::begin(mp), boost::end(mp), strategy);
    }
};

//=======================================================================

template
<
    typename Linear,
    typename Segment,
    typename Strategy
>
struct distance_brute_force
<
    Linear, Segment, Strategy,
    linear_tag, segment_tag, false
>
{
    typedef typename distance_result
        <
            Linear, Segment, Strategy
        >::type distance_type;

    static inline distance_type apply(Linear const& linear,
                                      Segment const& segment,
                                      Strategy const& strategy)
    {
        return detail::distance_brute_force::one_to_many
            <
                detail::distance_brute_force::distance_from_bg
            >::apply(segment,
                     geometry::segments_begin(linear),
                     geometry::segments_end(linear),
                     strategy);
    }
};


template
<
    typename Linear1,
    typename Linear2,
    typename Strategy
>
struct distance_brute_force
<
    Linear1, Linear2, Strategy,
    linear_tag, linear_tag, false
>
{
    typedef typename distance_result
        <
            Linear1, Linear2, Strategy
        >::type distance_type;

    static inline distance_type apply(Linear1 const& linear1,
                                      Linear2 const& linear2,
                                      Strategy const& strategy)
    {
        return detail::distance_brute_force::one_to_many
            <
                distance_brute_force
                    <
                        Linear1,
                        typename std::iterator_traits
                            <
                                segment_iterator<Linear2 const>
                            >::value_type,
                        Strategy
                    >
            >::apply(linear1,
                     geometry::segments_begin(linear2),
                     geometry::segments_end(linear2),
                     strategy);
    }
};

template
<
    typename Linear,
    typename Ring,
    typename Strategy
>
struct distance_brute_force
<
    Linear, Ring, Strategy,
    linear_tag, ring_tag, false
>
{
    typedef typename distance_result
        <
            Linear, Ring, Strategy
        >::type distance_type;

    static inline distance_type apply(Linear const& linear,
                                      Ring const& ring,
                                      Strategy const& strategy)
    {
        return detail::distance_brute_force::one_to_many
            <
                distance_brute_force
                    <
                        Linear,
                        typename std::iterator_traits
                            <
                                segment_iterator<Ring const>
                            >::value_type,
                        Strategy
                    >
            >::apply(linear,
                     geometry::segments_begin(ring),
                     geometry::segments_end(ring),
                     strategy);
    }
};

template
<
    typename Linear,
    typename Polygon,
    typename Strategy
>
struct distance_brute_force
<
    Linear, Polygon, Strategy,
    linear_tag, polygon_tag, false
>
{
    typedef typename distance_result
        <
            Linear, Polygon, Strategy
        >::type distance_type;

    static inline distance_type apply(Linear const& linear,
                                      Polygon const& polygon,
                                      Strategy const& strategy)
    {
        return detail::distance_brute_force::one_to_many
            <
                distance_brute_force
                    <
                        Linear,
                        typename std::iterator_traits
                            <
                                segment_iterator<Polygon const>
                            >::value_type,
                        Strategy
                    >
            >::apply(linear,
                     geometry::segments_begin(polygon),
                     geometry::segments_end(polygon),
                     strategy);
    }
};


template
<
    typename Linear,
    typename Box,
    typename Strategy
>
struct distance_brute_force
<
    Linear, Box, Strategy,
    linear_tag, box_tag, false
>
{
    typedef typename distance_result
        <
            Linear, Box, Strategy
        >::type distance_type;

    static inline distance_type apply(Linear const& linear,
                                      Box const& box,
                                      Strategy const& strategy)
    {
        return detail::distance_brute_force::one_to_many
            <
                detail::distance_brute_force::distance_from_bg
            >::apply(box,
                     geometry::segments_begin(linear),
                     geometry::segments_end(linear),
                     strategy);
    }
};

template
<
    typename Linear,
    typename MultiPoint,
    typename Strategy
>
struct distance_brute_force
<
    Linear, MultiPoint, Strategy,
    linear_tag, multi_point_tag, false
>
{
    typedef typename distance_result
        <
            Linear, MultiPoint, Strategy
        >::type distance_type;

    static inline distance_type apply(Linear const& linear,
                                      MultiPoint const& mp,
                                      Strategy const& strategy)
    {
        return detail::distance_brute_force::one_to_many
            <
                distance_brute_force
                <
                    Linear,
                    typename boost::range_value<MultiPoint>::type,
                    Strategy
                >
            >::apply(linear, boost::begin(mp), boost::end(mp), strategy);
    }
};

template
<
    typename Linear,
    typename MultiPolygon,
    typename Strategy
>
struct distance_brute_force
<
    Linear, MultiPolygon, Strategy,
    linear_tag, multi_polygon_tag, false
>
{
    typedef typename distance_result
        <
            Linear, MultiPolygon, Strategy
        >::type distance_type;

    static inline distance_type apply(Linear const& linear,
                                      MultiPolygon const& mp,
                                      Strategy const& strategy)
    {
        return detail::distance_brute_force::one_to_many
            <
                distance_brute_force
                <
                    Linear,
                    typename boost::range_value<MultiPolygon>::type,
                    Strategy
                >
            >::apply(linear, boost::begin(mp), boost::end(mp), strategy);
    }
};

//=================================================================

template
<
    typename Polygon,
    typename Segment,
    typename Strategy
>
struct distance_brute_force
<
    Polygon, Segment, Strategy,
    polygon_tag, segment_tag, false
>
{
    typedef typename distance_result
        <
            Polygon, Segment, Strategy
        >::type distance_type;

    static inline distance_type apply(Polygon const& polygon,
                                      Segment const& segment,
                                      Strategy const& strategy)
    {
        return detail::distance_brute_force::one_to_many
            <
                distance_brute_force
                    <
                        Segment,
                        typename std::iterator_traits
                            <
                                segment_iterator<Polygon const>
                            >::value_type,
                        Strategy
                    >
            >::apply(segment,
                     geometry::segments_begin(polygon),
                     geometry::segments_end(polygon),
                     strategy);
    }
};

template
<
    typename Polygon,
    typename Linear,
    typename Strategy
>
struct distance_brute_force
<
    Polygon, Linear, Strategy,
    polygon_tag, linear_tag, false
>
{
    typedef typename distance_result
        <
            Polygon, Linear, Strategy
        >::type distance_type;

    static inline distance_type apply(Polygon const& polygon,
                                      Linear const& linear,
                                      Strategy const& strategy)
    {
        return detail::distance_brute_force::one_to_many
            <
                distance_brute_force
                    <
                        Linear,
                        typename std::iterator_traits
                            <
                                segment_iterator<Polygon const>
                            >::value_type,
                        Strategy
                    >
            >::apply(linear,
                     geometry::segments_begin(polygon),
                     geometry::segments_end(polygon),
                     strategy);
    }
};

template
<
    typename Polygon,
    typename MultiPoint,
    typename Strategy
>
struct distance_brute_force
<
    Polygon, MultiPoint, Strategy,
    polygon_tag, multi_point_tag, false
>
{
    typedef typename distance_result
        <
            Polygon, MultiPoint, Strategy
        >::type distance_type;

    static inline distance_type apply(Polygon const& polygon,
                                      MultiPoint const& mp,
                                      Strategy const& strategy)
    {
        return detail::distance_brute_force::one_to_many
            <
                distance_brute_force
                    <
                        MultiPoint,
                        typename std::iterator_traits
                            <
                                segment_iterator<Polygon const>
                            >::value_type,
                        Strategy
                    >
            >::apply(mp,
                     geometry::segments_begin(polygon),
                     geometry::segments_end(polygon),
                     strategy);
    }
};

//========================================================================

template
<
    typename MultiPoint1,
    typename MultiPoint2,
    typename Strategy
>
struct distance_brute_force
<
    MultiPoint1, MultiPoint2, Strategy,
    multi_point_tag, multi_point_tag, false
>
{
    typedef typename distance_result
        <
            MultiPoint1, MultiPoint2, Strategy
        >::type distance_type;

    static inline distance_type apply(MultiPoint1 const& mp1,
                                      MultiPoint2 const& mp2,
                                      Strategy const& strategy)
    {
        return detail::distance_brute_force::one_to_many
            <
                distance_brute_force
                <
                    MultiPoint1,
                    typename boost::range_value<MultiPoint2>::type,
                    Strategy
                >
            >::apply(mp1, boost::begin(mp2), boost::end(mp2), strategy);
    }
};

template
<
    typename MultiPoint,
    typename Linear,
    typename Strategy
>
struct distance_brute_force
<
    MultiPoint, Linear, Strategy,
    multi_point_tag, linear_tag, false
>
{
    typedef typename distance_result
        <
            MultiPoint, Linear, Strategy
        >::type distance_type;

    static inline distance_type apply(MultiPoint const& mp,
                                      Linear const& l,
                                      Strategy const& strategy)
    {
        return detail::distance_brute_force::one_to_many
            <
                distance_brute_force
                <
                    MultiPoint,
                    typename boost::range_value<Linear>::type,
                    Strategy
                >
            >::apply(mp, boost::begin(l), boost::end(l), strategy);
    }
};

template
<
    typename MultiPoint,
    typename MultiPolygon,
    typename Strategy
>
struct distance_brute_force
<
    MultiPoint, MultiPolygon, Strategy,
    multi_point_tag, multi_polygon_tag, false
>
{
    typedef typename distance_result
        <
            MultiPoint, MultiPolygon, Strategy
        >::type distance_type;

    static inline distance_type apply(MultiPoint const& mp,
                                      MultiPolygon const& mpl,
                                      Strategy const& strategy)
    {
        return detail::distance_brute_force::one_to_many
            <
                distance_brute_force
                <
                    MultiPoint,
                    typename boost::range_value<MultiPolygon>::type,
                    Strategy
                >
            >::apply(mp, boost::begin(mpl), boost::end(mpl), strategy);
    }
};

template
<
    typename MultiPoint,
    typename Segment,
    typename Strategy
>
struct distance_brute_force
<
    MultiPoint, Segment, Strategy,
    multi_point_tag, segment_tag, false
>
{
    typedef typename distance_result
        <
            MultiPoint, Segment, Strategy
        >::type distance_type;

    static inline distance_type apply(MultiPoint const& mp,
                                      Segment const& segment,
                                      Strategy const& strategy)
    {
        return detail::distance_brute_force::one_to_many
            <
                detail::distance_brute_force::distance_from_bg
            >::apply(segment, boost::begin(mp), boost::end(mp), strategy);
    }
};

template
<
    typename MultiPoint,
    typename Ring,
    typename Strategy
>
struct distance_brute_force
<
    MultiPoint, Ring, Strategy,
    multi_point_tag, ring_tag, false
>
{
    typedef typename distance_result
        <
            MultiPoint, Ring, Strategy
        >::type distance_type;

    static inline distance_type apply(MultiPoint const& mp,
                                      Ring const& ring,
                                      Strategy const& strategy)
    {


        return detail::distance_brute_force::one_to_many
            <
                distance_brute_force
                <
                    MultiPoint,
                    typename std::iterator_traits
                    <
                        segment_iterator<Ring const>
                    >::value_type,
                    Strategy
                >
            >::apply(mp,
                     geometry::segments_begin(ring),
                     geometry::segments_end(ring),
                     strategy);
    }
};

template
<
    typename MultiPoint,
    typename Box,
    typename Strategy
>
struct distance_brute_force
<
    MultiPoint, Box, Strategy,
    multi_point_tag, box_tag, false
>
{
    typedef typename distance_result
        <
            MultiPoint, Box, Strategy
        >::type distance_type;

    static inline distance_type apply(MultiPoint const& mp,
                                      Box const& box,
                                      Strategy const& strategy)
    {
        return detail::distance_brute_force::one_to_many
            <
                distance_brute_force
                <
                    Box,
                    typename boost::range_value<MultiPoint>::type,
                    Strategy
                >
            >::apply(box, boost::begin(mp), boost::end(mp), strategy);
    }
};

//=====================================================================

template
<
    typename MultiPolygon,
    typename Segment,
    typename Strategy
>
struct distance_brute_force
<
    MultiPolygon, Segment, Strategy,
    multi_polygon_tag, segment_tag, false
>
{
    typedef typename distance_result
        <
            MultiPolygon, Segment, Strategy
        >::type distance_type;

    static inline distance_type apply(MultiPolygon const& mp,
                                      Segment const& segment,
                                      Strategy const& strategy)
    {
        return detail::distance_brute_force::one_to_many
            <
                distance_brute_force
                <
                    Segment,
                    typename boost::range_value<MultiPolygon>::type,
                    Strategy
                >
            >::apply(segment, boost::begin(mp), boost::end(mp), strategy);
    }
};

//========================================================================

template
<
    typename Segment1,
    typename Segment2,
    typename Strategy
>
struct distance_brute_force
<
    Segment1, Segment2, Strategy,
    segment_tag, segment_tag, false
> : detail::distance_brute_force::distance_from_bg
{};

template
<
    typename Segment,
    typename Ring,
    typename Strategy
>
struct distance_brute_force
<
    Segment, Ring, Strategy,
    segment_tag, ring_tag, false
>
{
    typedef typename distance_result
        <
            Segment, Ring, Strategy
        >::type distance_type;

    static inline distance_type apply(Segment const& segment,
                                      Ring const& ring,
                                      Strategy const& strategy)
    {
        return detail::distance_brute_force::one_to_many
            <
                distance_brute_force
                    <
                        Segment,
                        typename std::iterator_traits
                            <
                                segment_iterator<Ring const>
                            >::value_type,
                        Strategy
                    >
            >::apply(segment,
                     geometry::segments_begin(ring),
                     geometry::segments_end(ring),
                     strategy);
    }
};

template
<
    typename Segment,
    typename Box,
    typename Strategy
>
struct distance_brute_force
<
    Segment, Box, Strategy,
    segment_tag, box_tag, false
> : detail::distance_brute_force::distance_from_bg
{};

//====================================================================

template
<
    typename Box1,
    typename Box2,
    typename Strategy
>
struct distance_brute_force
<
    Box1, Box2, Strategy,
    box_tag, box_tag, false
> : detail::distance_brute_force::distance_from_bg
{};

} // namespace dispatch





template <typename Geometry1, typename Geometry2, typename Strategy>
inline typename distance_result<Geometry1, Geometry2, Strategy>::type
distance_brute_force(Geometry1 const& geometry1,
                     Geometry2 const& geometry2,
                     Strategy const& strategy)
{
    return dispatch::distance_brute_force
        <
            Geometry1, Geometry2, Strategy
        >::apply(geometry1, geometry2, strategy);
}

} // namespace unit_test


}} // namespace boost::geometry
#endif // BOOST_GEOMETRY_TEST_DISTANCE_BRUTE_FORCE_HPP
