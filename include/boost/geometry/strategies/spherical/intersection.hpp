// Boost.Geometry

// Copyright (c) 2016, Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_SPHERICAL_INTERSECTION_HPP
#define BOOST_GEOMETRY_STRATEGIES_SPHERICAL_INTERSECTION_HPP

#include <algorithm>

#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/radian_access.hpp>

#include <boost/geometry/algorithms/detail/assign_values.hpp>
#include <boost/geometry/algorithms/detail/assign_indexed_point.hpp>
#include <boost/geometry/algorithms/detail/equals/point_point.hpp>
#include <boost/geometry/algorithms/detail/recalculate.hpp>

#include <boost/geometry/arithmetic/arithmetic.hpp>
#include <boost/geometry/arithmetic/cross_product.hpp>
#include <boost/geometry/arithmetic/dot_product.hpp>

#include <boost/geometry/geometries/concepts/point_concept.hpp>
#include <boost/geometry/geometries/concepts/segment_concept.hpp>

#include <boost/geometry/policies/robustness/segment_ratio.hpp>

#include <boost/geometry/strategies/side_info.hpp>
#include <boost/geometry/strategies/intersection_result.hpp>

#include <boost/geometry/util/math.hpp>
#include <boost/geometry/util/select_calculation_type.hpp>


namespace boost { namespace geometry
{

namespace strategy { namespace intersection
{

template <typename Policy, typename CalculationType = void>
struct relate_spherical_segments
{
    typedef typename Policy::return_type return_type;

    // TODO: Move to SSF implementation

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
        typedef typename coordinate_type<Point3d>::type calc_t;

        PointSph res;

        calc_t x = get<0>(point_3d);
        calc_t y = get<1>(point_3d);
        calc_t z = get<2>(point_3d);

        set_from_radian<0>(res, atan2(y, x));
        set_from_radian<1>(res, asin(z)); // assuming r is 1

        return geometry::detail::return_normalized<PointSph>(res);
    }

    // -1 right
    // 1 left
    // 0 on
    template <typename Point3d>
    static inline int side_value(Point3d const& norm, Point3d const& pt)
    {
        typedef typename coordinate_type<Point3d>::type calc_t;
        calc_t c0 = 0;
        calc_t d = dot_product(norm, pt);
        return math::equals(d, c0) ? 0
            : d > c0 ? 1
            : -1; // d < 0
    }

    template <typename CoordinateType, typename SegmentRatio, typename Vector3d>
    struct segment_intersection_info
    {
        typedef typename select_most_precise
            <
                CoordinateType, double
            >::type promoted_type;

        promoted_type comparable_length_a() const
        {
            return robust_ra.denominator();
        }

        promoted_type comparable_length_b() const
        {
            return robust_rb.denominator();
        }

        template <typename Point, typename Segment>
        void assign_a(Point& point, Segment const& ) const
        {
            point = cart3d_to_sph<Point>(intersection_point);
        }
        template <typename Point, typename Segment>
        void assign_b(Point& point, Segment const& ) const
        {
            point = cart3d_to_sph<Point>(intersection_point);
        }

        Vector3d intersection_point;
        SegmentRatio robust_ra;
        SegmentRatio robust_rb;
    };

    template <typename Point1, typename Point2, typename Vec3d, typename CalcT>
    static inline bool calculate_endpoint_data(Point1 const& a1, Point1 const& a2,
                                               Point2 const& b1, Point2 const& b2,
                                               Vec3d const& a1v, // in
                                               Vec3d const& a2v, // in
                                               Vec3d const& norm1, // in
                                               Vec3d const& i1, // in
                                               CalcT& dist_a1_a2, CalcT& dist_a1_i1) // out
    {
        return calculate_xxx_data<Vec3d const>(a1, a2, b1, b2, a1v, a2v, norm1, i1, dist_a1_a2, dist_a1_i1);
    }

