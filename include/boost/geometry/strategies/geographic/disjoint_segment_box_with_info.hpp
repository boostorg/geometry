// Boost.Geometry

// Copyright (c) 2020, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_DISJOINT_SEGMENT_BOX_WITH_INFO_HPP
#define BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_DISJOINT_SEGMENT_BOX_WITH_INFO_HPP

#include <boost/geometry/core/assert.hpp>
#include <boost/geometry/core/coordinate_dimension.hpp>
#include <boost/geometry/core/coordinate_type.hpp>

#include <boost/geometry/formulas/karney_direct_variant.hpp>

namespace boost { namespace geometry
{

namespace strategy { namespace disjoint
{

template
<
    typename FormulaPolicy = strategy::andoyer,
    typename Spheroid = srs::spheroid<double>,
    typename CalculationType = void
>
struct geographic_segment_box_with_info
{
    typedef Spheroid model_type;

    inline geographic_segment_box_with_info()
        : m_spheroid()
    {}

    explicit inline geographic_segment_box_with_info(Spheroid const& spheroid)
        : m_spheroid(spheroid)
    {}


    template
    <
            typename Segment,
            typename BoxPoint,
            typename Strategy,
            typename T
    >
    inline void static apply(Segment const& segment,
                             BoxPoint const& box_point1,
                             BoxPoint const& box_point2,
                             Strategy const& strategy,
                             T& result)
    {
        cartesian_segment_box_with_info::apply(segment, box_point1,
                                               box_point2, strategy, result);
    }


    template
    <
            typename T,
            typename SegmentPoint,
            typename BoxPoint
    >
    inline bool apply_impl(SegmentPoint const& p0,
                           SegmentPoint const& p1,
                           BoxPoint const& bottom_left,
                           BoxPoint const& bottom_right,
                           T & result) const
    {
        // treat the other (parallel) edges of box differently
        // since they are not (geodesic) segments
        typedef typename coordinate_type<SegmentPoint>::type CT;

        CT lon1 = get_as_radian<0>(p0);
        CT lat1 = get_as_radian<1>(p0);
        CT lon2 = get_as_radian<0>(p1);
        CT lat2 = get_as_radian<1>(p1);
        CT lat3 = get_as_radian<1>(bottom_left);

        // ensure that az1 \in [0, pi/2]
        if (lon1 > lon2)
        {
            std::swap(lon1, lon2);
            std::swap(lat1, lat2);
        }
        if (lat1 > lat2)
        {
            lat1 *= -1;
            lat2 *= -1;
            lat3 *= -1;
        }

        CT az1;
        azimuth::geographic<>().apply(lon1, lat1, lon2, lat2, az1);

        CT lon3 = formula::karney_direct_variant<CT>
                ::apply(lon1 * math::r2d<CT>(),
                        lat1 * math::r2d<CT>(),
                        lat3 * math::r2d<CT>(),
                        az1 * math::r2d<CT>(),
                        m_spheroid);

        CT lon_left = get_as_radian<0>(bottom_left);
        CT lon_right = get_as_radian<0>(bottom_right);

#ifdef BOOST_GEOMETRY_DEBUG_GEOGRAPHIC_SEGMENT_BOX
        std::cout << lon_left * math::r2d<CT>() << " , "
                  << lon_right * math::r2d<CT>() << " , "
                  << lon3 << std::endl;
#endif
        math::normalize_longitude<radian>(lon_left);
        math::normalize_longitude<radian>(lon_right);
        math::normalize_longitude<degree>(lon3);
#ifdef BOOST_GEOMETRY_DEBUG_GEOGRAPHIC_SEGMENT_BOX
        std::cout << lon_left * math::r2d<CT>() << " , "
                  << lon_right * math::r2d<CT>() << " , "
                  << lon1 * math::r2d<CT>() << " , "
                  << lon2 * math::r2d<CT>() << " , "
                  << lon3 << std::endl;
#endif
        if (lon2 < lon1)
        {
            swap(lon1, lon2);
        }
        if (( lon_left * math::r2d<CT>() < lon3
             && lon3 < lon_right * math::r2d<CT>() ) &&
            ( lon1 * math::r2d<CT>() < lon3
             && lon3 < lon2 * math::r2d<CT>() ))
        {
            SegmentPoint p;
            set<0>(p, lon3);
            set_from_radian<1>(p, get_as_radian<1>(bottom_left));
            result.count = 1;
            result.intersections[0] = p;
            return true;
        }
        return false;
    }

    template
    <
            typename Segment,
            typename BoxPoint,
            typename T
            >
    inline bool apply(Segment const& segment,
                      BoxPoint const& box_point_left,
                      BoxPoint const& box_point_right,
                      T& result) const
    {

        typedef typename geometry::point_type<Segment>::type SegmentPoint;
        SegmentPoint p[2];
        geometry::detail::assign_point_from_index<0>(segment, p[0]);
        geometry::detail::assign_point_from_index<1>(segment, p[1]);

        typedef typename coordinate_type<SegmentPoint>::type CT;

        CT az1;
        CT lon1 = get_as_radian<0>(p[0]);
        CT lat1 = get_as_radian<1>(p[0]);
        CT lon2 = get_as_radian<0>(p[1]);
        CT lat2 = get_as_radian<1>(p[1]);

        if (lon2 < lon1)
        {
            swap(lon1, lon2);
            swap(lat1, lat2);
        }

        azimuth::geographic<>().apply(lon1, lat1, lon2, lat2, az1);
        CT vertex_lat = geometry::formula::vertex_latitude<CT, geographic_tag>
                ::apply(lat1,
                        az1,
                        m_spheroid);

#ifdef BOOST_GEOMETRY_DEBUG_GEOGRAPHIC_SEGMENT_BOX
        std::cout << "vertex=" << vertex_lat * math::r2d<CT>() << " < "
                  << get_as_radian<1>(box_point_left) * math::r2d<CT>() << " ? "
                  << std::endl;
#endif

        if (std::abs(vertex_lat) < std::abs(get_as_radian<1>(box_point_left)))
        {
            return false;
        }

        if (apply_impl(p[0], p[1],
                       box_point_left, box_point_right,
                       result))
        {
            return true;
        }
        else
        {
            CT vertex_lon = geometry::formula::vertex_longitude<CT, geographic_tag>
                    ::apply(lon1, lat1, lon2, lat2,
                            vertex_lat,
                            az1,
                            azimuth::geographic<>());

            if (vertex_lon < lon1 || vertex_lon > lon2)
            {
                return false;
            }

            SegmentPoint vertex;
            set_from_radian<0>(vertex, vertex_lon);
            set_from_radian<1>(vertex, vertex_lat);

            if (apply_impl(vertex, p[1],
                       box_point_left, box_point_right,
                       result))
            {
                return true;
            }
        }
        return false;
    }

    template
    <
            typename Segment,
            typename Box,
            typename Strategy,
            typename T
            >
    inline void apply(Segment const& segment,
                      Box const& box,
                      Strategy const&,
                      T& result) const
    {

        typedef typename geometry::point_type<Box>::type box_point;
        box_point top_left, top_right, bottom_left, bottom_right;
        geometry::detail::assign_box_corners(box,
                                             bottom_left, bottom_right,
                                             top_left, top_right);
        if (apply(segment, bottom_left, bottom_right, result))
        {
            return;
        }
        apply(segment, top_left, top_right, result);
    }

private:
    Spheroid m_spheroid;
};


}} // namespace strategy::disjoint

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_DISJOINT_SEGMENT_BOX_WITH_INFO_HPP
