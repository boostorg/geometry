// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2015 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2015 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2015 Mateusz Loskot, London, UK.

// This file was modified by Oracle on 2015.
// Modifications copyright (c) 2015, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_ENVELOPE_LINESTRING_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_ENVELOPE_LINESTRING_HPP

#include <iterator>
#include <vector>

#include <boost/assert.hpp>
#include <boost/numeric/conversion/cast.hpp>

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/coordinate_system.hpp>
#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/core/point_type.hpp>
#include <boost/geometry/core/tags.hpp>

#include <boost/geometry/iterators/segment_iterator.hpp>

#include <boost/geometry/util/convert_on_spheroid.hpp>
#include <boost/geometry/util/math.hpp>
#include <boost/geometry/util/normalize_spheroidal_coordinates.hpp>

#include <boost/geometry/algorithms/assign.hpp>
#include <boost/geometry/algorithms/num_points.hpp>

#include <boost/geometry/algorithms/detail/envelope/segment.hpp>
#include <boost/geometry/algorithms/detail/envelope/range.hpp>
#include <boost/geometry/algorithms/detail/envelope/range_of_boxes.hpp>

#include <boost/geometry/algorithms/dispatch/envelope.hpp>


namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace envelope
{


struct envelope_linear_on_spheroid
{
    template <typename Longitude, typename OutputIterator>
    static inline OutputIterator push_interval(Longitude const& lon1,
                                               Longitude const& lon2,
                                               OutputIterator oit)
    {
        typedef longitude_interval<Longitude> interval_type;

        typedef math::detail::constants_on_spheroid
            <
                Longitude, radian
            > constants;

        BOOST_ASSERT(! math::larger(lon1, lon2));
        BOOST_ASSERT(! math::larger(lon1, constants::max_longitude()));

        if (math::larger(lon2, constants::max_longitude()))
        {
            *oit++ = interval_type(lon1, constants::max_longitude());
            *oit++ = interval_type(constants::min_longitude(),
                                   lon2 - constants::period());
        }
        else
        {
            *oit++ = interval_type(lon1, lon2);
        }
        return oit;
    }

    template <typename Linear, typename Box>
    static inline void apply(Linear const& linear, Box& mbr)
    {
        typedef typename coordinate_type<Box>::type box_coordinate_type;
        typedef typename fp_coordinate_type<Linear>::type calculation_type;

        typedef typename geometry::segment_iterator
            <
                Linear const
            > iterator_type;

        typedef typename coordinate_system
            <
                Linear
            >::type::units linear_units_type;

        BOOST_ASSERT(geometry::num_points(linear) != 0);

        typedef longitude_interval<calculation_type> interval_type;

        calculation_type const pi = math::pi<calculation_type>();

        std::vector<interval_type> longitude_intervals;
        std::back_insert_iterator
            <
                std::vector<interval_type>
            > oit(longitude_intervals);

        calculation_type lat_min = 0, lat_max = 0;
        bool first = true;
        for (iterator_type seg_it = geometry::segments_begin(linear);
             seg_it != geometry::segments_end(linear);
             ++seg_it, first = false)
        {
            typename point_type<Linear>::type p[2];
            detail::assign_point_from_index<0>(*seg_it, p[0]);
            detail::assign_point_from_index<1>(*seg_it, p[1]);

            calculation_type lon1 = geometry::get<0>(p[0]);
            calculation_type lat1 = geometry::get<1>(p[0]);
            calculation_type lon2 = geometry::get<0>(p[1]);
            calculation_type lat2 = geometry::get<1>(p[1]);

            math::normalize_spheroidal_coordinates
                <
                    linear_units_type
                >(lon1, lat1);

            math::normalize_spheroidal_coordinates
                <
                    linear_units_type
                >(lon2, lat2);

            math::convert_coordinates<linear_units_type, radian>(lon1, lat1);
            math::convert_coordinates<linear_units_type, radian>(lon2, lat2);

            compute_mbr_of_segment::apply(lon1, lat1, lon2, lat2); 
            oit = push_interval(lon1, lon2, oit);

            if (first)
            {
                lat_min = lat1;
                lat_max = lat2;
            }

            // update min and max latitude, if needed
            if (math::smaller(lat1, lat_min))
            {
                lat_min = lat1;
            }

            if (math::larger(lat2, lat_max))
            {
                lat_max = lat2;
            }
        }

        calculation_type lon_min = 0, lon_max = 0;
        envelope_range_of_longitudes
            <
                radian
            >::apply(longitude_intervals, lon_min, lon_max);

        math::convert_coordinates
            <
                radian, typename coordinate_system<Box>::type::units
            >(lon_min, lat_min);

        math::convert_coordinates
            <
                radian, typename coordinate_system<Box>::type::units
            >(lon_max, lat_max);

        assign_values(mbr,
                      boost::numeric_cast<box_coordinate_type>(lon_min),
                      boost::numeric_cast<box_coordinate_type>(lat_min),
                      boost::numeric_cast<box_coordinate_type>(lon_max),
                      boost::numeric_cast<box_coordinate_type>(lat_max));
    }
};


template <typename CSTag>
struct envelope_linestring
    : envelope_range<>
{};

template <>
struct envelope_linestring<spherical_equatorial_tag>
    : envelope_linear_on_spheroid
{};


}} // namespace detail::envelope
#endif // DOXYGEN_NO_DETAIL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template <typename Linestring>
struct envelope<Linestring, linestring_tag>
    : detail::envelope::envelope_linestring<typename cs_tag<Linestring>::type>
{};

} // namespace dispatch
#endif


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_ENVELOPE_LINESTRING_HPP
