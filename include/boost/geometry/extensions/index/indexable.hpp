// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.SpatialIndex - indexable traits and functions
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_INDEXABLE_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_INDEXABLE_HPP

namespace boost { namespace geometry { namespace index {

namespace dispatch {

// Distinguish between indexables and other geometries

template <typename Geometry, typename GeometryTag>
struct indexable_type
{
    typedef void type;
};

template <typename Point>
struct indexable_type<Point, geometry::point_tag>
{
    typedef Point type;
};

template <typename Box>
struct indexable_type<Box, geometry::box_tag>
{
    typedef Box type;
};

} // namespace dispatch

namespace traits
{

template <typename Indexable>
struct indexable_type
{
    typedef typename dispatch::indexable_type<
        Indexable,
        typename geometry::traits::tag<Indexable>::type
    >::type type;
};

} // namespace traits

namespace dispatch {

template <typename Indexable, typename IndexableTag>
struct point_type
{
    typedef void type;
};

template <typename Indexable>
struct point_type<Indexable, geometry::box_tag>
{
    typedef typename geometry::traits::point_type<Indexable>::type type;
};

template <typename Indexable>
struct point_type<Indexable, geometry::point_tag>
{
    typedef Indexable type;
};

} // namespace dispatch

namespace traits {

template <typename Indexable>
struct point_type
{
    typedef typename dispatch::point_type<
        Indexable,
        typename geometry::traits::tag<Indexable>::type
    >::type type;
};

template <typename Indexable>
struct coordinate_system
{
    typedef typename geometry::traits::coordinate_system<
        typename point_type<Indexable>::type
    >::type type;
};

template <typename Indexable>
struct coordinate_type
{
    typedef typename geometry::traits::coordinate_type<
        typename point_type<Indexable>::type
    >::type type;
};

template <typename Indexable>
struct dimension
{
    static const size_t value =
        geometry::traits::dimension<
            typename point_type<Indexable>::type
        >::value;
};

} // namespace traits

namespace dispatch {

template <size_t Corner, size_t DimensionIndex, typename Indexable, typename IndexableTag>
struct indexable_indexed_access {};

template <size_t Corner, size_t DimensionIndex, typename Indexable>
struct indexable_indexed_access<Corner, DimensionIndex, Indexable, box_tag>
{
    typedef typename traits::point_type<Indexable>::type point_type;
    typedef typename traits::coordinate_type<point_type>::type coordinate_type;

    static inline coordinate_type get(Indexable const& i)
    {
        return geometry::get<Corner, DimensionIndex>(i);
    }
};

template <size_t Corner, size_t DimensionIndex, typename Indexable>
struct indexable_indexed_access<Corner, DimensionIndex, Indexable, point_tag>
{
    typedef typename traits::coordinate_type<Indexable>::type coordinate_type;

    static inline coordinate_type get(Indexable const& i)
    {
        return geometry::get<DimensionIndex>(i);
    }
};

} // namespace dispatch

namespace detail {

template <size_t Corner, size_t DimensionIndex, typename Indexable>
typename traits::coordinate_type<Indexable>::type get(Indexable const& i)
{
    return dispatch::indexable_indexed_access<
        Corner,
        DimensionIndex,
        Indexable,
        typename geometry::traits::tag<Indexable>::type
    >::get(i);
}

} // namespace detail

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_INDEXABLE_HPP
