// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ITERATORS_SEGMENT_ITERATOR_TYPE_HPP
#define BOOST_GEOMETRY_ITERATORS_SEGMENT_ITERATOR_TYPE_HPP

#include <boost/range.hpp>

#include <boost/geometry/core/interior_type.hpp>
#include <boost/geometry/core/point_type.hpp>
#include <boost/geometry/core/ring_type.hpp>
#include <boost/geometry/core/tags.hpp>

#include <boost/geometry/iterators/concatenate_iterator.hpp>
#include <boost/geometry/iterators/flatten_iterator.hpp>
#include <boost/geometry/iterators/point_iterator_type.hpp>
#include <boost/geometry/iterators/detail/segment_iterator/range_segment_iterator.hpp>
#include <boost/geometry/iterators/detail/segment_iterator/value_type.hpp>

#include <boost/geometry/geometries/pointing_segment.hpp>

#include <boost/geometry/iterators/dispatch/segment_iterator_type.hpp>
#include <boost/geometry/iterators/dispatch/segment_iterator.hpp>


namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


template <typename Linestring>
struct segment_iterator_type<Linestring, linestring_tag>
{
    typedef detail::segment_iterator::range_segment_iterator
        <
            Linestring,
            typename detail::segment_iterator::value_type<Linestring>::type
        > type;
};


template <typename Ring>
struct segment_iterator_type<Ring, ring_tag>
{
    typedef detail::segment_iterator::range_segment_iterator
        <
            Ring,
            typename detail::segment_iterator::value_type<Ring>::type
        > type;
};


template <typename Polygon>
class segment_iterator_type<Polygon, polygon_tag>
{
private:
    typedef typename detail_dispatch::point_iterator_inner_range_type
        <
            Polygon
        >::type inner_range;

    typedef typename detail::segment_iterator::value_type
        <
            Polygon
        >::type value_type;

public:
    typedef concatenate_iterator
        <
            detail::segment_iterator::range_segment_iterator
                <
                    inner_range, value_type
                >,
            flatten_iterator
                <
                    typename boost::range_iterator
                        <
                            typename geometry::interior_type<Polygon>::type
                        >::type,
                    typename dispatch::segment_iterator_type<inner_range>::type,
                    value_type,
                    dispatch::segments_begin<inner_range>,
                    dispatch::segments_end<inner_range>,
                    value_type
                >,
            value_type,
            value_type
        > type;
};


template <typename MultiLinestring>
class segment_iterator_type<MultiLinestring, multi_linestring_tag>
{
private:
    typedef typename detail_dispatch::point_iterator_inner_range_type
        <
            MultiLinestring
        >::type inner_range;

    typedef typename detail::segment_iterator::value_type
        <
            MultiLinestring
        >::type value_type;

public:
    typedef flatten_iterator
        <
            typename boost::range_iterator<MultiLinestring>::type,
            typename dispatch::segment_iterator_type<inner_range>::type,
            value_type,
            dispatch::segments_begin<inner_range>,
            dispatch::segments_end<inner_range>,
            value_type
        > type;
};


template <typename MultiPolygon>
class segment_iterator_type<MultiPolygon, multi_polygon_tag>
{
private:
    typedef typename detail_dispatch::point_iterator_inner_range_type
        <
            MultiPolygon
        >::type inner_range;

    typedef typename detail::segment_iterator::value_type
        <
            MultiPolygon
        >::type value_type;

public:
    typedef flatten_iterator
        <
            typename boost::range_iterator<MultiPolygon>::type,
            typename dispatch::segment_iterator_type<inner_range>::type,
            value_type,
            dispatch::segments_begin<inner_range>,
            dispatch::segments_end<inner_range>,
            value_type
        > type;
};



} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ITERATORS_SEGMENT_ITERATOR_TYPE_HPP
