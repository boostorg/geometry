// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ITERATORS_SEGMENT_ITERATOR_HPP
#define BOOST_GEOMETRY_ITERATORS_SEGMENT_ITERATOR_HPP

#include <boost/mpl/assert.hpp>
#include <boost/type_traits/is_convertible.hpp>
#include <boost/range.hpp>

#include <boost/geometry/core/exterior_ring.hpp>
#include <boost/geometry/core/interior_rings.hpp>
#include <boost/geometry/core/tags.hpp>

#include <boost/geometry/iterators/point_iterator_type.hpp>
#include <boost/geometry/iterators/segment_iterator_type.hpp>

#include <boost/geometry/iterators/dispatch/segment_iterator.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


// specializations for points_begin


template <typename Linestring>
struct segments_begin<Linestring, linestring_tag>
{
    typedef typename segment_iterator_type<Linestring>::type result_type;

    static inline result_type apply(Linestring& linestring)
    {
        return result_type(linestring);
    }
};


template <typename Ring>
struct segments_begin<Ring, ring_tag>
{
    typedef typename segment_iterator_type<Ring>::type result_type;

    static inline result_type apply(Ring& ring)
    {
        return result_type(ring);
    }
};


template <typename Polygon>
struct segments_begin<Polygon, polygon_tag>
{
    typedef typename detail_dispatch::point_iterator_inner_range_type
        <
            Polygon
        >::type inner_range;

    typedef typename segment_iterator_type<Polygon>::type return_type;

    static inline return_type apply(Polygon& polygon)
    {
        typedef typename return_type::second_iterator_type flatten_iterator;

        return return_type
            (segments_begin
                 <
                     inner_range
                 >::apply(geometry::exterior_ring(polygon)),
             segments_end<inner_range>::apply(geometry::exterior_ring(polygon)),
             flatten_iterator(boost::begin(geometry::interior_rings(polygon)),
                              boost::end(geometry::interior_rings(polygon))
                              ),
             flatten_iterator(boost::begin(geometry::interior_rings(polygon)),
                              boost::end(geometry::interior_rings(polygon))
                              )
             );
    }
};


template <typename MultiLinestring>
struct segments_begin<MultiLinestring, multi_linestring_tag>
{
    typedef typename segment_iterator_type<MultiLinestring>::type result_type;

    static inline result_type apply(MultiLinestring& multilinestring)
    {
        return result_type(boost::begin(multilinestring),
                           boost::end(multilinestring));
    }
};


template <typename MultiPolygon>
struct segments_begin<MultiPolygon, multi_polygon_tag>
{
    typedef typename segment_iterator_type<MultiPolygon>::type result_type;

    static inline result_type apply(MultiPolygon& multipolygon)
    {
        return result_type(boost::begin(multipolygon),
                           boost::end(multipolygon));
    }
};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH





#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


// specializations for points_end


template <typename Linestring>
struct segments_end<Linestring, linestring_tag>
{
    typedef typename segment_iterator_type<Linestring>::type result_type;

    static inline result_type apply(Linestring& linestring)
    {
        return result_type(linestring, true);
    }
};


template <typename Ring>
struct segments_end<Ring, ring_tag>
{
    typedef typename segment_iterator_type<Ring>::type result_type;

    static inline result_type apply(Ring& ring)
    {
        return result_type(ring, true);
    }
};


template <typename Polygon>
struct segments_end<Polygon, polygon_tag>
{
    typedef typename detail_dispatch::point_iterator_inner_range_type
        <
            Polygon
        >::type inner_range;

    typedef typename segment_iterator_type<Polygon>::type return_type;

    static inline return_type apply(Polygon& polygon)
    {
        typedef typename return_type::second_iterator_type flatten_iterator;

        return return_type
            (segments_end<inner_range>::apply(geometry::exterior_ring(polygon)),
             flatten_iterator(boost::begin(geometry::interior_rings(polygon)),
                              boost::end(geometry::interior_rings(polygon))
                              ),
             flatten_iterator( boost::end(geometry::interior_rings(polygon)) )
             );
    }
};


template <typename MultiLinestring>
struct segments_end<MultiLinestring, multi_linestring_tag>
{
    typedef typename segment_iterator_type<MultiLinestring>::type result_type;

    static inline result_type apply(MultiLinestring& multilinestring)
    {
        return result_type(boost::end(multilinestring));
    }
};


template <typename MultiPolygon>
struct segments_end<MultiPolygon, multi_polygon_tag>
{
    typedef typename segment_iterator_type<MultiPolygon>::type result_type;
    static inline result_type apply(MultiPolygon& multipolygon)
    {
        return result_type(boost::end(multipolygon));
    }
};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


// MK:: need to add doc here
template <typename Geometry>
class segment_iterator
    : public dispatch::segment_iterator_type<Geometry const>::type
{
private:
    typedef typename dispatch::segment_iterator_type<Geometry const>::type base;

    base const* base_ptr() const
    {
        return this;
    }

    template <typename OtherGeometry> friend class segment_iterator;

    template <typename G>
    friend inline segment_iterator<G> segments_begin(G const&);

    template <typename G>
    friend inline segment_iterator<G> segments_end(G const&);

    segment_iterator(base const& base_it) : base(base_it) {}

public:
    segment_iterator() {}

    template <typename OtherGeometry>
    segment_iterator(segment_iterator<OtherGeometry> const& other)
        : base(*other.base_ptr())
    {
        static const bool is_conv
            = boost::is_convertible<
                typename dispatch::segment_iterator_type
                    <
                        OtherGeometry const
                    >::type,
                typename dispatch::segment_iterator_type<Geometry const>::type
            >::value;

        BOOST_MPL_ASSERT_MSG((is_conv),
                             NOT_CONVERTIBLE,
                             (segment_iterator<OtherGeometry const>));
    }
};


// MK:: need to add doc here
template <typename Geometry>
inline segment_iterator<Geometry>
segments_begin(Geometry const& geometry)
{
    return dispatch::segments_begin<Geometry const>::apply(geometry);
}


// MK:: need to add doc here
template <typename Geometry>
inline segment_iterator<Geometry>
segments_end(Geometry const& geometry)
{
    return dispatch::segments_end<Geometry const>::apply(geometry);
}


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ITERATORS_SEGMENT_ITERATOR_HPP