    template <typename Point1, typename Point2, typename Vec3d, typename CalcT>
    static inline bool calculate_ip_data_exact(Point1 const& a1, Point1 const& a2,
                                               Point2 const& b1, Point2 const& b2,
                                               Vec3d const& a1v, // in
                                               Vec3d const& a2v, // in
                                               Vec3d const& norm1, // in
                                               Vec3d & i1, // in, out
                                               CalcT& dist_a1_a2, CalcT& dist_a1_i1) // out
    {
        return calculate_xxx_data<Vec3d>(a1, a2, b1, b2, a1v, a2v, norm1, i1, dist_a1_a2, dist_a1_i1);
    }

    template <typename T>
    static inline void assign_if_nonconst(T & l, T const& r)
    {
        l = r;
    }

    template <typename T>
    static inline void assign_if_nonconst(T const&, T const&)
    {}

    template <typename Vec3dIP, typename Point1, typename Point2, typename Vec3d, typename CalcT>
    static inline bool calculate_xxx_data(Point1 const& a1, Point1 const& a2,
                                          Point2 const& b1, Point2 const& b2,
                                          Vec3d const& a1v, // in
                                          Vec3d const& a2v, // in
                                          Vec3d const& norm1, // in
                                          Vec3dIP & i1, // in, out
                                          CalcT& dist_a1_a2, CalcT& dist_a1_i1) // out
    {
        CalcT const c0 = 0;
        CalcT const c1 = 1;
        CalcT const c2 = 2;
        CalcT const c4 = 4;
        CalcT const eps = std::numeric_limits<CalcT>::epsilon();
        CalcT const small_number = eps * 10000;
            
        CalcT cos_a1_a2 = dot_product(a1v, a2v);
        dist_a1_a2 = -cos_a1_a2 + c1; // [1, -1] -> [0, 2] representing [0, pi]

        CalcT cos_a1_i1 = dot_product(a1v, i1);
        dist_a1_i1 = -cos_a1_i1 + c1; // [0, 2] representing [0, pi]
        if (dot_product(norm1, cross_product(a1v, i1)) < c0) // left or right of a1 on a
            dist_a1_i1 = -dist_a1_i1; // [0, 2] -> [0, -2] representing [0, -pi]
        if (dist_a1_i1 <= -c2) // <= -pi
            dist_a1_i1 += c4; // += 2pi

        if (segment_ratio<CalcT>(dist_a1_i1, dist_a1_a2).on_segment())
            return true;
        
        using geometry::detail::equals::equals_point_point;
        if (math::abs(dist_a1_i1) <= small_number
            && (equals_point_point(a1, b1) || equals_point_point(a1, b2)))
        {
            assign_if_nonconst(i1, a1v);
            dist_a1_i1 = 0;
            return true;
        }

        if (math::abs(dist_a1_a2 - dist_a1_i1) <= small_number
            && (equals_point_point(a2, b1) || equals_point_point(a2, b2)))
        {
            assign_if_nonconst(i1, a2v);
            dist_a1_i1 = dist_a1_a2;
            return true;
        }

        return false;
    }

