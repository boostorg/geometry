// Boost.Geometry

// Copyright (c) 2016, Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_FORMULAS_GEOGRAPHIC_HPP
#define BOOST_GEOMETRY_FORMULAS_GEOGRAPHIC_HPP

#include <boost/geometry/core/coordinate_system.hpp>
#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/radian_access.hpp>

#include <boost/geometry/arithmetic/arithmetic.hpp>
#include <boost/geometry/arithmetic/cross_product.hpp>
#include <boost/geometry/arithmetic/dot_product.hpp>

#include <boost/geometry/formulas/eccentricity_sqr.hpp>
#include <boost/geometry/formulas/flattening.hpp>

#include <boost/geometry/util/math.hpp>
#include <boost/geometry/util/normalize_spheroidal_coordinates.hpp>
#include <boost/geometry/util/select_coordinate_type.hpp>

namespace boost { namespace geometry {
    
namespace formula {

template <typename Point3d, typename PointGeo, typename Spheroid>
inline Point3d geo_to_cart3d(PointGeo const& point_geo, Spheroid const& spheroid)
{
    typedef typename coordinate_type<Point3d>::type calc_t;

    calc_t const c1 = 1;
    calc_t const e_sqr = eccentricity_sqr<calc_t>(spheroid);

    calc_t const lon = get_as_radian<0>(point_geo);
    calc_t const lat = get_as_radian<1>(point_geo);

    Point3d res;

    calc_t const sin_lat = sin(lat);

    // "unit" spheroid, a = 1
    calc_t const N = c1 / math::sqrt(c1 - e_sqr * math::sqr(sin_lat));
    calc_t const N_cos_lat = N * cos(lat);

    set<0>(res, N_cos_lat * cos(lon));
    set<1>(res, N_cos_lat * sin(lon));
    set<2>(res, N * (c1 - e_sqr) * sin_lat);

    return res;
}

template <typename PointGeo, typename Spheroid, typename Point3d>
inline void geo_to_cart3d(PointGeo const& point_geo, Point3d & result, Point3d & north, Point3d & east, Spheroid const& spheroid)
{
    typedef typename coordinate_type<Point3d>::type calc_t;

    calc_t const c1 = 1;
    calc_t const e_sqr = eccentricity_sqr<calc_t>(spheroid);

    calc_t const lon = get_as_radian<0>(point_geo);
    calc_t const lat = get_as_radian<1>(point_geo);

    calc_t const sin_lon = sin(lon);
    calc_t const cos_lon = cos(lon);
    calc_t const sin_lat = sin(lat);
    calc_t const cos_lat = cos(lat);

    // "unit" spheroid, a = 1
    calc_t const N = c1 / math::sqrt(c1 - e_sqr * math::sqr(sin_lat));
    calc_t const N_cos_lat = N * cos_lat;

    set<0>(result, N_cos_lat * cos_lon);
    set<1>(result, N_cos_lat * sin_lon);
    set<2>(result, N * (c1 - e_sqr) * sin_lat);

    set<0>(east, -sin_lon);
    set<1>(east, cos_lon);
    set<2>(east, 0);

    set<0>(north, -sin_lat * cos_lon);
    set<1>(north, -sin_lat * sin_lon);
    set<2>(north, cos_lat);
}

template <typename PointGeo, typename Point3d, typename Spheroid>
inline PointGeo cart3d_to_geo(Point3d const& point_3d, Spheroid const& spheroid)
{
    typedef typename coordinate_type<PointGeo>::type coord_t;
    typedef typename coordinate_type<Point3d>::type calc_t;

    calc_t const c1 = 1;
    //calc_t const c2 = 2;
    calc_t const e_sqr = eccentricity_sqr<calc_t>(spheroid);

    calc_t const x = get<0>(point_3d);
    calc_t const y = get<1>(point_3d);
    calc_t const z = get<2>(point_3d);
    calc_t const xy_l = math::sqrt(math::sqr(x) + math::sqr(y));

    calc_t const lonr = atan2(y, x);
    
    // NOTE: Alternative version
    // http://www.iag-aig.org/attach/989c8e501d9c5b5e2736955baf2632f5/V60N2_5FT.pdf
    // calc_t const lonr = c2 * atan2(y, x + xy_l);
    
    calc_t const latr = atan2(z, (c1 - e_sqr) * xy_l);

    // NOTE: If h is equal to 0 then there is no need to improve value of latitude
    //       because then N_i / (N_i + h_i) = 1
    // http://www.navipedia.net/index.php/Ellipsoidal_and_Cartesian_Coordinates_Conversion

    PointGeo res;

    set_from_radian<0>(res, lonr);
    set_from_radian<1>(res, latr);

    coord_t lon = get<0>(res);
    coord_t lat = get<1>(res);

    math::normalize_spheroidal_coordinates
        <
            typename coordinate_system<PointGeo>::type::units,
            coord_t
        >(lon, lat);

    set<0>(res, lon);
    set<1>(res, lat);

    return res;
}

template <typename Point3d, typename Spheroid>
inline Point3d projected_to_xy(Point3d const& point_3d, Spheroid const& spheroid)
{
    typedef typename coordinate_type<Point3d>::type coord_t;    
    
    // len_xy = sqrt(x^2 + y^2)
    // r = len_xy - |z / tan(lat)|
    // assuming h = 0
    // lat = atan2(z, (1 - e^2) * len_xy);
    // |z / tan(lat)| = (1 - e^2) * len_xy
    // r = e^2 * len_xy
    // x_res = r * cos(lon) = e^2 * len_xy * x / len_xy = e^2 * x
    // y_res = r * sin(lon) = e^2 * len_xy * y / len_xy = e^2 * y
    
    coord_t const c0 = 0;
    coord_t const e_sqr = formula::eccentricity_sqr<coord_t>(spheroid);

    Point3d res;

    set<0>(res, e_sqr * get<0>(point_3d));
    set<1>(res, e_sqr * get<1>(point_3d));
    set<2>(res, c0);

    return res;
}

template <typename Point3d, typename Spheroid>
inline Point3d projected_to_surface(Point3d const& direction, Spheroid const& spheroid)
{
    typedef typename coordinate_type<Point3d>::type coord_t;

    coord_t const c0 = 0;
    coord_t const c2 = 2;
    coord_t const c4 = 4;

    // calculate the point of intersection of a ray and spheroid's surface
    // the origin is the origin of the coordinate system
    //(x*x+y*y)/(a*a) + z*z/(b*b) = 1
    // x = d.x * t
    // y = d.y * t
    // z = d.z * t        
    coord_t const dx = get<0>(direction);
    coord_t const dy = get<1>(direction);
    coord_t const dz = get<2>(direction);

    //coord_t const a_sqr = math::sqr(get_radius<0>(spheroid));
    //coord_t const b_sqr = math::sqr(get_radius<2>(spheroid));
    // "unit" spheroid, a = 1
    coord_t const a_sqr = 1;
    coord_t const b_sqr = math::sqr(get_radius<2>(spheroid) / get_radius<0>(spheroid));

    coord_t const param_a = (dx*dx + dy*dy) / a_sqr + dz*dz / b_sqr;
    coord_t const delta = c4 * param_a;
    // delta >= 0
    coord_t const t = math::sqrt(delta) / (c2 * param_a);

    // result = direction * t
    Point3d result = direction;
    multiply_value(result, t);

    return result;
}

template <typename Point3d, typename Spheroid>
inline bool projected_to_surface(Point3d const& origin, Point3d const& direction, Point3d & result1, Point3d & result2, Spheroid const& spheroid)
{
    typedef typename coordinate_type<Point3d>::type coord_t;

    coord_t const c0 = 0;
    coord_t const c1 = 1;
    coord_t const c2 = 2;
    coord_t const c4 = 4;

    // calculate the point of intersection of a ray and spheroid's surface
    //(x*x+y*y)/(a*a) + z*z/(b*b) = 1
    // x = o.x + d.x * t
    // y = o.y + d.y * t
    // z = o.z + d.z * t        
    coord_t const ox = get<0>(origin);
    coord_t const oy = get<1>(origin);
    coord_t const oz = get<2>(origin);
    coord_t const dx = get<0>(direction);
    coord_t const dy = get<1>(direction);
    coord_t const dz = get<2>(direction);

    //coord_t const a_sqr = math::sqr(get_radius<0>(spheroid));
    //coord_t const b_sqr = math::sqr(get_radius<2>(spheroid));
    // "unit" spheroid, a = 1
    coord_t const a_sqr = 1;
    coord_t const b_sqr = math::sqr(get_radius<2>(spheroid) / get_radius<0>(spheroid));

    coord_t const param_a = (dx*dx + dy*dy) / a_sqr + dz*dz / b_sqr;
    coord_t const param_b = c2 * ((ox*dx + oy*dy) / a_sqr + oz*dz / b_sqr);
    coord_t const param_c = (ox*ox + oy*oy) / a_sqr + oz*oz / b_sqr - c1;

    coord_t const delta = math::sqr(param_b) - c4 * param_a*param_c;

    // equals() ?
    if (delta < c0 || param_a == 0)
    {
        return false;
    }

    // result = origin + direction * t

    coord_t const sqrt_delta = math::sqrt(delta);
    coord_t const two_a = c2 * param_a;

    coord_t const t1 = (-param_b + sqrt_delta) / two_a;
    result1 = direction;
    multiply_value(result1, t1);
    add_point(result1, origin);

    coord_t const t2 = (-param_b - sqrt_delta) / two_a;
    result2 = direction;
    multiply_value(result2, t2);
    add_point(result2, origin);

    return true;
}

template <typename Point3d, typename Spheroid>
inline bool great_elliptic_intersection(Point3d const& a1, Point3d const& a2,
                                        Point3d const& b1, Point3d const& b2,
                                        Point3d & result,
                                        Spheroid const& spheroid)
{
    typedef typename coordinate_type<Point3d>::type coord_t;

    coord_t c0 = 0;
    coord_t c1 = 1;

    Point3d n1 = cross_product(a1, a2);
    Point3d n2 = cross_product(b1, b2);

    // intersection direction
    Point3d id = cross_product(n1, n2);
    coord_t id_len_sqr = dot_product(id, id);

    if (math::equals(id_len_sqr, c0))
    {
        return false;
    }

    // no need to normalize a1 and a2 because the intersection point on
    // the opposite side of the globe is at the same distance from the origin
    coord_t cos_a1i = dot_product(a1, id);
    coord_t cos_a2i = dot_product(a2, id);
    coord_t gri = math::detail::greatest(cos_a1i, cos_a2i);
    Point3d neg_id = id;
    multiply_value(neg_id, -c1);
    coord_t cos_a1ni = dot_product(a1, neg_id);
    coord_t cos_a2ni = dot_product(a2, neg_id);
    coord_t grni = math::detail::greatest(cos_a1ni, cos_a2ni);

    if (gri >= grni)
    {
        result = projected_to_surface(id, spheroid);
    }
    else
    {
        result = projected_to_surface(neg_id, spheroid);
    }

    return true;
}

template <typename Point3d, typename Spheroid>
inline bool elliptic_intersection(Point3d const& a1, Point3d const& a2,
                                  Point3d const& b1, Point3d const& b2,
                                  Point3d & result,
                                  Spheroid const& spheroid)
{
    typedef typename coordinate_type<Point3d>::type coord_t;

    coord_t c0 = 0;
    coord_t c1 = 1;

    Point3d axy1 = projected_to_xy(a1, spheroid);
    Point3d axy2 = projected_to_xy(a2, spheroid);
    Point3d bxy1 = projected_to_xy(b1, spheroid);
    Point3d bxy2 = projected_to_xy(b2, spheroid);

    // axy12 = axy2 - axy1;
    Point3d axy12 = axy2;
    subtract_point(axy12, axy1);
    // bxy12 = bxy2 - bxy1;
    Point3d bxy12 = bxy2;
    subtract_point(bxy12, bxy1);

    // aorig = axy1 + axy12 * 0.5;
    Point3d aorig = axy12;
    multiply_value(aorig, coord_t(0.5));
    add_point(aorig, axy1);
    // borig = bxy1 + bxy12 * 0.5;
    Point3d borig = bxy12;
    multiply_value(borig, coord_t(0.5));
    add_point(borig, bxy1);

    // av1 = a1 - aorig
    Point3d a1v = a1;
    subtract_point(a1v, aorig);
    // av2 = a2 - aorig
    Point3d a2v = a2;
    subtract_point(a2v, aorig);
    // bv1 = b1 - borig
    Point3d b1v = b1;
    subtract_point(b1v, borig);
    // bv2 = b2 - borig
    Point3d b2v = b2;
    subtract_point(b2v, borig);

    Point3d n1 = cross_product(a1v, a2v);
    Point3d n2 = cross_product(b1v, b2v);
        
    coord_t n1_len = math::sqrt(dot_product(n1, n1));
    coord_t n2_len = math::sqrt(dot_product(n2, n2));

    if (math::equals(n1_len, c0) || math::equals(n2_len, c0))
    {
        return false;
    }

    // normalize
    divide_value(n1, n1_len);
    divide_value(n2, n2_len);
    
    // Below
    // n . (p - o) = 0
    // n . p - n . o = 0
    // n . p + d = 0
    // n . p = h

    // intersection direction
    Point3d id = cross_product(n1, n2);

    if (math::equals(dot_product(id, id), c0))
    {
        return false;
    }

    coord_t dot_n1_n2 = dot_product(n1, n2);
    coord_t dot_n1_n2_sqr = math::sqr(dot_n1_n2);

    coord_t h1 = dot_product(n1, aorig);
    coord_t h2 = dot_product(n2, borig);

    coord_t denom = c1 - dot_n1_n2_sqr;
    coord_t C1 = (h1 - h2 * dot_n1_n2) / denom;
    coord_t C2 = (h2 - h1 * dot_n1_n2) / denom;
    
    // C1 * n1 + C2 * n2
    Point3d C1_n1 = n1;
    multiply_value(C1_n1, C1);
    Point3d C2_n2 = n2;
    multiply_value(C2_n2, C2);
    Point3d io = C1_n1;
    add_point(io, C2_n2);

    Point3d ip1_s, ip2_s;
    if (! projected_to_surface(io, id, ip1_s, ip2_s, spheroid))
    {
        return false;
    }

    coord_t a1v_len = math::sqrt(dot_product(a1v, a1v));
    divide_value(a1v, a1v_len);
    coord_t a2v_len = math::sqrt(dot_product(a2v, a2v));
    divide_value(a2v, a2v_len);
    coord_t b1v_len = math::sqrt(dot_product(b1v, b1v));
    divide_value(b1v, a1v_len);
    coord_t b2v_len = math::sqrt(dot_product(b2v, b2v));
    divide_value(b2v, a2v_len);

    // NOTE: could be used to calculate comparable distances/angles
    coord_t cos_a1i = dot_product(a1v, id);
    coord_t cos_a2i = dot_product(a2v, id);
    coord_t gri = math::detail::greatest(cos_a1i, cos_a2i);
    Point3d neg_id = id;
    multiply_value(neg_id, -c1);
    coord_t cos_a1ni = dot_product(a1v, neg_id);
    coord_t cos_a2ni = dot_product(a2v, neg_id);
    coord_t grni = math::detail::greatest(cos_a1ni, cos_a2ni);

    result = gri >= grni ? ip1_s : ip2_s;

    return true;
}

} // namespace formula

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_FORMULAS_GEOGRAPHIC_HPP
