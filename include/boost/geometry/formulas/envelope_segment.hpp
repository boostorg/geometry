// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2017 Oracle and/or its affiliates.
// Contributed and/or modified by Vissarion Fisikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_FORMULAS_ENVELOPE_SEGMENT_HPP
#define BOOST_GEOMETRY_FORMULAS_ENVELOPE_SEGMENT_HPP

#include <boost/geometry/algorithms/detail/envelope/transform_units.hpp>
#include <boost/geometry/geometries/helper_geometry.hpp>
#include <boost/geometry/formulas/vertex_latitude.hpp>

namespace boost { namespace geometry { namespace formula
{

// Computes the MBR of a segment given by (lon1, lat1) and (lon2,
// lat2), and with azimuths a1 and a2 at the two endpoints of the
// segment.
// It is assumed that the spherical coordinates of the segment are
// normalized and in radians.
// The longitudes and latitudes of the endpoints are overridden by
// those of the box.
template <typename CS_Tag>
class envelope_segment
{
private:

    // degrees or radians
    template <typename CalculationType>
    static inline void swap(CalculationType& lon1,
                            CalculationType& lat1,
                            CalculationType& lon2,
                            CalculationType& lat2)
    {
        std::swap(lon1, lon2);
        std::swap(lat1, lat2);
    }

    // radians
    template <typename CalculationType>
    static inline bool contains_pi_half(CalculationType const& a1,
                                        CalculationType const& a2)
    {
        // azimuths a1 and a2 are assumed to be in radians
        BOOST_GEOMETRY_ASSERT(! math::equals(a1, a2));

        static CalculationType const pi_half = math::half_pi<CalculationType>();

        return (a1 < a2)
            ? (a1 < pi_half && pi_half < a2)
            : (a1 > pi_half && pi_half > a2);
    }

    // radians or degrees
    template <typename Units, typename CoordinateType>
    static inline bool crosses_antimeridian(CoordinateType const& lon1,
                                            CoordinateType const& lon2)
    {
        typedef math::detail::constants_on_spheroid
            <
                CoordinateType, Units
            > constants;

        return math::abs(lon1 - lon2) > constants::half_period(); // > pi
    }

    // degrees or radians
    template <typename Units, typename CalculationType, typename Strategy>
    static inline void compute_box_corners(CalculationType& lon1,
                                           CalculationType& lat1,
                                           CalculationType& lon2,
                                           CalculationType& lat2,
                                           Strategy const& strategy)
    {
        // coordinates are assumed to be in radians
        BOOST_GEOMETRY_ASSERT(lon1 <= lon2);

        CalculationType lon1_rad = math::as_radian<Units>(lon1);
        CalculationType lat1_rad = math::as_radian<Units>(lat1);
        CalculationType lon2_rad = math::as_radian<Units>(lon2);
        CalculationType lat2_rad = math::as_radian<Units>(lat2);

        CalculationType a1, a2;
        strategy.apply(lon1_rad, lat1_rad, lon2_rad, lat2_rad, a1, a2);

        if (lat1 > lat2)
        {
            std::swap(lat1, lat2);
            std::swap(lat1_rad, lat2_rad);
        }

        if (math::equals(a1, a2))
        {
            // the segment must lie on the equator or is very short
            return;
        }

        if (contains_pi_half(a1, a2))
        {
            CalculationType p_max = geometry::formula
                ::vertex_latitude<CalculationType, CS_Tag>::apply(lat1_rad, a1);

            CalculationType const mid_lat = lat1 + lat2;
            if (mid_lat < 0)
            {
                // update using min latitude
                CalculationType const lat_min_rad = -p_max;
                CalculationType const lat_min
                        = math::from_radian<Units>(lat_min_rad);

                if (lat1 > lat_min)
                {
                    lat1 = lat_min;
                }
            }
            else if (mid_lat > 0)
            {
                // update using max latitude
                CalculationType const lat_max_rad = p_max;
                CalculationType const lat_max
                        = math::from_radian<Units>(lat_max_rad);

                if (lat2 < lat_max)
                {
                    lat2 = lat_max;
                }
            }
        }
    }

    template <typename Units, typename CalculationType, typename Strategy>
    static inline void apply(CalculationType& lon1,
                             CalculationType& lat1,
                             CalculationType& lon2,
                             CalculationType& lat2,
                             Strategy const& strategy)
    {
        typedef math::detail::constants_on_spheroid
            <
                CalculationType, Units
            > constants;

        bool is_pole1 = math::equals(math::abs(lat1), constants::max_latitude());
        bool is_pole2 = math::equals(math::abs(lat2), constants::max_latitude());

        if (is_pole1 && is_pole2)
        {
            // both points are poles; nothing more to do:
            // longitudes are already normalized to 0
            // but just in case
            lon1 = 0;
            lon2 = 0;
        }
        else if (is_pole1 && !is_pole2)
        {
            // first point is a pole, second point is not:
            // make the longitude of the first point the same as that
            // of the second point
            lon1 = lon2;
        }
        else if (!is_pole1 && is_pole2)
        {
            // second point is a pole, first point is not:
            // make the longitude of the second point the same as that
            // of the first point
            lon2 = lon1;
        }

        if (lon1 == lon2)
        {
            // segment lies on a meridian
            if (lat1 > lat2)
            {
                std::swap(lat1, lat2);
            }
            return;
        }

        BOOST_GEOMETRY_ASSERT(!is_pole1 && !is_pole2);

        if (lon1 > lon2)
        {
            swap(lon1, lat1, lon2, lat2);
        }

        if (crosses_antimeridian<Units>(lon1, lon2))
        {
            lon1 += constants::period();
            swap(lon1, lat1, lon2, lat2);
        }

        compute_box_corners<Units>(lon1, lat1, lon2, lat2, strategy);
    }

public:
    template <
              typename Units,
              typename CalculationType,
              typename Box,
              typename Strategy
             >
    static inline void apply(CalculationType lon1,
                             CalculationType lat1,
                             CalculationType lon2,
                             CalculationType lat2,
                             Box& mbr,
                             Strategy const& strategy)
    {
        typedef typename coordinate_type<Box>::type box_coordinate_type;

        typedef typename helper_geometry
            <
                Box, box_coordinate_type, Units
            >::type helper_box_type;

        helper_box_type radian_mbr;

        apply<Units>(lon1, lat1, lon2, lat2, strategy);

        geometry::set
            <
                min_corner, 0
            >(radian_mbr, boost::numeric_cast<box_coordinate_type>(lon1));

        geometry::set
            <
                min_corner, 1
            >(radian_mbr, boost::numeric_cast<box_coordinate_type>(lat1));

        geometry::set
            <
                max_corner, 0
            >(radian_mbr, boost::numeric_cast<box_coordinate_type>(lon2));

        geometry::set
            <
                max_corner, 1
            >(radian_mbr, boost::numeric_cast<box_coordinate_type>(lat2));

        geometry::detail::envelope::transform_units(radian_mbr, mbr);
    }
};


}}} //namespace boost::geometry::formula

#endif // BOOST_GEOMETRY_FORMULAS_ENVELOPE_SEGMENT_HPP