    template <typename Point1, typename Point2, typename Vec3d, typename CalcT>
    static inline bool calculate_ip_data(Point1 const& a1, Point1 const& a2,
                                         Point2 const& b1, Point2 const& b2,
                                         Vec3d const& a1v, // in
                                         Vec3d const& a2v, // in
                                         Vec3d const& norm1, // in
                                         Vec3d & i1, // in-out
                                         CalcT& dist_a1_a2, CalcT& dist_a1_i1) // out
    {
        //CalcT const c0 = 0;
        CalcT const c1 = 1;
        CalcT const c2 = 2;
        CalcT const c4 = 4;
        CalcT const eps = std::numeric_limits<CalcT>::epsilon();
        CalcT const small_number = eps * 10000;

        if (calculate_ip_data_exact(a1, a2, b1, b2, a1v, a2v, norm1, i1, dist_a1_a2, dist_a1_i1))
            return true;

        // check the ip on the other side of the sphere
        CalcT dist_a1_i2 = dist_a1_i1 - c2; // dist_a1_i2 = dist_a1_i1 - pi;
        if (dist_a1_i2 <= -c2)          // <= -pi
            dist_a1_i2 += c4;           // += 2pi;

        if (segment_ratio<CalcT>(dist_a1_i2, dist_a1_a2).on_segment())
        {
            dist_a1_i1 = dist_a1_i2;
            multiply_value(i1, -c1); // the opposite intersection
            return true;
        }

        using geometry::detail::equals::equals_point_point;
        if (math::abs(dist_a1_i2) <= small_number
            && (equals_point_point(a1, b1) || equals_point_point(a1, b2)))
        {
            i1 = a1v;
            dist_a1_i1 = 0;
            return true;
        }

        if (math::abs(dist_a1_a2 - dist_a1_i2) <= small_number
            && (equals_point_point(a2, b1) || equals_point_point(a2, b2)))
        {
            i1 = a2v;
            dist_a1_i1 = dist_a1_a2;
            return true;
        }

        // neither intersection is between endpoints
        return false;
    }

    template <typename CalcT, typename Segment, typename Point1, typename Point2, typename Vec3d>
    static inline return_type collinear_one_degenerted(Segment const& segment, bool degenerated_a,
                                                       Point1 const& a1, Point1 const& a2,
                                                       Point2 const& b1, Point2 const& b2,
                                                       Vec3d const& v1, Vec3d const& v2, Vec3d const& norm,
                                                       Vec3d const& vother)
    {
        CalcT dist_1_2, dist_1_o;
        return ! calculate_endpoint_data(a1, a2, b1, b2, v1, v2, norm, vother, dist_1_2, dist_1_o)
                ? Policy::disjoint()
                : Policy::one_degenerate(segment, segment_ratio<CalcT>(dist_1_o, dist_1_2), degenerated_a);
    }

