// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ALGORITHMS_OGC_IS_VALID_HPP
#define BOOST_GEOMETRY_ALGORITHMS_OGC_IS_VALID_HPP

#include <boost/range.hpp>

#include <boost/geometry/core/closure.hpp>
#include <boost/geometry/core/exterior_ring.hpp>
#include <boost/geometry/core/interior_rings.hpp>
#include <boost/geometry/core/ring_type.hpp>
#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tags.hpp>
#include <boost/geometry/multi/core/tags.hpp>

#include <boost/geometry/algorithms/intersects.hpp>
#include <boost/geometry/algorithms/disjoint.hpp>

#include <boost/geometry/algorithms/ogc/dispatch/is_valid.hpp>

#include <boost/geometry/algorithms/ogc/detail/is_valid/box.hpp>
#include <boost/geometry/algorithms/ogc/detail/is_valid/segment.hpp>
#include <boost/geometry/algorithms/ogc/detail/is_valid/linestring.hpp>
#include <boost/geometry/algorithms/ogc/detail/is_valid/multi_range.hpp>
#include <boost/geometry/algorithms/ogc/detail/is_valid/pointlike.hpp>
#include <boost/geometry/algorithms/ogc/detail/is_valid/has_spikes.hpp>


namespace boost { namespace geometry { namespace ogc
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace is_valid
{




template <typename Polygon>
struct is_valid_polygon
{
    static inline bool apply(Polygon const& polygon)
    {
        typedef typename ring_type<Polygon>::type ring_type;

        if ( !dispatch::is_valid<ring_type>::apply(exterior_ring(polygon)) )
        {
            return false;
        }

        if ( !is_valid_multi_range<ring_type>::apply
             (boost::begin(geometry::interior_rings(polygon)),
              boost::end(geometry::interior_rings(polygon))) )
        {
            return false;
        }

        // MK::need check for self-intersection points; not done yet
        return true;
    }
};



template <typename MultiPolygon>
struct is_valid_multipolygon
{
    static inline bool apply(MultiPolygon const& multipolygon)
    {
        typedef typename boost::range_value<MultiPolygon>::type polygon_type;

        if ( !is_valid_multi_range
                  <
                      polygon_type
                  >::apply(boost::begin(multipolygon),
                           boost::end(multipolygon)) )
        {
            return false;
        }

        // MK::need to check that they are (almost) disjoint
        return true;
    }

};



}} // namespace detail::is_valid
#endif // DOXYGEN_NO_DETAIL




#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


// A Ring is a Polygon.
// A Polygon is always a simple geometric object provided that it is valid.
//
// Reference (for polygon validity): OGC 06-103r4 (§6.1.11.1)
template <typename Ring>
struct is_valid<Ring, ring_tag>
{
    static inline bool apply(Ring const& ring)
    {
        if ( boost::size(ring) < 3 )
        {
            return false;
        }

        if ( detail::is_valid::has_spikes<Ring, closure<Ring>::value>(ring) )
        {
            return false;
        }

        return !geometry::intersects(ring);
    }
};


// A Polygon is always a simple geometric object provided that it is valid.
//
// Reference (for validity of Polygons): OGC 06-103r4 (§6.1.11.1)
template <typename Polygon>
struct is_valid<Polygon, polygon_tag>
    : detail::is_valid::is_valid_polygon<Polygon>
{};



// Not clear what the definition is.
// Right now we check that each element is simple (in fact valid), and
// that the MultiPolygon is also valid.
//
// Reference (for validity of MultiPolygons): OGC 06-103r4 (§6.1.14)
template <typename MultiPolygon>
struct is_valid<MultiPolygon, multi_polygon_tag>
    : detail::is_valid::is_valid_multipolygon<MultiPolygon>
{};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH



template <typename Geometry>
inline bool is_valid(Geometry const& g)
{
    return dispatch::is_valid<Geometry>::apply(g);
}



}}} // namespace boost::geometry::ogc



#endif // BOOST_GEOMETRY_MYSQL_ALGORITHMS_IS_VALID_HPP
