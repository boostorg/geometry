// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.Index - maxdist used in R-tree k nearest neighbors query
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_ALGORITHMS_MAXDIST_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_ALGORITHMS_MAXDIST_HPP

#include <boost/geometry/extensions/index/algorithms/detail/diff_abs.hpp>
#include <boost/geometry/extensions/index/algorithms/detail/sum_for_indexable.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail {

// minmaxdist component

struct maxdist_tag {};

template <
    typename Point,
    typename BoxIndexable,
    size_t DimensionIndex>
struct sum_for_indexable_dimension<Point, BoxIndexable, box_tag, maxdist_tag, DimensionIndex>
{
    typedef typename geometry::default_distance_result<Point, BoxIndexable>::type result_type;

    inline static result_type apply(Point const& pt, BoxIndexable const& i)
    {
        typedef typename index::traits::coordinate_type<Point>::type point_coord_t;
        typedef typename index::traits::coordinate_type<BoxIndexable>::type indexable_coord_t;

        point_coord_t pt_c = geometry::get<DimensionIndex>(pt);
        indexable_coord_t ind_c_min = geometry::get<geometry::min_corner, DimensionIndex>(i);
        indexable_coord_t ind_c_max = geometry::get<geometry::max_corner, DimensionIndex>(i);

        result_type further_diff = 0;

        if ( (ind_c_min + ind_c_max) / 2 <= pt_c )
            further_diff = pt_c - ind_c_min;
        else
            further_diff = detail::diff_abs(pt_c, ind_c_max); // unsigned values protection

        return further_diff * further_diff;
    }
};

} // namespace detail

template <typename Point, typename Indexable>
typename geometry::default_distance_result<Point, Indexable>::type
maxdist(Point const& pt, Indexable const& i)
{
    return detail::sum_for_indexable<
        Point,
        Indexable,
        typename index::traits::tag<Indexable>::type,
        detail::maxdist_tag,
        index::traits::dimension<Indexable>::value
    >::apply(pt, i);
}

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_ALGORITHMS_MAXDIST_HPP
