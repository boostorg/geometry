// Boost.Geometry Index
//
// n-dimensional box-linestring intersection
//
// Copyright (c) 2011-2013 Adam Wulkiewicz, Lodz, Poland.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_INDEX_DETAIL_ALGORITHMS_PATH_INTERSECTION_HPP
#define BOOST_GEOMETRY_INDEX_DETAIL_ALGORITHMS_PATH_INTERSECTION_HPP

#include <boost/geometry/index/detail/algorithms/segment_intersection.hpp>

namespace boost { namespace geometry { namespace index { namespace detail {

namespace dispatch {

template <typename Indexable, typename LineString, typename Tag>
struct path_intersection
{
    BOOST_MPL_ASSERT_MSG((false), NOT_IMPLEMENTED_FOR_THIS_GEOMETRY, (path_intersection));
};

template <typename Indexable, typename Linestring>
struct path_intersection<Indexable, Linestring, point_tag>
{
    BOOST_MPL_ASSERT_MSG((false), SEGMENT_POINT_INTERSECTION_UNAVAILABLE, (path_intersection));
};

template <typename Indexable, typename Linestring>
struct path_intersection<Indexable, Linestring, box_tag>
{
    typedef typename default_length_result<Linestring>::type length_type;

    static inline bool apply(Indexable const& b, Linestring const& path, length_type & distance)
    {
        typedef typename ::boost::range_value<Linestring>::type point_type;
        typedef typename default_relative_distance_type<Indexable, point_type>::type relative_distance_type;
        typedef typename ::boost::range_const_iterator<Linestring>::type const_iterator;        

        if ( ::boost::size(path) < 2 )
            return false;

        const_iterator it0 = ::boost::begin(path);
        const_iterator it1 = ::boost::begin(path) + 1;
        const_iterator last = ::boost::end(path);

        distance = 0;

        for ( ; it1 != last ; ++it0, ++it1 )
        {
            typename default_distance_result<point_type, point_type>::type
                dist = geometry::distance(*it0, *it1);

            relative_distance_type rel_dist;
            if ( index::detail::segment_intersection(b, *it0, *it1, rel_dist) )
            {
                distance += dist * rel_dist;
                return true;
            }
            else
                distance += dist;
        }

        return false;
    }
};

} // namespace dispatch

// TODO - change the name e.g. to path_intersection_distance
//        and segment_intersection e.g. to segment_intersection_relative_distance

template <typename Indexable, typename Linestring> inline
bool path_intersection(Indexable const& b,
                       Linestring const& path,
                       typename default_length_result<Linestring>::type & distance)
{
    // TODO check Indexable and Linestring concepts

    return dispatch::path_intersection<
            Indexable, Linestring, typename detail::traits::tag<Indexable>::type
        >::apply(b, path, distance);
}

}}}} // namespace boost::geometry::index::detail

#endif // BOOST_GEOMETRY_INDEX_DETAIL_ALGORITHMS_PATH_INTERSECTION_HPP
