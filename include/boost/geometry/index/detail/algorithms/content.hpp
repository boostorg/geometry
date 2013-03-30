// Boost.Geometry Index
//
// n-dimensional box's content (hypervolume) - 2d area, 3d volume, ...
//
// Copyright (c) 2011-2013 Adam Wulkiewicz, Lodz, Poland.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_INDEX_DETAIL_ALGORITHMS_CONTENT_HPP
#define BOOST_GEOMETRY_INDEX_DETAIL_ALGORITHMS_CONTENT_HPP

namespace boost { namespace geometry { namespace index { namespace detail {

template <typename Indexable>
struct default_content_result
{
    typedef typename select_most_precise<
        typename detail::traits::coordinate_type<Indexable>::type,
        long double
    >::type type;
};

namespace dispatch {

template <typename Box, size_t CurrentDimension>
struct content_for_each_dimension
{
    BOOST_STATIC_ASSERT(0 < CurrentDimension);
    BOOST_STATIC_ASSERT(CurrentDimension <= traits::dimension<Box>::value);

    static inline typename detail::default_content_result<Box>::type apply(Box const& b)
    {
        return content_for_each_dimension<Box, CurrentDimension - 1>::apply(b) *
            ( detail::get<max_corner, CurrentDimension - 1>(b) - detail::get<min_corner, CurrentDimension - 1>(b) );
    }
};

template <typename Box>
struct content_for_each_dimension<Box, 1>
{
    static inline typename detail::default_content_result<Box>::type apply(Box const& b)
    {
        return detail::get<max_corner, 0>(b) - detail::get<min_corner, 0>(b);
    }
};

template <typename Indexable, typename Tag>
struct content
{
    // TODO: awulkiew - static assert?
};

template <typename Indexable>
struct content<Indexable, point_tag>
{
    static typename detail::default_content_result<Indexable>::type apply(Indexable const&)
    {
        return 0;
    }
};

template <typename Indexable>
struct content<Indexable, box_tag>
{
    static typename default_content_result<Indexable>::type apply(Indexable const& b)
    {
        return dispatch::content_for_each_dimension<Indexable, detail::traits::dimension<Indexable>::value>::apply(b);
    }
};

} // namespace dispatch

template <typename Indexable>
typename default_content_result<Indexable>::type content(Indexable const& b)
{
    return dispatch::content<Indexable,
                             typename detail::traits::tag<Indexable>::type
                            >::apply(b);
}

}}}} // namespace boost::geometry::index::detail

#endif // BOOST_GEOMETRY_INDEX_DETAIL_ALGORITHMS_CONTENT_HPP
