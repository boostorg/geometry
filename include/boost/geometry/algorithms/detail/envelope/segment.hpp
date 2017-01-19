// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2015 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2015 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2015 Mateusz Loskot, London, UK.

// This file was modified by Oracle on 2015, 2016.
// Modifications copyright (c) 2015-2016, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle
// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_ENVELOPE_SEGMENT_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_ENVELOPE_SEGMENT_HPP

#include <cstddef>
#include <utility>

#include <boost/numeric/conversion/cast.hpp>

#include <boost/geometry/core/assert.hpp>
#include <boost/geometry/core/coordinate_system.hpp>
#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/core/srs.hpp>
#include <boost/geometry/core/point_type.hpp>
#include <boost/geometry/core/radian_access.hpp>
#include <boost/geometry/core/tags.hpp>

#include <boost/geometry/util/math.hpp>

#include <boost/geometry/geometries/helper_geometry.hpp>

#include <boost/geometry/strategies/compare.hpp>
#include <boost/geometry/strategies/geographic/envelope_segment.hpp>

#include <boost/geometry/algorithms/detail/assign_indexed_point.hpp>
#include <boost/geometry/algorithms/detail/normalize.hpp>

#include <boost/geometry/algorithms/detail/envelope/point.hpp>
#include <boost/geometry/algorithms/detail/envelope/transform_units.hpp>

#include <boost/geometry/algorithms/detail/expand/point.hpp>

#include <boost/geometry/algorithms/dispatch/envelope.hpp>

#include <boost/geometry/formulas/vertex_latitude.hpp>

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace envelope
{


template <std::size_t Dimension, std::size_t DimensionCount>
struct envelope_one_segment
{
    template<typename Point, typename Box, typename Strategy>
    static inline void apply(Point const& p1,
                             Point const& p2,
                             Box& mbr,
                             Strategy const& strategy)
    {
        envelope_one_point<Dimension, DimensionCount>::apply(p1, mbr, strategy);
        detail::expand::point_loop
            <
                strategy::compare::default_strategy,
                strategy::compare::default_strategy,
                Dimension,
                DimensionCount
            >::apply(mbr, p2, strategy);
    }
};

template <std::size_t DimensionCount, typename CS_Tag>
struct envelope_segment_on_sphere_or_spheroid
{
    template <typename Point, typename Box, typename Strategy>
    static inline void apply(Point const& p1,
                             Point const& p2,
                             Box& mbr,
                             Strategy const& strategy)
    {
        // first compute the envelope range for the first two coordinates
        Point p1_normalized = detail::return_normalized<Point>(p1);
        Point p2_normalized = detail::return_normalized<Point>(p2);

        strategy.apply(p1_normalized, p2_normalized, mbr);

        // now compute the envelope range for coordinates of
        // dimension 2 and higher
        envelope_one_segment<2, DimensionCount>::apply(p1, p2, mbr, strategy);
    }

    template <typename Segment, typename Box>
    static inline void apply(Segment const& segment, Box& mbr)
    {
        typename point_type<Segment>::type p[2];
        detail::assign_point_from_index<0>(segment, p[0]);
        detail::assign_point_from_index<1>(segment, p[1]);
        apply(p[0], p[1], mbr);
    }
};



template <std::size_t DimensionCount, typename CS_Tag>
struct envelope_segment
    : envelope_one_segment<0, DimensionCount>
{};


template <std::size_t DimensionCount>
struct envelope_segment<DimensionCount, spherical_equatorial_tag>
    : envelope_segment_on_sphere_or_spheroid<DimensionCount, spherical_equatorial_tag>
{};


template <std::size_t DimensionCount>
struct envelope_segment<DimensionCount, geographic_tag>
    : envelope_segment_on_sphere_or_spheroid<DimensionCount, geographic_tag>
{};


}} // namespace detail::envelope
#endif // DOXYGEN_NO_DETAIL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


template <typename Segment, typename CS_Tag>
struct envelope<Segment, segment_tag, CS_Tag>
{
    template <typename Box, typename Strategy>
    static inline void apply(Segment const& segment,
                             Box& mbr,
                             Strategy const& strategy)
    {
        typename point_type<Segment>::type p[2];
        detail::assign_point_from_index<0>(segment, p[0]);
        detail::assign_point_from_index<1>(segment, p[1]);
        detail::envelope::envelope_segment
            <
                dimension<Segment>::value, CS_Tag
            >::apply(p[0], p[1], mbr, strategy);
    }
};

} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_ENVELOPE_SEGMENT_HPP
