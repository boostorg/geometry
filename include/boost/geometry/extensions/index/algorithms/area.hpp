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

template <typename Box>
struct area_result
{
    typedef typename select_most_precise<
        typename coordinate_type<Box>::type,
        long double
    >::type type;
};

namespace detail {

template <typename Box, size_t CurrentDimension>
struct area_for_each_dimension
{
    BOOST_STATIC_ASSERT(0 < CurrentDimension);
    BOOST_STATIC_ASSERT(CurrentDimension <= traits::dimension<Box>::value);

    static inline typename area_result<Box>::type apply(Box const& b)
    {
        return area_for_each_dimension<Box, CurrentDimension - 1>::apply(b) *
            ( geometry::get<max_corner, CurrentDimension - 1>(b) - geometry::get<min_corner, CurrentDimension - 1>(b) );
    }
};

template <typename Box>
struct area_for_each_dimension<Box, 1>
{
    static inline typename area_result<Box>::type apply(Box const& b)
    {
        return geometry::get<max_corner, 0>(b) - geometry::get<min_corner, 0>(b);
    }
};

} // namespace detail

template <typename Box>
typename area_result<Box>::type area(Box const& b)
{
    return detail::area_for_each_dimension<Box, traits::dimension<Box>::value>::apply(b);
}

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_ALGORITHMS_AREA_HPP
