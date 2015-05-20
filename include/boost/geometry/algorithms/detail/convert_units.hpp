// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2015, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_CONVERT_UNITS_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_CONVERT_UNITS_HPP

#include <boost/type_traits/is_same.hpp>

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tags.hpp>

#include <boost/geometry/util/math.hpp>

#include <boost/geometry/views/detail/indexed_point_view.hpp>


namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace units_conversion
{


template <typename UnitsFrom, typename UnitsTo>
struct convert_coordinate
{};

template <>
struct convert_coordinate<degree, radian>
{
    template <typename CoordinateType>
    static inline CoordinateType apply(CoordinateType const& value)
    {
        return value * math::d2r<CoordinateType>();
    }
};

template <>
struct convert_coordinate<radian, degree>
{
    template <typename CoordinateType>
    static inline CoordinateType apply(CoordinateType const& value)
    {
        return value * math::r2d<CoordinateType>();
    }
};


template
<
    typename UnitsFrom,
    typename UnitsTo,
    bool SameUnits = boost::is_same<UnitsFrom, UnitsTo>::value
>
struct convert_point
{
    template <std::size_t Dimension, typename Point>
    static inline void convert_units(Point& point)
    {
        geometry::set
            <
                Dimension
            >(point,
              convert_coordinate
                  <
                      UnitsFrom, UnitsTo
                  >::apply(geometry::get<Dimension>(point)));
    }

    template <typename Point>
    static inline void apply(Point& point)
    {
        convert_units<0>(point);
        convert_units<1>(point);
    }
};

template <typename UnitsFrom, typename UnitsTo>
struct convert_point<UnitsFrom, UnitsTo, true>
{
    template <typename Point>
    static inline void apply(Point&)
    {
    }
};



template
<
    typename UnitsFrom,
    typename UnitsTo,
    bool SameUnits = boost::is_same<UnitsFrom, UnitsTo>::value
>
struct convert_box
{
    template <typename Box>
    static inline void apply(Box& box)
    {
        detail::indexed_point_view<Box, min_corner> p_min(box);
        detail::indexed_point_view<Box, max_corner> p_max(box);

        convert_point<UnitsFrom, UnitsTo>::apply(p_min);
        convert_point<UnitsFrom, UnitsTo>::apply(p_max);
    }
};

template <typename UnitsFrom, typename UnitsTo>
struct convert_box<UnitsFrom, UnitsTo, true>
{
    template <typename Box>
    static inline void apply(Box&)
    {
    }
};


}} // namespace detail::units_conversion
#endif // DOXYGEN_NO_DETAIL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


template
<
    typename UnitsFrom,
    typename UnitsTo,
    typename Geometry,
    typename Tag = typename tag<Geometry>::type
>
struct convert_units
{};

template <typename UnitsFrom, typename UnitsTo, typename Point>
struct convert_units<UnitsFrom, UnitsTo, Point, point_tag>
    : detail::units_conversion::convert_point<UnitsFrom, UnitsTo>
{};

template <typename UnitsFrom, typename UnitsTo, typename Box>
struct convert_units<UnitsFrom, UnitsTo, Box, box_tag>
    : detail::units_conversion::convert_box<UnitsFrom, UnitsTo>
{};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH

#ifndef DOXYGEN_NO_DETAIL
namespace detail
{


template <typename UnitsFrom, typename UnitsTo, typename Geometry>
inline void convert_units(Geometry& geometry)
{
    dispatch::convert_units<UnitsFrom, UnitsTo, Geometry>::apply(geometry);
}


} // namespace detail
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_CONVERT_UNITS_HPP
