// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.Index - mindist used in R-tree k nearest neighbors query
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_ALGORITHMS_MINDIST_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_ALGORITHMS_MINDIST_HPP

#include <boost/geometry/extensions/index/algorithms/detail/sum_for_indexable.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail {

struct mindist_tag {};

template <
    typename Point,
    typename PointIndexable,
    size_t N>
struct sum_for_indexable<Point, PointIndexable, point_tag, mindist_tag, N>
{
    typedef typename geometry::default_distance_result<Point, PointIndexable>::type result_type;

    inline static result_type apply(Point const& pt, PointIndexable const& i)
    {
        return geometry::comparable_distance(pt, i);
    }
};

template <
    typename Point,
    typename BoxIndexable,
    size_t DimensionIndex>
struct sum_for_indexable_dimension<Point, BoxIndexable, box_tag, mindist_tag, DimensionIndex>
{
    typedef typename geometry::default_distance_result<Point, BoxIndexable>::type result_type;

    inline static result_type apply(Point const& pt, BoxIndexable const& i)
    {
        typedef typename index::traits::coordinate_type<Point>::type point_coord_t;
        typedef typename index::traits::coordinate_type<BoxIndexable>::type indexable_coord_t;

        point_coord_t pt_c = geometry::get<DimensionIndex>(pt);
        indexable_coord_t ind_c_min = geometry::get<geometry::min_corner, DimensionIndex>(i);
        indexable_coord_t ind_c_max = geometry::get<geometry::max_corner, DimensionIndex>(i);

        result_type diff = 0;

        if ( pt_c < ind_c_min )
            diff = ind_c_min - pt_c;
        else if ( ind_c_max < pt_c )
            diff = pt_c - ind_c_max;

        return diff * diff;
    }
};

} // namespace detail

template <typename Point, typename Indexable>
typename geometry::default_distance_result<Point, Indexable>::type
mindist(Point const& pt, Indexable const& i)
{
    return detail::sum_for_indexable<
        Point,
        Indexable,
        typename index::traits::tag<Indexable>::type,
        detail::mindist_tag,
        index::traits::dimension<Indexable>::value
    >::apply(pt, i);
}

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_ALGORITHMS_MINDIST_HPP
