// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef BOOST_GEOMETRY_GEOMETRIES_REGISTER_POINT_HPP
#define BOOST_GEOMETRY_GEOMETRIES_REGISTER_POINT_HPP



// This file implements a "macro party",
// for registration of custom geometry types

#ifndef DOXYGEN_NO_SPECIALIZATIONS

// Starting point, specialize basic traits necessary to register a point
// (the 'accessor' is technically not specialization but definition, specialized in another macro)
#define BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_TRAITS(Point, Dim, CoordinateType, CoordinateSystem) \
    template<> struct tag<Point> { typedef point_tag type; }; \
    template<> struct dimension<Point> : boost::mpl::int_<Dim> {}; \
    template<> struct coordinate_type<Point> { typedef CoordinateType type; }; \
    template<> struct coordinate_system<Point> { typedef CoordinateSystem type; }; \
    template<std::size_t Dimension> struct Point##accessor {};


// Non Const version
#define BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESS(Point, CoordinateType) \
    template<std::size_t Dimension> struct access<Point, Dimension> { \
        static inline CoordinateType get(Point const& p) \
        { return Point##accessor<Dimension>::get(p); } \
        static inline void set(Point& p, CoordinateType const& value) \
        { Point##accessor<Dimension>::set(p, value); } \
    };

// Const version
#define BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESS_CONST(Point, CoordinateType) \
    template<std::size_t Dimension> struct access<Point, Dimension> { \
        static inline CoordinateType get(Point const& p) \
        { return Point##accessor<Dimension>::get(p); } \
    };




// Specialize the point-specific-accessor class, declared below, per dimension
#define BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESSOR(Point, Dim, CoordinateType, Get, Set) \
    template<> struct Point##accessor< Dim > \
    { \
        static inline CoordinateType get(Point const& p) { return p. Get; } \
        static inline void set(Point& p, CoordinateType const& value) { p. Set = value; } \
    };


// Const version
#define BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESSOR_CONST(Point, Dim, CoordinateType, Get) \
    template<> struct Point##accessor< Dim > \
    { \
        static inline CoordinateType get(Point const& p) \
        { return p. Get; } \
    };


// Get/set version
#define BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESSOR_GET_SET(Point, Dim, CoordinateType, Get, Set) \
    template<> struct Point##accessor< Dim > \
    { \
        static inline CoordinateType get(Point const& p) \
        { return  p. Get (); } \
        static inline void set(Point& p, CoordinateType const& value) \
        { p. Set ( value ); } \
    };



#define BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESS_2D(Point, CoordinateType, Get0, Get1, Set0, Set1) \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESSOR(Point, 0, CoordinateType, Get0, Set0) \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESSOR(Point, 1, CoordinateType, Get1, Set1)

#define BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESS_3D(Point, CoordinateType, Get0, Get1, Get2, Set0, Set1, Set2) \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESSOR(Point, 0, CoordinateType, Get0, Set0) \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESSOR(Point, 1, CoordinateType, Get1, Set1) \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESSOR(Point, 2, CoordinateType, Get2, Set2)

// Const versions
#define BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESS_CONST_2D(Point, CoordinateType, Get0, Get1) \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESSOR_CONST(Point, 0, CoordinateType, Get0) \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESSOR_CONST(Point, 1, CoordinateType, Get1)

#define BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESS_CONST_3D(Point, CoordinateType, Get0, Get1, Get2) \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESSOR_CONST(Point, 0, CoordinateType, Get0) \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESSOR_CONST(Point, 1, CoordinateType, Get1) \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESSOR_CONST(Point, 2, CoordinateType, Get2)

#endif // DOXYGEN_NO_SPECIALIZATIONS



// Library user macro to register a custom 2D point
#define BOOST_GEOMETRY_REGISTER_POINT_2D(Point, CoordinateType, CoordinateSystem, Field0, Field1) \
namespace boost { namespace geometry { namespace traits {  \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_TRAITS(Point, 2, CoordinateType, CoordinateSystem) \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESS(Point, CoordinateType) \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESS_2D(Point, CoordinateType, Field0, Field1, Field0, Field1) \
}}}

// Library user macro to register a custom 3D point
#define BOOST_GEOMETRY_REGISTER_POINT_3D(Point, CoordinateType, CoordinateSystem, Field0, Field1, Field2) \
namespace boost { namespace geometry { namespace traits {  \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_TRAITS(Point, 3, CoordinateType, CoordinateSystem) \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESS(Point, CoordinateType) \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESS_3D(Point, CoordinateType, Field0, Field1, Field2, Field0, Field1, Field2) \
}}}



// Library user macro to register a custom 2D point (CONST version)
#define BOOST_GEOMETRY_REGISTER_POINT_2D_CONST(Point, CoordinateType, CoordinateSystem, Field0, Field1) \
namespace boost { namespace geometry { namespace traits {  \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_TRAITS(Point, 2, CoordinateType, CoordinateSystem) \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESS_CONST(Point, CoordinateType) \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESS_CONST_2D(Point, CoordinateType, Field0, Field1) \
}}}

// Library user macro to register a custom 3D point (CONST version)
#define BOOST_GEOMETRY_REGISTER_POINT_3D_CONST(Point, CoordinateType, CoordinateSystem, Field0, Field1, Field2) \
namespace boost { namespace geometry { namespace traits {  \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_TRAITS(Point, 3, CoordinateType, CoordinateSystem) \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESS_CONST(Point, CoordinateType) \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESS_CONST_3D(Point, CoordinateType, Field0, Field1, Field2) \
}}}


// Library user macro to register a custom 2D point (having separate get/set methods)
#define BOOST_GEOMETRY_REGISTER_POINT_2D_GET_SET(Point, CoordinateType, CoordinateSystem, Get0, Get1, Set0, Set1) \
namespace boost { namespace geometry { namespace traits {  \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_TRAITS(Point, 2, CoordinateType, CoordinateSystem) \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESS(Point, CoordinateType) \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESSOR_GET_SET(Point, 0, CoordinateType, Get0, Set0) \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESSOR_GET_SET(Point, 1, CoordinateType, Get1, Set1) \
}}}


// Library user macro to register a custom 3D point (having separate get/set methods)
#define BOOST_GEOMETRY_REGISTER_POINT_3D_GET_SET(Point, CoordinateType, CoordinateSystem, Get0, Get1, Get2, Set0, Set1, Set2) \
namespace boost { namespace geometry { namespace traits {  \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_TRAITS(Point, 3, CoordinateType, CoordinateSystem) \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESS(Point, CoordinateType) \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESSOR_GET_SET(Point, 0, CoordinateType, Get0, Set0) \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESSOR_GET_SET(Point, 1, CoordinateType, Get1, Set1) \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_POINT_ACCESSOR_GET_SET(Point, 2, CoordinateType, Get2, Set2) \
}}}


#endif // BOOST_GEOMETRY_GEOMETRIES_REGISTER_POINT_HPP
