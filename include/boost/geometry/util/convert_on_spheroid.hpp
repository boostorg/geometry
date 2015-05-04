// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2015, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_UTIL_CONVERT_ON_SPHEROID_HPP
#define BOOST_GEOMETRY_UTIL_CONVERT_ON_SPHEROID_HPP

#include <boost/numeric/conversion/cast.hpp>

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/coordinate_system.hpp>
#include <boost/geometry/core/coordinate_type.hpp>

#include <boost/geometry/util/math.hpp>

#include <boost/geometry/algorithms/assign.hpp>
#include <boost/geometry/algorithms/convert.hpp>


namespace boost { namespace geometry
{

namespace math
{


template <typename U, typename T>
inline U deg2rad(T const& value)
{
    static U const d2r = math::pi<U>() / U(180.0);
    return boost::numeric_cast<U>(value) * d2r;
}

template <typename U, typename T>
inline U rad2deg(T const& value)
{
    static U const r2d = U(180.0) / math::pi<U>();
    return boost::numeric_cast<U>(value) * r2d;
}




namespace detail
{

template
<
    typename Point,
    typename PointOut,
    typename PointUnits = typename coordinate_system<Point>::type::units,
    typename PointOutUnits = typename coordinate_system<PointOut>::type::units
>
struct convert_point
{
    static inline void apply(Point const& point, PointOut& point_out)
    {
        geometry::convert(point, point_out);
    }
};


template <typename Point, typename PointOut>
struct convert_point<Point, PointOut, radian, degree>
{
    static inline void apply(Point const& point, PointOut& point_out)
    {
        typedef typename coordinate_type<PointOut>::type out_coordinate_type;

        assign_values(point_out,
            rad2deg<out_coordinate_type>(geometry::get<0>(point)),
            rad2deg<out_coordinate_type>(geometry::get<1>(point)));
    }
};

template <typename Point, typename PointOut>
struct convert_point<Point, PointOut, degree, radian>
{
    static inline void apply(Point const& point, PointOut& point_out)
    {
        typedef typename coordinate_type<PointOut>::type out_coordinate_type;

        assign_values(point_out,
            deg2rad<out_coordinate_type>(geometry::get<0>(point)),
            deg2rad<out_coordinate_type>(geometry::get<1>(point)));
    }
};



template
<
    typename Box,
    typename BoxOut,
    typename BoxUnits = typename coordinate_system<Box>::type::units,
    typename BoxOutUnits = typename coordinate_system<BoxOut>::type::units
>
struct convert_box
{
    static inline void apply(Box const& box, BoxOut& box_out)
    {
        geometry::convert(box, box_out);
    }
};

template <typename Box, typename BoxOut>
struct convert_box<Box, BoxOut, radian, degree>
{
    static inline void apply(Box const& box, BoxOut& box_out)
    {
        typedef typename coordinate_type<BoxOut>::type out_coordinate_type;

        assign_values(box_out,
            rad2deg<out_coordinate_type>(geometry::get<min_corner, 0>(box)),
            rad2deg<out_coordinate_type>(geometry::get<min_corner, 1>(box)),
            rad2deg<out_coordinate_type>(geometry::get<max_corner, 0>(box)),
            rad2deg<out_coordinate_type>(geometry::get<max_corner, 1>(box)));
    }
};

template <typename Box, typename BoxOut>
struct convert_box<Box, BoxOut, degree, radian>
{
    static inline void apply(Box const& box, BoxOut& box_out)
    {
        typedef typename coordinate_type<BoxOut>::type out_coordinate_type;

        assign_values(box_out,
            deg2rad<out_coordinate_type>(geometry::get<min_corner, 0>(box)),
            deg2rad<out_coordinate_type>(geometry::get<min_corner, 1>(box)),
            deg2rad<out_coordinate_type>(geometry::get<max_corner, 0>(box)),
            deg2rad<out_coordinate_type>(geometry::get<max_corner, 1>(box)));
    }
};

template <typename UnitsIn, typename UnitsOut>
struct convert_coordinate
{
    template <typename T>
    static inline void apply(T&)
    {
    }
};

template <>
struct convert_coordinate<degree, radian>
{
    template <typename T>
    static inline void apply(T& value)
    {
        value = deg2rad<T>(value);
    }
};

template <>
struct convert_coordinate<radian, degree>
{
    template <typename T>
    static inline void apply(T& value)
    {
        value = rad2deg<T>(value);
    }
};

} // namespace detail


template <typename BoxOut, typename BoxIn>
inline BoxOut convert_box(BoxIn const& box)
{
    BoxOut box_out;
    detail::convert_box<BoxIn, BoxOut>::apply(box, box_out);
    return box_out;
}

template <typename PointOut, typename PointIn>
inline PointOut convert_point(PointIn const& point)
{
    PointOut point_out;
    detail::convert_point<PointIn, PointOut>::apply(point, point_out);
    return point_out;
}


template <typename UnitsIn, typename UnitsOut, typename T>
inline void convert_coordinates(T& lon, T& lat)
{                
    detail::convert_coordinate<UnitsIn, UnitsOut>::apply(lon);
    detail::convert_coordinate<UnitsIn, UnitsOut>::apply(lat);
}


} // namespace math

}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_UTIL_CONVERT_ON_SPHEROID_HPP
