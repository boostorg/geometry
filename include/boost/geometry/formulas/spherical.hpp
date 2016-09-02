// Boost.Geometry

// Copyright (c) 2016, Oracle and/or its affiliates.
// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_FORMULAS_SPHERICAL_HPP
#define BOOST_GEOMETRY_FORMULAS_SPHERICAL_HPP

#include <boost/geometry/core/coordinate_system.hpp>
#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/radian_access.hpp>

//#include <boost/geometry/arithmetic/arithmetic.hpp>
#include <boost/geometry/arithmetic/cross_product.hpp>
#include <boost/geometry/arithmetic/dot_product.hpp>

#include <boost/geometry/util/math.hpp>
#include <boost/geometry/util/normalize_spheroidal_coordinates.hpp>
#include <boost/geometry/util/select_coordinate_type.hpp>

namespace boost { namespace geometry {
    
namespace formula {

template <typename T>
struct result_spherical
{
    result_spherical()
        : azimuth(0)
        , reverse_azimuth(0)
    {}

    T azimuth;
    T reverse_azimuth;
};

template <typename Point3d, typename PointSph>
static inline Point3d sph_to_cart3d(PointSph const& point_sph)
{
    typedef typename coordinate_type<Point3d>::type calc_t;

    Point3d res;

    calc_t lon = get_as_radian<0>(point_sph);
    calc_t lat = get_as_radian<1>(point_sph);

    calc_t const cos_lat = cos(lat);
    set<0>(res, cos_lat * cos(lon));
    set<1>(res, cos_lat * sin(lon));
    set<2>(res, sin(lat));

    return res;
}

template <typename PointSph, typename Point3d>
static inline PointSph cart3d_to_sph(Point3d const& point_3d)
{
    typedef typename coordinate_type<PointSph>::type coord_t;
    typedef typename coordinate_type<Point3d>::type calc_t;

    PointSph res;

    calc_t const x = get<0>(point_3d);
    calc_t const y = get<1>(point_3d);
    calc_t const z = get<2>(point_3d);

    set_from_radian<0>(res, atan2(y, x));
    set_from_radian<1>(res, asin(z));

    coord_t lon = get<0>(res);
    coord_t lat = get<1>(res);

    math::normalize_spheroidal_coordinates
        <
            typename coordinate_system<PointSph>::type::units,
            coord_t
        >(lon, lat);

    set<0>(res, lon);
    set<1>(res, lat);

    return res;
}

// -1 right
// 1 left
// 0 on
template <typename Point3d1, typename Point3d2>
static inline int sph_side_value(Point3d1 const& norm, Point3d2 const& pt)
{
    typedef typename select_coordinate_type<Point3d1, Point3d2>::type calc_t;
    calc_t c0 = 0;
    calc_t d = dot_product(norm, pt);
    return math::equals(d, c0) ? 0
        : d > c0 ? 1
        : -1; // d < 0
}

template <typename CT, bool ReverseAzimuth, typename T1, typename T2>
static inline result_spherical<CT> spherical_azimuth(T1 const& lon1,
                                                 T1 const& lat1,
                                                 T2 const& lon2,
                                                 T2 const& lat2)
{
    typedef result_spherical<CT> result_type;
    result_type result;

    // http://williams.best.vwh.net/avform.htm#Crs
    // https://en.wikipedia.org/wiki/Great-circle_navigation
    CT dlon = lon2 - lon1;
    CT cos_dlon = cos(dlon);
    CT sin_dlon = sin(dlon);
    CT cos_lat1 = cos(lat1);
    CT cos_lat2 = cos(lat2);
    CT sin_lat1 = sin(lat1);
    CT sin_lat2 = sin(lat2);

    // An optimization which should kick in often for Boxes
    //if ( math::equals(dlon, ReturnType(0)) )
    //if ( get<0>(p1) == get<0>(p2) )
    //{
    //    return - sin(get_as_radian<1>(p1)) * cos_p2lat);
    //}

    // "An alternative formula, not requiring the pre-computation of d"
    // In the formula below dlon is used as "d"
    result.azimuth = atan2(sin_dlon * cos_lat2,
                           cos_lat1 * sin_lat2 - sin_lat1 * cos_lat2 * cos_dlon);

    if (ReverseAzimuth)
    {
        result.reverse_azimuth =
                     atan2(sin_dlon * cos_lat1,
                           sin_lat2 * cos_lat1 * cos_dlon - cos_lat2 * sin_lat1);
    }

    return result;
}

} // namespace formula

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_FORMULAS_SPHERICAL_HPP
