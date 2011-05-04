// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.SpatialIndex - n-dimensional box's area/volume
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_ALGORITHMS_AREA_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_ALGORITHMS_AREA_HPP

namespace boost { namespace geometry { namespace index {

template <typename Indexable>
struct default_area_result
{
    typedef typename select_most_precise<
        typename traits::coordinate_type<Indexable>::type,
        long double
    >::type type;
};

namespace detail {

template <typename Box, size_t CurrentDimension>
struct area_for_each_dimension
{
    BOOST_STATIC_ASSERT(0 < CurrentDimension);
    BOOST_STATIC_ASSERT(CurrentDimension <= traits::dimension<Box>::value);

    static inline typename default_area_result<Box>::type apply(Box const& b)
    {
        return area_for_each_dimension<Box, CurrentDimension - 1>::apply(b) *
            ( index::get<max_corner, CurrentDimension - 1>(b) - index::get<min_corner, CurrentDimension - 1>(b) );
    }
};

template <typename Box>
struct area_for_each_dimension<Box, 1>
{
    static inline typename default_area_result<Box>::type apply(Box const& b)
    {
        return index::get<max_corner, 0>(b) - index::get<min_corner, 0>(b);
    }
};

} // namespace detail

namespace dispatch {

template <typename Indexable, typename Tag>
struct area
{
    // TODO: awulkiew - static assert?
};

template <typename Indexable>
struct area<Indexable, point_tag>
{
    static typename default_area_result<Indexable>::type apply(Indexable const&)
    {
        return 0;
    }
};

template <typename Indexable>
struct area<Indexable, box_tag>
{
    static typename default_area_result<Indexable>::type apply(Indexable const& b)
    {
        return detail::area_for_each_dimension<Indexable, traits::dimension<Indexable>::value>::apply(b);
    }
};

} // namespace dispatch

template <typename Indexable>
typename default_area_result<Indexable>::type area(Indexable const& b)
{
    return dispatch::area<Indexable, typename index::traits::tag<Indexable>::type>::apply(b);
}

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_ALGORITHMS_AREA_HPP
