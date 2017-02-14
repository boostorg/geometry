// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2014 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2014 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2014 Mateusz Loskot, London, UK.
// Copyright (c) 2013-2014 Adam Wulkiewicz, Lodz, Poland.

// This file was modified by Oracle on 2013-2017.
// Modifications copyright (c) 2013-2017, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle
// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_DISJOINT_SEGMENT_BOX_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_DISJOINT_SEGMENT_BOX_HPP

#include <cstddef>

#include <boost/geometry/core/tags.hpp>

#include <boost/geometry/algorithms/dispatch/disjoint.hpp>
#include <boost/geometry/algorithms/covered_by.hpp>
#include <boost/geometry/formulas/vertex_longitude.hpp>

namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace disjoint
{

template <typename Segment, typename Box, typename CS_Tag>
struct disjoint_segment_box_sphere_or_spheroid
{
    template <typename CT>
    static inline void swap(CT& lon1,
                            CT& lat1,
                            CT& lon2,
                            CT& lat2)
    {
        std::swap(lon1, lon2);
        std::swap(lat1, lat2);
    }


    template <typename CT>
    static inline CT compute_vertex_lon(CT lon1,
                                        CT lat1,
                                        CT lon2,
                                        CT lat2,
                                        CT vertex_lat,
                                        CT alp1)
    {
        if (vertex_lat == lat1)
        {
            return lon1;
        }
        if (vertex_lat == lat2)
        {
            return lon2;
        }
        if (lat1 == lat2)
        {
            return formula::vertex_longitude<CT, CS_Tag>
                    ::apply(lat1, lat2, vertex_lat, lon2 - lon1, alp1)
                    + lon2;
        }
        else
        {
            return formula::vertex_longitude<CT, CS_Tag>
                    ::apply(lat1, lat2, vertex_lat, lon2 - lon1, alp1)
                    + lon2;
        }
    }

    template <typename Strategy>
    static inline bool apply(Segment const& segment,
                             Box const& box,
                             Strategy const& azimuth_strategy)
    {
        assert_dimension_equal<Segment, Box>();

        typedef typename point_type<Segment>::type segment_point_type;
        segment_point_type p0, p1;
        geometry::detail::assign_point_from_index<0>(segment, p0);
        geometry::detail::assign_point_from_index<1>(segment, p1);

        bool contains_p0 = geometry::covered_by(p0, box);
        bool contains_p1 = geometry::covered_by(p1, box);

        // Test simple cases first

        if (contains_p0 && contains_p1)
        {
            return true;
        }
        if (contains_p0 != contains_p1)
        {
            return false;
        }

        // Test intersection by comparing angles

        typedef typename coordinate_type<segment_point_type>::type CT;

        segment_point_type p0_normalized =
                geometry::detail::return_normalized<segment_point_type>(p0);
        segment_point_type p1_normalized =
                geometry::detail::return_normalized<segment_point_type>(p1);

        CT lon1 = geometry::get_as_radian<0>(p0_normalized);
        CT lat1 = geometry::get_as_radian<1>(p0_normalized);
        CT lon2 = geometry::get_as_radian<0>(p1_normalized);
        CT lat2 = geometry::get_as_radian<1>(p1_normalized);

        if (lon1 > lon2)
        {
            swap(lon1, lat1, lon2, lat2);
        }

        CT alp1, a_b0, a_b1, a_b2, a_b3;

        CT b_lon_min = geometry::get<geometry::min_corner, 0>(box) * math::d2r<CT>();
        CT b_lat_min = geometry::get<geometry::min_corner, 1>(box) * math::d2r<CT>();
        CT b_lon_max = geometry::get<geometry::max_corner, 0>(box) * math::d2r<CT>();
        CT b_lat_max = geometry::get<geometry::max_corner, 1>(box) * math::d2r<CT>();

        azimuth_strategy.apply(lon1, lat1, lon2, lat2, alp1);
        azimuth_strategy.apply(lon1, lat1, b_lon_min, b_lat_min, a_b0);
        azimuth_strategy.apply(lon1, lat1, b_lon_max, b_lat_min, a_b1);
        azimuth_strategy.apply(lon1, lat1, b_lon_min, b_lat_max, a_b2);
        azimuth_strategy.apply(lon1, lat1, b_lon_max, b_lat_max, a_b3);

        bool b0(alp1 > a_b0), b1(alp1 > a_b1), b2(alp1 > a_b2), b3(alp1 > a_b3);

/*
        std::cout << b0 << b1 << b2 << b3 << (b0 & b1 & b2 & b3)
                     << !(b0 & b1 & b2 & b3)
                     << (b0 | b1 | b2 | b3)
                     << std::endl;
*/

        if (~(b0 & b1 & b2 & b3) & (b0 | b1 | b2 | b3))
        {
            return false;
        }

        // Test intersection by testing the vertex of the geodesic

        geometry::model::box<segment_point_type> box_seg;

        //Do not compute alp1 twice, pass the already computed alp1 to envelope_segment_impl
        geometry::detail::envelope::envelope_segment_impl<typename cs_tag<Segment>::type>
                ::template apply<geometry::radian>(lon1, lat1, lon2, lat2,
                                                   box_seg, azimuth_strategy, alp1);

        CT vertex_lat = geometry::get<geometry::max_corner, 1>(box_seg) * math::d2r<CT>();
        //std::cout << "vrt_lat=" << math::from_radian<degree>(vertex_lat) << std::endl;

        CT vertex_lon = compute_vertex_lon(lon1, lat1, lon2, lat2, vertex_lat, alp1);
        //std::cout << "vrt_lon=" << math::from_radian<degree>(vertex_lon) << std::endl;

        segment_point_type p_vertex_deg(vertex_lon * math::r2d<CT>(),
                                        vertex_lat * math::r2d<CT>());

        if (geometry::covered_by(p_vertex_deg, box))
        {
            return false;
        }

        return true;
    }
};

struct disjoint_segment_box
{
    template <typename Segment, typename Box, typename Strategy>
    static inline bool apply(Segment const& segment, Box const& box, Strategy const& strategy)
    {
        return strategy.apply(segment, box);
    }
};

}} // namespace detail::disjoint
#endif // DOXYGEN_NO_DETAIL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


template <typename Segment, typename Box, std::size_t DimensionCount>
struct disjoint<Segment, Box, DimensionCount, segment_tag, box_tag, false>
    : detail::disjoint::disjoint_segment_box
{};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_DISJOINT_SEGMENT_BOX_HPP
