// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_GEOMETRIES_ADAPTED_BOOST_POLYGON_POINT_HPP
#define BOOST_GEOMETRY_GEOMETRIES_ADAPTED_BOOST_POLYGON_POINT_HPP


#include <cstddef>

#include <boost/polygon/polygon.hpp>

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/core/coordinate_dimension.hpp>
#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/core/tags.hpp>

#include <boost/polygon/polygon.hpp>

namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_TRAITS_SPECIALIZATIONS
namespace traits
{



// Assign the point-tag, preventing arrays of points getting a point-tag
template <typename CoordinateType>
struct tag<boost::polygon::point_data<CoordinateType> >
{
    typedef point_tag type;
};


template <typename CoordinateType>
struct coordinate_type<boost::polygon::point_data<CoordinateType> >
{
    typedef CoordinateType type;
};

template <typename CoordinateType>
struct coordinate_system<boost::polygon::point_data<CoordinateType> >
{
    typedef cs::cartesian type;
};


template <typename CoordinateType>
struct dimension<boost::polygon::point_data<CoordinateType> >: boost::mpl::int_<2> {};


template <typename CoordinateType>
struct access<boost::polygon::point_data<CoordinateType>, 0>
{
    static inline CoordinateType get(boost::polygon::point_data<CoordinateType> const& p)
    {
        return p.x();
    }

    static inline void set(boost::polygon::point_data<CoordinateType>& p,
        CoordinateType const& value)
    {
        p.x(value);
    }
};

template <typename CoordinateType>
struct access<boost::polygon::point_data<CoordinateType>, 1>
{
    static inline CoordinateType get(boost::polygon::point_data<CoordinateType> const& p)
    {
        return p.y();
    }

    static inline void set(boost::polygon::point_data<CoordinateType>& p,
        CoordinateType const& value)
    {
        p.y(value);
    }
};


} // namespace traits
#endif // DOXYGEN_NO_TRAITS_SPECIALIZATIONS


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_GEOMETRIES_ADAPTED_BOOST_POLYGON_POINT_HPP

