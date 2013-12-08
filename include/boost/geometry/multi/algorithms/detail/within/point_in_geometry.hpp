// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2012 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2012 Mateusz Loskot, London, UK.

// This file was modified by Oracle on 2013.
// Modifications copyright (c) 2013, Oracle and/or its affiliates.

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_MULTI_ALGORITHMS_DETAIL_WITHIN_POINT_IN_GEOMETRY_HPP
#define BOOST_GEOMETRY_MULTI_ALGORITHMS_DETAIL_WITHIN_POINT_IN_GEOMETRY_HPP

#include <boost/geometry/algorithms/detail/within/point_in_geometry.hpp>

namespace boost { namespace geometry {

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace within {

template <typename G>
struct point_in_geometry_dispatch<G, multi_polygon_tag>
{
    template <typename P, typename S> static inline
    int apply(P const& p, G const& g, S const& s)
    {
        // For invalid multipolygons
        //int res = -1; // outside

        typedef typename boost::range_value<G>::type polygon_type;
        typedef typename boost::range_const_iterator<G>::type iterator;
        for ( iterator it = boost::begin(g) ; it != boost::end(g) ; ++it )
        {
            int pip = detail::within::point_in_geometry_dispatch<polygon_type>::apply(p, *it, s);

            if ( 1 == pip ) // inside polygon
                return 1;
            // Only for valid multi-polygons
            if ( 0 == pip )
                return 0;
            // For invalid multi-polygons
            //if ( res < pip ) // point must be inside at least one polygon
            //    res = pip;
        }

        //return res; // for invalid multipolygons
        return -1; // for valid multipolygons
    }
};

template <typename G>
struct point_in_geometry_dispatch<G, multi_linestring_tag>
{
    template <typename P, typename S> static inline
    int apply(P const& p, G const& g, S const& s)
    {
        int pip = -1; // outside

        typedef typename boost::range_value<G>::type linestring_type;
        typedef typename boost::range_value<linestring_type>::type point_type;
        typedef typename boost::range_iterator<G const>::type iterator;
        iterator it = boost::begin(g);
        for ( ; it != boost::end(g) ; ++it )
        {
            pip = detail::within::point_in_geometry_dispatch<linestring_type>::apply(p, *it, s);

            if ( 0 <= pip )
                break;
        }

        // outside
        if ( pip < 0 )
            return -1;

        unsigned boundaries = pip == 0 ? 1 : 0;

        for ( ; it != boost::end(g) ; ++it )
        {
            if ( boost::size(*it) < 2 )
                continue;

            point_type const& front = *boost::begin(*it);
            point_type const& back = *(--boost::end(*it));
            // is closed_ring
            if ( detail::equals::equals_point_point(front, back) )
                continue;
            if ( detail::equals::equals_point_point(p, front)
              || detail::equals::equals_point_point(p, back) )
                ++boundaries;
        }

        return boundaries % 2 ? 1 : 0;
    }
};

}} // namespace detail::within
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_MULTI_ALGORITHMS_DETAIL_WITHIN_POINT_IN_GEOMETRY_HPP
