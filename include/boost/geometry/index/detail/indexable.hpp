// Boost.Geometry Index
//
// Indexable's traits and related functions
//
// Copyright (c) 2011-2013 Adam Wulkiewicz, Lodz, Poland.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_INDEX_DETAIL_INDEXABLE_HPP
#define BOOST_GEOMETRY_INDEX_DETAIL_INDEXABLE_HPP

namespace boost { namespace geometry { namespace index { namespace detail {

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

template <typename Indexable>
struct tag
{
    typedef typename geometry::traits::tag<
        Indexable
    >::type type;
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

template <typename Indexable>
struct default_box_type
{
    typedef geometry::model::box<
        geometry::model::point<
            typename traits::coordinate_type<Indexable>::type,
            traits::dimension<Indexable>::value,
            typename traits::coordinate_system<Indexable>::type
        >
    > type;
};

}}}} // namespace boost::geometry::index::detail

#endif // BOOST_GEOMETRY_INDEX_DETAIL_INDEXABLE_HPP
