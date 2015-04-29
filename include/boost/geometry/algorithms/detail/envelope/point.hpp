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

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_ENVELOPE_POINT_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_ENVELOPE_POINT_HPP

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/core/coordinate_system.hpp>
#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/core/tags.hpp>

#include <boost/geometry/util/convert_on_spheroid.hpp>
#include <boost/geometry/util/normalize_spheroidal_coordinates.hpp>

#include <boost/geometry/algorithms/assign.hpp>
#include <boost/geometry/algorithms/convert.hpp>

#include <boost/geometry/algorithms/dispatch/envelope.hpp>


namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace envelope
{

struct envelope_point_on_spheroid
{
    template<typename Point, typename Box>
    static inline void apply(Point const& point, Box& mbr)
    {
        typedef typename point_type<Box>::type box_point_type;
        typedef typename coordinate_type<Box>::type box_coordinate_type;

        box_point_type box_point = math::convert_point<box_point_type>(point);

        box_coordinate_type lon = geometry::get<0>(box_point);
        box_coordinate_type lat = geometry::get<1>(box_point);

        math::normalize_spheroidal_coordinates
            <
                typename coordinate_system<Box>::type::units
            >(lon, lat);

        assign_values(mbr, lon, lat, lon, lat);
    }
};


template <typename CSTag>
struct envelope_one_point
{
    template<typename Point, typename Box>
    static inline void apply(Point const& point, Box& mbr)
    {
        geometry::convert(point, mbr);
    }
};

template <>
struct envelope_one_point<spherical_equatorial_tag>
    : envelope_point_on_spheroid
{};

template <>
struct envelope_one_point<geographic_tag>
    : envelope_point_on_spheroid
{};

}} // namespace detail::envelope
#endif // DOXYGEN_NO_DETAIL

#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


template <typename Point>
struct envelope<Point, point_tag>
    : detail::envelope::envelope_one_point<typename cs_tag<Point>::type>
{};


} // namespace dispatch
#endif

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_ENVELOPE_POINT_HPP
