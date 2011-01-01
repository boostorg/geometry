// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_MAKE_HPP
#define BOOST_GEOMETRY_ALGORITHMS_MAKE_HPP

#include <boost/geometry/algorithms/assign.hpp>

#include <boost/geometry/geometries/concepts/check.hpp>

namespace boost { namespace geometry
{

/*!
    \brief Make a geometry
    \ingroup make
    \details the Generic Geometry Library uses concepts for all its geometries. Therefore it does not rely
    on constructors. The "make" functions are object generators creating geometries. There are overloads
    with two, three, four or six values, which are implemented depending on the geometry specified.
    \note It does not work with array-point types, like int[2]
    \tparam G the geometry type
    \tparam T the coordinate type
    \return the geometry
 */
template <typename Geometry, typename T>
inline Geometry make(T const& c1, T const& c2)
{
    concept::check<Geometry>();

    Geometry geometry;
    dispatch::assign
        <
            typename tag<Geometry>::type,
            Geometry,
            geometry::dimension<Geometry>::type::value
        >::apply(geometry, c1, c2);
    return geometry;
}

/*!
    \brief Make a geometry
    \ingroup make
    \return a 3D point
 */
template <typename Geometry, typename T>
inline Geometry make(T const& c1, T const& c2, T const& c3)
{
    concept::check<Geometry>();

    Geometry geometry;
    dispatch::assign
        <
            typename tag<Geometry>::type,
            Geometry,
            geometry::dimension<Geometry>::type::value
        >::apply(geometry, c1, c2, c3);
    return geometry;
}

template <typename Geometry, typename T>
inline Geometry make(T const& c1, T const& c2, T const& c3, T const& c4)
{
    concept::check<Geometry>();

    Geometry geometry;
    dispatch::assign
        <
            typename tag<Geometry>::type,
            Geometry,
            geometry::dimension<Geometry>::type::value
        >::apply(geometry, c1, c2, c3, c4);
    return geometry;
}



template <typename Geometry, typename Range>
inline Geometry make(Range const& range)
{
    concept::check<Geometry>();

    Geometry geometry;
    append(geometry, range);
    return geometry;
}


/*!
    \brief Create a box with inverse infinite coordinates
    \ingroup make
    \details The make_inverse function initialize a 2D or 3D box with large coordinates, the
    min corner is very large, the max corner is very small
    \tparam Geometry the geometry type
    \return the box
 */
template <typename Geometry>
inline Geometry make_inverse()
{
    concept::check<Geometry>();

    Geometry geometry;
    dispatch::assign_inverse
        <
            typename tag<Geometry>::type,
            Geometry
        >::apply(geometry);
    return geometry;
}

/*!
    \brief Create a geometry with "zero" coordinates
    \ingroup make
    \details The make_zero function initializes a 2D or 3D point or box with coordinates of zero
    \tparam Geometry the geometry type
    \return the geometry
 */
template <typename Geometry>
inline Geometry make_zero()
{
    concept::check<Geometry>();

    Geometry geometry;
    dispatch::assign_zero
        <
            typename tag<Geometry>::type,
            Geometry
        >::apply(geometry);
    return geometry;
}

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_MAKE_HPP
