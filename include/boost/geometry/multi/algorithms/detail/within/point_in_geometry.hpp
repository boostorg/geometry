// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2012 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2012 Mateusz Loskot, London, UK.
// Copyright (c) 2013 Adam Wulkiewicz, Lodz, Poland.

// This file was modified by Oracle on 2013, 2014.
// Modifications copyright (c) 2013, 2014 Oracle and/or its affiliates.

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

#ifndef BOOST_GEOMETRY_MULTI_ALGORITHMS_DETAIL_WITHIN_POINT_IN_GEOMETRY_HPP
#define BOOST_GEOMETRY_MULTI_ALGORITHMS_DETAIL_WITHIN_POINT_IN_GEOMETRY_HPP

#include <boost/geometry/algorithms/detail/within/point_in_geometry.hpp>

namespace boost { namespace geometry {

#ifndef DOXYGEN_NO_DETAIL
namespace detail_dispatch { namespace within {

template <typename Geometry>
struct point_in_geometry<Geometry, multi_polygon_tag>
{
    template <typename Point, typename Strategy> static inline
    int apply(Point const& point, Geometry const& geometry, Strategy const& strategy)
    {
        // For invalid multipolygons
        //int res = -1; // outside

        typedef typename boost::range_value<Geometry>::type polygon_type;
        typedef typename boost::range_const_iterator<Geometry>::type iterator;
        for ( iterator it = boost::begin(geometry) ; it != boost::end(geometry) ; ++it )
        {
            int pip = point_in_geometry<polygon_type>::apply(point, *it, strategy);

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

template <typename Geometry>
struct point_in_geometry<Geometry, multi_linestring_tag>
{
    template <typename Point, typename Strategy> static inline
    int apply(Point const& point, Geometry const& geometry, Strategy const& strategy)
    {
        int pip = -1; // outside

        typedef typename boost::range_value<Geometry>::type linestring_type;
        typedef typename boost::range_value<linestring_type>::type point_type;
        typedef typename boost::range_iterator<Geometry const>::type iterator;
        iterator it = boost::begin(geometry);
        for ( ; it != boost::end(geometry) ; ++it )
        {
            pip = point_in_geometry<linestring_type>::apply(point, *it, strategy);

            if ( 0 <= pip )
            {
                ++it;
                break;
            }
        }

        // outside
        if ( pip < 0 )
            return -1;

        // TODO: the following isn't needed for covered_by()

        unsigned boundaries = pip == 0 ? 1 : 0;

        for ( ; it != boost::end(geometry) ; ++it )
        {
            if ( boost::size(*it) < 2 )
                continue;

            point_type const& front = *boost::begin(*it);
            point_type const& back = *(--boost::end(*it));

            // is closed_ring - no boundary
            if ( detail::equals::equals_point_point(front, back) )
                continue;

            // is point on boundary
            if ( detail::equals::equals_point_point(point, front)
              || detail::equals::equals_point_point(point, back) )
            {
                ++boundaries;
            }
        }

        // if the number of boundaries is odd, the point is on the boundary
        return boundaries % 2 ? 0 : 1;
    }
};

template <typename Geometry>
struct point_in_geometry<Geometry, multi_point_tag>
{
    template <typename Point, typename Strategy> static inline
    int apply(Point const& point, Geometry const& geometry, Strategy const& strategy)
    {
        typedef typename boost::range_value<Geometry>::type point_type;
        typedef typename boost::range_const_iterator<Geometry>::type iterator;
        for ( iterator it = boost::begin(geometry) ; it != boost::end(geometry) ; ++it )
        {
            int pip = point_in_geometry<point_type>::apply(point, *it, strategy);

            if ( pip > 0 ) // inside
                return 1;
        }

        return -1; // outside
    }
};

}} // namespace detail_dispatch::within
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_MULTI_ALGORITHMS_DETAIL_WITHIN_POINT_IN_GEOMETRY_HPP
