// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_CORE_POINT_ITERATOR_TYPE_HPP
#define BOOST_GEOMETRY_CORE_POINT_ITERATOR_TYPE_HPP

#include <boost/geometry/core/dispatch/point_iterator_type.hpp>
#include <boost/geometry/core/dispatch/point_iterator.hpp>

#include <boost/range.hpp>
#include <boost/geometry/core/point_type.hpp>
#include <boost/geometry/core/tags.hpp>
#include <boost/geometry/multi/core/tags.hpp>
#include <boost/geometry/util/flatten_iterator.hpp>
#include <boost/geometry/util/concatenate_iterator.hpp>


namespace boost { namespace geometry
{



#ifndef DOXYGEN_NO_DETAIL
namespace core_detail
{


template <typename Geometry>
struct point_iterator_value_type
{
    typedef typename boost::mpl::if_c
        <
            !boost::is_const<Geometry>::type::value,
            typename geometry::point_type<Geometry>::type,
            typename geometry::point_type<Geometry>::type const
        >::type type;
};




template
<
    typename Geometry, 
    typename Tag = typename tag<Geometry>::type
>
struct point_iterator_inner_range_type
{
    typedef typename boost::mpl::if_c
        <
            !boost::is_const<Geometry>::type::value,
            typename boost::range_value<Geometry>::type,
            typename boost::range_value<Geometry>::type const
        >::type type;
};


template <typename Polygon>
struct point_iterator_inner_range_type<Polygon, polygon_tag>
{
    typedef typename boost::mpl::if_c
        <
            !boost::is_const<Polygon>::type::value,
            typename geometry::ring_type<Polygon>::type,
            typename geometry::ring_type<Polygon>::type const
        >::type type;
};



} // namespace core_detail
#endif // DOXYGEN_NO_DETAIL





#ifndef DOXYGEN_NO_DISPATCH
namespace core_dispatch
{


// linestring
template <typename Linestring>
struct point_iterator_type<Linestring, linestring_tag>
{
    typedef typename boost::range_iterator<Linestring>::type type;
};


// ring
template <typename Ring>
struct point_iterator_type<Ring, ring_tag>
{
    typedef typename boost::range_iterator<Ring>::type type;
};


// polygon
template <typename Polygon>
class point_iterator_type<Polygon, polygon_tag>
{
private:
    typedef typename core_detail::point_iterator_inner_range_type
        <
            Polygon
        >::type InnerRange;

public:
    typedef util::concatenate_iterator
        <
            typename boost::range_iterator<InnerRange>::type,
            util::flatten_iterator
                <
                    typename boost::range_iterator
                        <
                            typename geometry::interior_type<Polygon>::type
                        >::type,
                    typename core_dispatch::point_iterator_type
                        <
                            InnerRange
                        >::type,
                    typename core_detail::point_iterator_value_type
                        <
                            Polygon
                        >::type,
                    core_dispatch::points_begin<InnerRange>,
                    core_dispatch::points_end<InnerRange>
                >,
            typename core_detail::point_iterator_value_type<Polygon>::type
        > type;
};


// multi-point
template <typename MultiPoint>
struct point_iterator_type<MultiPoint, multi_point_tag>
{
    typedef typename boost::range_iterator<MultiPoint>::type type;
};


// multi-linestring
template <typename MultiLinestring>
class point_iterator_type<MultiLinestring, multi_linestring_tag>
{
private:
    typedef typename core_detail::point_iterator_inner_range_type
        <
            MultiLinestring
        >::type InnerRange;

public:
    typedef util::flatten_iterator
        <
            typename boost::range_iterator<MultiLinestring>::type,
            typename core_dispatch::point_iterator_type<InnerRange>::type,
            typename core_detail::point_iterator_value_type
                <
                    MultiLinestring
                >::type,
            core_dispatch::points_begin<InnerRange>,
            core_dispatch::points_end<InnerRange>
        > type;
};


// multi-polygon
template <typename MultiPolygon>
class point_iterator_type<MultiPolygon, multi_polygon_tag>
{
private:
    typedef typename core_detail::point_iterator_inner_range_type
        <
            MultiPolygon
        >::type InnerRange;

public:
    typedef util::flatten_iterator
        <
            typename boost::range_iterator<MultiPolygon>::type,
            typename core_dispatch::point_iterator_type<InnerRange>::type,
            typename core_detail::point_iterator_value_type<MultiPolygon>::type,
            core_dispatch::points_begin<InnerRange>,
            core_dispatch::points_end<InnerRange>
        > type;
};





} // namespace core_dispatch
#endif // DOXYGEN_NO_DISPATCH



// MK::need to add doc here
template <typename Geometry>
struct point_iterator_type
{
    typedef typename core_dispatch::point_iterator_type<Geometry>::type type;
};



}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_CORE_POINT_ITERATOR_TYPE_HPP