    // Relate segments a and b
    template <typename Segment1, typename Segment2, typename RobustPolicy>
    static inline return_type apply(Segment1 const& a, Segment2 const& b, RobustPolicy const&)
    {
        BOOST_CONCEPT_ASSERT( (concept::ConstSegment<Segment1>) );
        BOOST_CONCEPT_ASSERT( (concept::ConstSegment<Segment2>) );

        typedef typename point_type<Segment1>::type point1_t;
        typedef typename point_type<Segment2>::type point2_t;
        point1_t a1, a2;
        point2_t b1, b2;

        // TODO: use indexed_point_view if possible?
        detail::assign_point_from_index<0>(a, a1);
        detail::assign_point_from_index<1>(a, a2);
        detail::assign_point_from_index<0>(b, b1);
        detail::assign_point_from_index<1>(b, b2);

        // TODO: check only 2 first coordinates here?
        using geometry::detail::equals::equals_point_point;
        bool a_is_point = equals_point_point(a1, a2);
        bool b_is_point = equals_point_point(b1, b2);

        if(a_is_point && b_is_point)
        {
            return equals_point_point(a1, b2)
                ? Policy::degenerate(a, true)
                : Policy::disjoint()
                ;
        }

        typedef typename select_calculation_type
            <Segment1, Segment2, CalculationType>::type calc_t;

        calc_t const c0 = 0;
        calc_t const c1 = 0;

        typedef model::point<calc_t, 3, cs::cartesian> vec3d_t;

        vec3d_t const a1v = sph_to_cart3d<vec3d_t>(a1);
        vec3d_t const a2v = sph_to_cart3d<vec3d_t>(a2);
        vec3d_t const b1v = sph_to_cart3d<vec3d_t>(b1);
        vec3d_t const b2v = sph_to_cart3d<vec3d_t>(b2);
        
        vec3d_t norm1 = cross_product(a1v, a2v);
        vec3d_t norm2 = cross_product(b1v, b2v);

        side_info sides;
        // not normalized normals, the same as in SSF
        sides.set<0>(side_value(norm2, a1v), side_value(norm2, a2v));
        if (sides.same<0>())
        {
            // Both points are at same side of other segment, we can leave
            return Policy::disjoint();
        }
        // not normalized normals, the same as in SSF
        sides.set<1>(side_value(norm1, b1v), side_value(norm1, b2v));
        if (sides.same<1>())
        {
            // Both points are at same side of other segment, we can leave
            return Policy::disjoint();
        }

        // NOTE: at this point the segments may still be disjoint

        bool collinear = sides.collinear();

        calc_t const len1 = math::sqrt(dot_product(norm1, norm1));
        calc_t const len2 = math::sqrt(dot_product(norm2, norm2));

        // point or opposite sides of a sphere, assume point
        if (math::equals(len1, c0))
        {
            a_is_point = true;
            if (sides.get<0, 0>() == 0 || sides.get<0, 1>() == 0)
            {
                sides.set<0>(0, 0);
            }
        }
        else
        {
            // normalize
            divide_value(norm1, len1);
        }

        if (math::equals(len2, c0))
        {
            b_is_point = true;
            if (sides.get<1, 0>() == 0 || sides.get<1, 1>() == 0)
            {
                sides.set<1>(0, 0);
            }
        }
        else
        {
            // normalize
            divide_value(norm2, len2);
        }

        // check both degenerated once more
        if (a_is_point && b_is_point)
        {
            return equals_point_point(a1, b2)
                ? Policy::degenerate(a, true)
                : Policy::disjoint()
                ;
        }

        // NOTE: at this point one of the segments may be degenerated
        //       and the segments may still be disjoint

        // --------------------------------- //
        // TODO: handle degenerated segments //
        // --------------------------------- //

        calc_t dot_n1n2 = dot_product(norm1, norm2);

        // NOTE: this is technically not needed since theoretically above sides
        //       are calculated, but just in case check the normals.
        //       Have in mind that SSF side strategy doesn't check this.
        // collinear if normals are equal or opposite: cos(a) in {-1, 1}
        if (math::equals(math::abs(dot_n1n2), c1))
        {
            collinear = true;
            sides.set<0>(0, 0);
            sides.set<1>(0, 0);
        }
        
        if (collinear)
        {
            if (a_is_point)
            {
                return collinear_one_degenerted<calc_t>(a, true, b1, b2, a1, a2, b1v, b2v, norm2, a1v);
            }
            else if (b_is_point)
            {
                // b2 used to be consistent with (degenerated) checks above (is it needed?)
                return collinear_one_degenerted<calc_t>(b, false, a1, a2, b1, b2, a1v, a2v, norm1, b1v);
            }
            else
            {
                calc_t dist_a1_a2, dist_a1_b1, dist_a1_b2;
                calc_t dist_b1_b2, dist_b1_a1, dist_b1_a2;
                // use shorter segment
                if (len1 <= len2)
                {
                    calculate_endpoint_data(a1, a2, b1, b2, a1v, a2v, norm1, b1v, dist_a1_a2, dist_a1_b1);
                    calculate_endpoint_data(a1, a2, b1, b2, a1v, a2v, norm1, b2v, dist_a1_a2, dist_a1_b2);
                    dist_b1_b2 = dist_a1_b2 - dist_a1_b1;
                    dist_b1_a1 = -dist_a1_b1;
                    dist_b1_a2 = dist_a1_a2 - dist_a1_b1;
                }
                else
                {
                    calculate_endpoint_data(b1, b2, a1, a2, b1v, b2v, norm2, a1v, dist_b1_b2, dist_b1_a1);
                    calculate_endpoint_data(b1, b2, a1, a2, b1v, b2v, norm2, a2v, dist_b1_b2, dist_b1_a2);
                    dist_a1_a2 = dist_b1_a2 - dist_b1_a1;
                    dist_a1_b1 = -dist_b1_a1;
                    dist_a1_b2 = dist_b1_b2 - dist_b1_a1;
                }

                segment_ratio<calc_t> ra_from(dist_b1_a1, dist_b1_b2);
                segment_ratio<calc_t> ra_to(dist_b1_a2, dist_b1_b2);
                segment_ratio<calc_t> rb_from(dist_a1_b1, dist_a1_a2);
                segment_ratio<calc_t> rb_to(dist_a1_b2, dist_a1_a2);
                
                int const a1_wrt_b = position_value(c0, dist_a1_b1, dist_a1_b2);
                int const a2_wrt_b = position_value(dist_a1_a2, dist_a1_b1, dist_a1_b2);
                int const b1_wrt_a = position_value(c0, dist_b1_a1, dist_b1_a2);
                int const b2_wrt_a = position_value(dist_b1_b2, dist_b1_a1, dist_b1_a2);

                if (a1_wrt_b == 1)
                {
                    ra_from.assign(0, dist_b1_b2);
                    rb_from.assign(0, dist_a1_a2);
                }
                else if (a1_wrt_b == 3)
                {
                    ra_from.assign(dist_b1_b2, dist_b1_b2);
                    rb_to.assign(0, dist_a1_a2);
                }

                if (a2_wrt_b == 1)
                {
                    ra_to.assign(0, dist_b1_b2);
                    rb_from.assign(dist_a1_a2, dist_a1_a2);
                }
                else if (a2_wrt_b == 3)
                {
                    ra_to.assign(dist_b1_b2, dist_b1_b2);
                    rb_to.assign(dist_a1_a2, dist_a1_a2);
                }

                if ((a1_wrt_b < 1 && a2_wrt_b < 1) || (a1_wrt_b > 3 && a2_wrt_b > 3))
                {
                    return Policy::disjoint();
                }

                bool const opposite = dot_n1n2 < c0;

                return Policy::segments_collinear(a, b, opposite,
                    a1_wrt_b, a2_wrt_b, b1_wrt_a, b2_wrt_a,
                    ra_from, ra_to, rb_from, rb_to);
            }
        }
        else
        {
            if (a_is_point || b_is_point)
            {
                return Policy::disjoint();
            }

            // great circles intersections
            vec3d_t i1 = cross_product(norm1, norm2);
            calc_t len = math::sqrt(dot_product(i1, i1)); // length != 0 at this point
            divide_value(i1, len); // normalize i1

            calc_t dist_a1_a2, dist_a1_i1, dist_b1_b2, dist_b1_i1;
            if (calculate_ip_data(a1, a2, b1, b2, a1v, a2v, norm1, i1, dist_a1_a2, dist_a1_i1)
                && calculate_ip_data_exact(b1, b2, a1, a2, b1v, b2v, norm2, i1, dist_b1_b2, dist_b1_i1))
            {
                // intersects
                segment_intersection_info
                    <
                        calc_t,
                        segment_ratio<calc_t>,
                        vec3d_t
                    > sinfo;

                sinfo.robust_ra.assign(dist_a1_i1, dist_a1_a2);
                sinfo.robust_rb.assign(dist_b1_i1, dist_b1_b2);
                sinfo.intersection_point = i1;

                return Policy::segments_crosses(sides, sinfo, a, b);
            }
            else
            {
                return Policy::disjoint();
            }
        }
    }

private:
    template <typename ProjCoord1, typename ProjCoord2>
    static inline int position_value(ProjCoord1 const& ca1,
                                     ProjCoord2 const& cb1,
                                     ProjCoord2 const& cb2)
    {
        // S1x  0   1    2     3   4
        // S2       |---------->
        return math::equals(ca1, cb1) ? 1
             : math::equals(ca1, cb2) ? 3
             : cb1 < cb2 ?
                ( ca1 < cb1 ? 0
                : ca1 > cb2 ? 4
                : 2 )
              : ( ca1 > cb1 ? 0
                : ca1 < cb2 ? 4
                : 2 );
    }
};


}} // namespace strategy::intersection

}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_STRATEGIES_SPHERICAL_INTERSECTION_HPP
