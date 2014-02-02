// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2013.
// Modifications copyright (c) 2013, Oracle and/or its affiliates.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_POINT_GEOMETRY_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_POINT_GEOMETRY_HPP

#include <boost/geometry/algorithms/detail/disjoint/point_point.hpp> // later change to equal/point_point.hpp
#include <boost/geometry/algorithms/detail/within/point_in_geometry.hpp>
#include <boost/geometry/algorithms/within.hpp>
#include <boost/geometry/algorithms/covered_by.hpp>

#include <boost/geometry/algorithms/detail/relate/result.hpp>

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace relate {

template <typename Point1, typename Point2>
struct point_point
{
    static inline result apply(Point1 const& point1, Point2 const& point2)
    {
        bool equal = detail::equals::equals_point_point(point1, point2);

        if ( equal )
            return result("0FFFFFFFT");
        else
            return result("FF0FFF0FT");
    }
};

// NOTE: Those tests should be consistent with within(Point, Box) and covered_by(Point, Box)
// There is no EPS used in those functions, values are compared using < or <=
// so comparing MIN and MAX in the same way should be fine

template <typename Box, std::size_t I = 0, std::size_t D = geometry::dimension<Box>::value>
struct box_has_interior
{
    static inline bool apply(Box const& box)
    {
        return geometry::get<min_corner, I>(box) < geometry::get<max_corner, I>(box)
            && box_has_interior<Box, I + 1, D>::apply(box);
    }
};

template <typename Box, std::size_t D>
struct box_has_interior<Box, D, D>
{
    static inline bool apply(Box const&) { return true; }
};

// NOTE: especially important here (see the NOTE above).

template <typename Box, std::size_t I = 0, std::size_t D = geometry::dimension<Box>::value>
struct box_has_equal_min_max
{
    static inline bool apply(Box const& box)
    {
        return geometry::get<min_corner, I>(box) == geometry::get<max_corner, I>(box)
            && box_has_equal_min_max<Box, I + 1, D>::apply(box);
    }
};

template <typename Box, std::size_t D>
struct box_has_equal_min_max<Box, D, D>
{
    static inline bool apply(Box const&) { return true; }
};

template <typename Point, typename Box>
struct point_box
{
    static inline result apply(Point const& point, Box const& box)
    {
        if ( geometry::within(point, box) ) // this also means that the box has interior
        {
            return result("0FFFFFTTT");
        }
        else if ( geometry::covered_by(point, box) ) // point is on the boundary
        {
            //if ( box_has_interior<Box>::apply(box) )
            //{
            //    return result("F0FFFFTTT");
            //}
            //else if ( box_has_equal_min_max<Box>::apply(box) ) // no boundary outside point
            //{
            //    return result("F0FFFFFFT");
            //}
            //else // no interior outside point
            //{
            //    return result("F0FFFFFTT");
            //}
            return result("F0FFFF**T");
        }
        else 
        {
            /*if ( box_has_interior<Box>::apply(box) )
            {
                return result("FF0FFFTTT");
            }
            else
            {
                return result("FF0FFFFTT");
            }*/
            return result("FF0FFF*TT");
        }
    }
};

template <typename Box, typename Point>
struct box_point
{
    static inline result apply(Box const& box, Point const& point)
    {
        if ( geometry::within(point, box) )
            return result("0FTFFTFFT");
        else if ( geometry::covered_by(point, box) )
            return result("FF*0F*FFT");
        else 
            return result("FF*FFT0FT");
    }
};

// non-point geometry
template <typename Point, typename Geometry>
struct point_geometry
{
    static inline result apply(Point const& point, Geometry const& geometry)
    {
        int pig = detail::within::point_in_geometry(point, geometry);

        // TODO: * - if geometry has interior and/or boundary
        // e.g. isn't 1-point linestring or linear ring or 0-area polygon

        if ( pig < 0 ) // not within
            return result("FF0FFF**T");
        else if ( pig == 0 )
            return result("F0FFFF**T");
        else // pig > 0 - within
            return result("0FFFFF**T");
    }
};

// transposed result of point_geometry
template <typename Geometry, typename Point>
struct geometry_point
{
    static inline result apply(Geometry const& geometry, Point const& point)
    {
        int pig = detail::within::point_in_geometry(point, geometry);

        // TODO: * - if geometry has interior and/or boundary
        // e.g. isn't 1-point linestring or linear ring or 0-area polygon

        if ( pig < 0 ) // not within
            return result("FF*FF*0FT");
        else if ( pig == 0 )
            return result("FF*0F*FFT");
        else // pig > 0 - within
            return result("0F*FF*FFT");
    }
};

}} // namespace detail::relate
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_POINT_GEOMETRY_HPP
