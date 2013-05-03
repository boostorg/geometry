// Boost.Geometry Index
//
// n-dimensional box-segment intersection
//
// Copyright (c) 2011-2013 Adam Wulkiewicz, Lodz, Poland.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_INDEX_DETAIL_ALGORITHMS_SEGMENT_INTERSECTION_HPP
#define BOOST_GEOMETRY_INDEX_DETAIL_ALGORITHMS_SEGMENT_INTERSECTION_HPP

#include <boost/geometry/index/detail/indexable.hpp>

namespace boost { namespace geometry { namespace index { namespace detail {

template <typename Indexable, typename Point>
struct default_relative_distance_type
{
    typedef typename select_most_precise<
        typename select_most_precise<
            typename traits::coordinate_type<Indexable>::type,
            typename traits::coordinate_type<Point>::type
        >::type,
        float // TODO - use bigger type, calculated from the size of coordinate types
    >::type type;


    BOOST_MPL_ASSERT_MSG((!::boost::is_unsigned<type>::value),
        THIS_TYPE_SHOULDNT_BE_UNSIGNED, (type));
};

namespace dispatch {

template <typename Box, typename Point, size_t I>
struct box_segment_intersection_dim
{
    BOOST_STATIC_ASSERT(I < traits::dimension<Box>::value);
    BOOST_STATIC_ASSERT(I < traits::dimension<Point>::value);
    BOOST_STATIC_ASSERT(traits::dimension<Point>::value == traits::dimension<Box>::value);

    typedef typename default_relative_distance_type<Box, Point>::type relative_distance_type;

    // WARNING! - relative_distance_type must be IEEE float for this to work

    static inline bool apply(Box const& b, Point const& p0, Point const& p1,
                             relative_distance_type & t_near, relative_distance_type & t_far)
    {
        relative_distance_type ray_d = geometry::get<I>(p1) - geometry::get<I>(p0);
        relative_distance_type tn = ( detail::get<min_corner, I>(b) - geometry::get<I>(p0) ) / ray_d;
        relative_distance_type tf = ( detail::get<max_corner, I>(b) - geometry::get<I>(p0) ) / ray_d;
        if ( tf < tn )
            ::std::swap(tn, tf);

        if ( t_near < tn )
            t_near = tn;
        if ( tf < t_far )
            t_far = tf;

        return 0 <= t_far && t_near <= t_far;
    }
};

template <typename Box, typename Point, size_t CurrentDimension>
struct box_segment_intersection
{
    BOOST_STATIC_ASSERT(0 < CurrentDimension);

    typedef box_segment_intersection_dim<Box, Point, CurrentDimension - 1> for_dim;
    typedef typename for_dim::relative_distance_type relative_distance_type;

    static inline bool apply(Box const& b, Point const& p0, Point const& p1,
                             relative_distance_type & t_near, relative_distance_type & t_far)
    {
        return box_segment_intersection<Box, Point, CurrentDimension - 1>::apply(b, p0, p1, t_near, t_far)
            && for_dim::apply(b, p0, p1, t_near, t_far);
    }
};

template <typename Box, typename Point>
struct box_segment_intersection<Box, Point, 1>
{
    typedef box_segment_intersection_dim<Box, Point, 0> for_dim;
    typedef typename for_dim::relative_distance_type relative_distance_type;

    static inline bool apply(Box const& b, Point const& p0, Point const& p1,
                             relative_distance_type & t_near, relative_distance_type & t_far)
    {
        return for_dim::apply(b, p0, p1, t_near, t_far);
    }
};

template <typename Indexable, typename Point, typename Tag>
struct segment_intersection
{
    BOOST_MPL_ASSERT_MSG((false), NOT_IMPLEMENTED_FOR_THIS_GEOMETRY, (segment_intersection));
};

template <typename Indexable, typename Point>
struct segment_intersection<Indexable, Point, point_tag>
{
    BOOST_MPL_ASSERT_MSG((false), SEGMENT_POINT_INTERSECTION_UNAVAILABLE, (segment_intersection));
};

template <typename Indexable, typename Point>
struct segment_intersection<Indexable, Point, box_tag>
{
    typedef dispatch::box_segment_intersection<Indexable, Point, detail::traits::dimension<Indexable>::value> impl;
    typedef typename impl::relative_distance_type relative_distance_type;

    static inline bool apply(Indexable const& b, Point const& p0, Point const& p1, relative_distance_type & relative_distance)
    {
        relative_distance_type t_near = -(::std::numeric_limits<relative_distance_type>::max)();
        relative_distance_type t_far = (::std::numeric_limits<relative_distance_type>::max)();

        return impl::apply(b, p0, p1, t_near, t_far) &&
               (t_near <= 1) &&
               ( relative_distance = 0 < t_near ? t_near : 0, true );
    }
};

} // namespace dispatch

template <typename Indexable, typename Point> inline
bool segment_intersection(Indexable const& b,
                          Point const& p0,
                          Point const& p1,
                          typename default_relative_distance_type<Indexable, Point>::type & relative_distance)
{
    // TODO check Indexable and Point concepts

    return dispatch::segment_intersection<
            Indexable, Point,
            typename detail::traits::tag<Indexable>::type
        >::apply(b, p0, p1, relative_distance);
}

}}}} // namespace boost::geometry::index::detail

#endif // BOOST_GEOMETRY_INDEX_DETAIL_ALGORITHMS_SEGMENT_INTERSECTION_HPP
