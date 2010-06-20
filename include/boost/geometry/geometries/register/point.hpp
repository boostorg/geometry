// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_GEOMETRIES_REGISTER_POINT_HPP
#define BOOST_GEOMETRY_GEOMETRIES_REGISTER_POINT_HPP


#include <cstddef>

/**
 * @file point.hpp
 * @brief Macros for registration of custom point geometry types.
 * This file implements a "macro party" for registration of custom geometry types.
 */

#ifndef DOXYGEN_NO_SPECIALIZATIONS

// Starting point, specialize basic traits necessary to register a point
#define BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_TRAITS(Point, Dim, CoordinateType, CoordinateSystem) \
    template<> struct tag<Point> { typedef point_tag type; }; \
    template<> struct dimension<Point> : boost::mpl::int_<Dim> {}; \
    template<> struct coordinate_type<Point> { typedef CoordinateType type; }; \
    template<> struct coordinate_system<Point> { typedef CoordinateSystem type; }; 

// Specialize access class per dimension
#define BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESS(Point, Dim, CoordinateType, Get, Set) \
    template<> struct access<Point, Dim> \
    { \
        static inline CoordinateType get(Point const& p) { return p. Get; } \
        static inline void set(Point& p, CoordinateType const& value) { p. Set = value; } \
    };

// Const version
#define BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESS_CONST(Point, Dim, CoordinateType, Get) \
    template<> struct access<Point, Dim> \
    { \
        static inline CoordinateType get(Point const& p) { return p. Get; } \
    };


// Getter/setter version
#define BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESS_GET_SET(Point, Dim, CoordinateType, Get, Set) \
    template<> struct access<Point, Dim> \
    { \
        static inline CoordinateType get(Point const& p) \
        { return  p. Get (); } \
        static inline void set(Point& p, CoordinateType const& value) \
        { p. Set ( value ); } \
    };

#endif // DOXYGEN_NO_SPECIALIZATIONS


/// @brief Library user macro to register a custom 2D point
#define BOOST_GEOMETRY_REGISTER_POINT_2D(Point, CoordinateType, CoordinateSystem, Field0, Field1) \
namespace boost { namespace geometry { namespace traits {  \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_TRAITS(Point, 2, CoordinateType, CoordinateSystem) \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESS(Point, 0, CoordinateType, Field0, Field0) \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESS(Point, 1, CoordinateType, Field1, Field1) \
}}}

/// @brief Library user macro to register a custom 3D point
#define BOOST_GEOMETRY_REGISTER_POINT_3D(Point, CoordinateType, CoordinateSystem, Field0, Field1, Field2) \
namespace boost { namespace geometry { namespace traits {  \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_TRAITS(Point, 3, CoordinateType, CoordinateSystem) \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESS(Point, 0, CoordinateType, Field0, Field0) \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESS(Point, 1, CoordinateType, Field1, Field1) \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESS(Point, 2, CoordinateType, Field2, Field2) \
}}}

/// @brief Library user macro to register a custom 2D point (CONST version)
#define BOOST_GEOMETRY_REGISTER_POINT_2D_CONST(Point, CoordinateType, CoordinateSystem, Field0, Field1) \
namespace boost { namespace geometry { namespace traits {  \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_TRAITS(Point, 2, CoordinateType, CoordinateSystem) \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESS_CONST(Point, 0, CoordinateType, Field0) \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESS_CONST(Point, 1, CoordinateType, Field1) \
}}}

/// @brief Library user macro to register a custom 3D point (CONST version)
#define BOOST_GEOMETRY_REGISTER_POINT_3D_CONST(Point, CoordinateType, CoordinateSystem, Field0, Field1, Field2) \
namespace boost { namespace geometry { namespace traits {  \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_TRAITS(Point, 3, CoordinateType, CoordinateSystem) \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESS_CONST(Point, 0, CoordinateType, Field0) \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESS_CONST(Point, 1, CoordinateType, Field1) \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESS_CONST(Point, 2, CoordinateType, Field2) \
}}}

/// @brief Library user macro to register a custom 2D point (having separate get/set methods)
#define BOOST_GEOMETRY_REGISTER_POINT_2D_GET_SET(Point, CoordinateType, CoordinateSystem, Get0, Get1, Set0, Set1) \
namespace boost { namespace geometry { namespace traits {  \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_TRAITS(Point, 2, CoordinateType, CoordinateSystem) \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESS_GET_SET(Point, 0, CoordinateType, Get0, Set0) \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESS_GET_SET(Point, 1, CoordinateType, Get1, Set1) \
}}}

/// @brief Library user macro to register a custom 3D point (having separate get/set methods)
#define BOOST_GEOMETRY_REGISTER_POINT_3D_GET_SET(Point, CoordinateType, CoordinateSystem, Get0, Get1, Get2, Set0, Set1, Set2) \
namespace boost { namespace geometry { namespace traits {  \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_TRAITS(Point, 3, CoordinateType, CoordinateSystem) \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESS_GET_SET(Point, 0, CoordinateType, Get0, Set0) \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESS_GET_SET(Point, 1, CoordinateType, Get1, Set1) \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESS_GET_SET(Point, 2, CoordinateType, Get2, Set2) \
}}}

#endif // BOOST_GEOMETRY_GEOMETRIES_REGISTER_POINT_HPP
