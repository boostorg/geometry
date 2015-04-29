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

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_ENVELOPE_BOX_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_ENVELOPE_BOX_HPP

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/core/coordinate_system.hpp>
#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/core/tags.hpp>

#include <boost/geometry/util/math.hpp>
#include <boost/geometry/util/normalize_spheroidal_coordinates.hpp>
#include <boost/geometry/util/convert_on_spheroid.hpp>

#include <boost/geometry/algorithms/assign.hpp>
#include <boost/geometry/algorithms/convert.hpp>

#include <boost/geometry/algorithms/dispatch/envelope.hpp>


namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace envelope
{


struct envelope_box_on_spheroid
{
    template <typename Box>
    static inline bool is_band(Box const& box)
    {
        typedef typename coordinate_type<Box>::type coordinate_type;
        coordinate_type const period = math::detail::constants_on_spheroid
            <
                coordinate_type,
                typename coordinate_system<Box>::type::units
            >::period();

        coordinate_type longitude_diff = geometry::get<max_corner, 0>(box)
            - geometry::get<min_corner, 0>(box);

        return ! math::smaller(math::abs(longitude_diff), period);
    }

    template<typename BoxIn, typename BoxOut>
    static inline void apply(BoxIn const& box_in, BoxOut& mbr)
    {
        typedef typename coordinate_type<BoxOut>::type box_out_coordinate_type;
        typedef typename coordinate_system<BoxOut>::type::units mbr_units_type;

        bool const band = is_band(box_in);

        mbr = math::convert_box<BoxOut>(box_in);
        box_out_coordinate_type lon1 = geometry::get<min_corner, 0>(mbr);
        box_out_coordinate_type lat1 = geometry::get<min_corner, 1>(mbr);
        box_out_coordinate_type lon2 = geometry::get<max_corner, 0>(mbr);
        box_out_coordinate_type lat2 = geometry::get<max_corner, 1>(mbr);

        math::normalize_spheroidal_box_coordinates
            <
                mbr_units_type
            >(lon1, lat1, lon2, lat2, band);

        assign_values(mbr, lon1, lat1, lon2, lat2);
    }
};


template <typename CSTag>
struct envelope_box
{
    template<typename BoxIn, typename BoxOut>
    static inline void apply(BoxIn const& box_in, BoxOut& mbr)
    {
        geometry::convert(box_in, mbr);
    }
};

template <>
struct envelope_box<spherical_equatorial_tag>
    : envelope_box_on_spheroid
{};

template <>
struct envelope_box<geographic_tag>
    : envelope_box_on_spheroid
{};


}} // namespace detail::envelope
#endif // DOXYGEN_NO_DETAIL

#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


template <typename Box>
struct envelope<Box, box_tag>
    : detail::envelope::envelope_box<typename cs_tag<Box>::type>
{};


} // namespace dispatch
#endif

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_ENVELOPE_BOX_HPP
