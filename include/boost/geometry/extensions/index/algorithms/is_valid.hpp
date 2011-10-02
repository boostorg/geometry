// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.SpatialIndex - n-dimensional box's / point validity check
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_ALGORITHMS_IS_VALID_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_ALGORITHMS_IS_VALID_HPP

namespace boost { namespace geometry { namespace index {

namespace detail {

template <typename Box, size_t Dimension>
struct is_valid_box
{
    BOOST_MPL_ASSERT_MSG(
        (0 < Dimension && Dimension <= index::traits::dimension<Box>::value),
        INVALID_DIMENSION_PARAMETER,
        (is_valid_box));

    static inline bool apply(Box const& b)
    {
        return is_valid_box<Box, Dimension - 1>::apply(b) &&
            ( index::get<min_corner, Dimension - 1>(b) <= index::get<max_corner, Dimension - 1>(b) );
    }
};

template <typename Box>
struct is_valid_box<Box, 1>
{
    static inline bool apply(Box const& b)
    {
        return index::get<min_corner, 0>(b) <= index::get<max_corner, 0>(b);
    }
};

} // namespace detail

namespace dispatch {

template <typename Indexable, typename Tag>
struct is_valid
{
    BOOST_MPL_ASSERT_MSG(
        (false),
        NOT_IMPLEMENTED_FOR_THIS_INDEXABLE,
        (is_valid));
};

template <typename Indexable>
struct is_valid<Indexable, point_tag>
{
    static inline bool apply(Indexable const&)
    {
        return true;
    }
};

template <typename Indexable>
struct is_valid<Indexable, box_tag>
{
    static inline bool apply(Indexable const& b)
    {
        return detail::is_valid_box<Indexable, index::traits::dimension<Indexable>::value>::apply(b);
    }
};

} // namespace dispatch

template <typename Indexable>
inline bool is_valid(Indexable const& b)
{
    return dispatch::is_valid<Indexable, typename index::traits::tag<Indexable>::type>::apply(b);
}

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_ALGORITHMS_IS_VALID_HPP
