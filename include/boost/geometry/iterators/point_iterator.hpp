// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_CORE_POINT_ITERATOR_HPP
#define BOOST_GEOMETRY_CORE_POINT_ITERATOR_HPP

#include <boost/geometry/core/dispatch/point_iterator.hpp>
#include <boost/geometry/core/point_iterator_type.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DISPATCH
namespace core_dispatch
{


// specializations for points_begin


// linestring
template <typename Linestring>
struct points_begin<Linestring, linestring_tag>
{
    static inline typename point_iterator_type<Linestring>::type
    apply(Linestring& linestring)
    {
        return boost::begin(linestring);
    }
};


// ring
template <typename Ring>
struct points_begin<Ring, ring_tag>
{
    static inline typename point_iterator_type<Ring>::type
    apply(Ring& ring)
    {
        return boost::begin(ring);
    }
};


// polygon
template <typename Polygon>
struct points_begin<Polygon, polygon_tag>
{
    typedef typename point_iterator_type<Polygon>::type return_type;

    static inline return_type apply(Polygon& polygon)
    {
        typedef typename return_type::second_iterator_type flatten_iterator;

        return return_type
            (boost::begin(geometry::exterior_ring(polygon)),
             boost::end(geometry::exterior_ring(polygon)),
             flatten_iterator(boost::begin(geometry::interior_rings(polygon)),
                              boost::end(geometry::interior_rings(polygon))
                              )
             );
    }
};


// multi-point
template <typename MultiPoint>
struct points_begin<MultiPoint, multi_point_tag>
{
    static inline typename point_iterator_type<MultiPoint>::type
    apply(MultiPoint& multipoint)
    {
        return boost::begin(multipoint);
    }
};


// multi-linestring
template <typename MultiLinestring>
struct points_begin<MultiLinestring, multi_linestring_tag>
{
    static inline typename point_iterator_type<MultiLinestring>::type
    apply(MultiLinestring& multilinestring)
    {
        return typename point_iterator_type
            <
                MultiLinestring
            >::type(boost::begin(multilinestring), boost::end(multilinestring));
    }
};


// multi-polygon
template <typename MultiPolygon>
struct points_begin<MultiPolygon, multi_polygon_tag>
{
    static inline typename point_iterator_type<MultiPolygon>::type
    apply(MultiPolygon& multipolygon)
    {
        return typename point_iterator_type
            <
                MultiPolygon
            >::type(boost::begin(multipolygon), boost::end(multipolygon));
    }
};

} // namespace core_dispatch
#endif // DOXYGEN_NO_DISPATCH





#ifndef DOXYGEN_NO_DISPATCH
namespace core_dispatch
{


// specializations for points_end


// linestring
template <typename Linestring>
struct points_end<Linestring, linestring_tag>
{
    static inline typename point_iterator_type<Linestring>::type
    apply(Linestring& linestring)
    {
        return boost::end(linestring);
    }
};


// ring
template <typename Ring>
struct points_end<Ring, ring_tag>
{
    static inline typename point_iterator_type<Ring>::type
    apply(Ring& ring)
    {
        return boost::end(ring);
    }
};


// polygon
template <typename Polygon>
struct points_end<Polygon, polygon_tag>
{
    typedef typename point_iterator_type<Polygon>::type return_type;

    static inline return_type apply(Polygon& polygon)
    {
        typedef typename return_type::second_iterator_type flatten_iterator;

        return return_type
            (boost::end(geometry::exterior_ring(polygon)),
             flatten_iterator( boost::end(geometry::interior_rings(polygon)) )
             );
    }
};


// multi-point
template <typename MultiPoint>
struct points_end<MultiPoint, multi_point_tag>
{
    static inline typename point_iterator_type<MultiPoint>::type
    apply(MultiPoint& multipoint)
    {
        return boost::end(multipoint);
    }
};


// multi-linestring
template <typename MultiLinestring>
struct points_end<MultiLinestring, multi_linestring_tag>
{
    static inline typename point_iterator_type<MultiLinestring>::type
    apply(MultiLinestring& multilinestring)
    {
        return typename point_iterator_type
            <
                MultiLinestring
            >::type(boost::end(multilinestring));
    }
};


// multi-polygon
template <typename MultiPolygon>
struct points_end<MultiPolygon, multi_polygon_tag>
{
    static inline typename point_iterator_type<MultiPolygon>::type
    apply(MultiPolygon& multipolygon)
    {
        return typename point_iterator_type
            <
                MultiPolygon
            >::type(boost::end(multipolygon));
    }
};


} // namespace core_dispatch
#endif // DOXYGEN_NO_DISPATCH




// MK:: need to add doc here
template <typename Geometry>
inline typename point_iterator_type<Geometry>::type
points_begin(Geometry& geometry)
{
    return core_dispatch::points_begin<Geometry>::apply(geometry);
}


// MK:: need to add doc here
template <typename Geometry>
inline typename point_iterator_type<Geometry>::type
points_end(Geometry& geometry)
{
    return core_dispatch::points_end<Geometry>::apply(geometry);
}




}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_CORE_POINT_ITERATOR_HPP
