// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2012 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2012 Mateusz Loskot, London, UK.

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_BALL_CORE_ACCESS_HPP
#define BOOST_GEOMETRY_EXTENSIONS_BALL_CORE_ACCESS_HPP



#include <boost/geometry/core/access.hpp>

#include <boost/geometry/extensions/ball/core/tags.hpp>


namespace boost { namespace geometry
{

int const center_point = 2;

int const radius_value = 3;

#ifndef DOXYGEN_NO_DISPATCH
namespace core_dispatch
{

template
<
    typename Ball,
    typename CoordinateType,
    std::size_t Dimension
>
struct indexed_access<ball_tag, Ball, CoordinateType, center_point, Dimension, boost::false_type>
{
    static inline CoordinateType get(Ball const& b)
    {
        return traits::indexed_access<Ball, center_point, Dimension>::get(b);
    }
    static inline void set(Ball& b, CoordinateType const& value)
    {
        traits::indexed_access<Ball, center_point, Dimension>::set(b, value);
    }
};


/*!
    \brief Traits class to get/set radius of a circle/sphere/(ellipse)
    \details the radius access meta-functions give read/write access to the radius of a circle or a sphere,
    or to the major/minor axis or an ellipse, or to one of the 3 equatorial radii of an ellipsoid.

    It should be specialized per geometry, in namespace core_dispatch. Those specializations should
    forward the call via traits to the geometry class, which could be specified by the user.

    There is a corresponding generic radius_get and radius_set function
    \par Geometries:
        - n-sphere (circle,sphere)
        - upcoming ellipse
    \par Specializations should provide:
        - inline static T get(G const& geometry)
        - inline static void set(G& geometry, T const& radius)
    \ingroup traits
*/
template
<
    typename Ball,
    typename RadiusType,
    std::size_t Dimension
>
struct indexed_access<ball_tag, Ball, RadiusType, radius_value, Dimension, boost::false_type>
{
    static inline RadiusType get(Ball const& b)
    {
        return traits::indexed_access<Ball, radius_value, Dimension>::get(b);
    }
    static inline void set(Ball& b, RadiusType const& value)
    {
        traits::indexed_access<Ball, radius_value, Dimension>::set(b, value);
    }
};


} // namespace core_dispatch
#endif // DOXYGEN_NO_DISPATCH


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_EXTENSIONS_BALL_CORE_ACCESS_HPP
