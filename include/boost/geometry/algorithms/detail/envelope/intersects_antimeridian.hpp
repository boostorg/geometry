// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2015, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_ENVELOPE_INTERSECTS_ANTIMERIDIAN_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_ENVELOPE_INTERSECTS_ANTIMERIDIAN_HPP

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/coordinate_system.hpp>
#include <boost/geometry/core/coordinate_type.hpp>

#include <boost/geometry/util/math.hpp>
#include <boost/geometry/util/normalize_spheroidal_coordinates.hpp>


namespace boost { namespace geometry                  
{

namespace detail { namespace envelope
{


struct intersects_antimeridian
{
    template <typename Units, typename CoordinateType>
    static inline bool apply(CoordinateType const& lon1,
                             CoordinateType const& lat1,
                             CoordinateType const& lon2,
                             CoordinateType const& lat2)
    {
        typedef math::detail::constants_on_spheroid
            <
                CoordinateType, Units
            > constants;
    
        return
            math::equals(math::abs(lat1), constants::max_latitude())
            ||
            math::equals(math::abs(lat2), constants::max_latitude())
            ||
            math::larger(math::abs(lon1 - lon2), constants::half_period());
    }

    template <typename Segment>
    static inline bool apply(Segment const& segment)
    {
        typedef typename coordinate_type<Segment>::type coordinate_type;
        typedef typename coordinate_system<Segment>::type::units units_type;

        coordinate_type lon1 = geometry::get<0, 0>(segment);
        coordinate_type lat1 = geometry::get<0, 1>(segment);
        coordinate_type lon2 = geometry::get<1, 0>(segment);
        coordinate_type lat2 = geometry::get<1, 1>(segment);

        math::normalize_spheroidal_coordinates
            <
                units_type, coordinate_type
            >(lon1, lat1);

        math::normalize_spheroidal_coordinates
            <
                units_type, coordinate_type
            >(lon2, lat2);

        return apply<units_type>(lon1, lat1, lon2, lat2);
    }

    template <typename Point>
    static inline bool apply(Point const& p1, Point const& p2)
    {
        typedef typename coordinate_type<Point>::type coordinate_type;
        typedef typename coordinate_system<Point>::type::units units_type;

        coordinate_type lon1 = geometry::get<0>(p1);
        coordinate_type lat1 = geometry::get<1>(p1);
        coordinate_type lon2 = geometry::get<0>(p2);
        coordinate_type lat2 = geometry::get<1>(p2);

        math::normalize_spheroidal_coordinates
            <
                units_type, coordinate_type
            >(lon1, lat1);

        math::normalize_spheroidal_coordinates
            <
                units_type, coordinate_type
            >(lon2, lat2);

        return apply<units_type>(lon1, lat1, lon2, lat2);
    }
};


}} // namespace detail::envelope

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_ENVELOPE_INTERSECTS_ANTIMERIDIAN_HPP
