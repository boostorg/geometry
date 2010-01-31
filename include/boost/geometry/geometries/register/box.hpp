// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef BOOST_GEOMETRY_GEOMETRIES_REGISTER_BOX_HPP
#define BOOST_GEOMETRY_GEOMETRIES_REGISTER_BOX_HPP


#ifndef DOXYGEN_NO_SPECIALIZATIONS


#define BOOST_GEOMETRY_DETAIL_SPECIALIZE_BOX_ACCESS(Box, Point, MinCorner, MaxCorner) \
template <size_t D> \
struct indexed_access<Box, min_corner, D> \
{ \
    typedef typename coordinate_type<Point>::type ct; \
    static inline ct get(Box const& b) \
    { return geometry::get<D>(b. MinCorner);  } \
    static inline void set(Box& b, ct const& value) \
    { geometry::set<D>(b. MinCorner, value);  } \
}; \
template <size_t D> \
struct indexed_access<Box, max_corner, D> \
{ \
    typedef typename coordinate_type<Point>::type ct; \
    static inline ct get(Box const& b) \
    { return geometry::get<D>(b. MaxCorner);  } \
    static inline void set(Box& b, ct const& value) \
    { geometry::set<D>(b. MaxCorner, value);  } \
};


#define BOOST_GEOMETRY_DETAIL_SPECIALIZE_BOX_ACCESS_TEMPLATIZED(Box, MinCorner, MaxCorner) \
template <typename P, size_t D> \
struct indexed_access<Box<P>, min_corner, D> \
{ \
    typedef typename coordinate_type<P>::type ct; \
    static inline ct get(Box<P> const& b) \
    { return geometry::get<D>(b. MinCorner);  } \
    static inline void set(Box<P>& b, ct const& value) \
    { geometry::set<D>(b. MinCorner, value);  } \
}; \
template <typename P, size_t D> \
struct indexed_access<Box<P>, max_corner, D> \
{ \
    typedef typename coordinate_type<P>::type ct; \
    static inline ct get(Box<P> const& b) \
    { return geometry::get<D>(b. MaxCorner);  } \
    static inline void set(Box<P>& b, ct const& value) \
    { geometry::set<D>(b. MaxCorner, value);  } \
};


#define BOOST_GEOMETRY_DETAIL_SPECIALIZE_BOX_ACCESS_4VALUES(Box, Point, Left, Bottom, Right, Top) \
template <> struct indexed_access<Box, min_corner, 0> \
{ \
    typedef coordinate_type<Point>::type ct; \
    static inline ct get(Box const& b) { return b. Left;  } \
    static inline void set(Box& b, ct const& value) { b. Left = value; } \
}; \
template <> struct indexed_access<Box, min_corner, 1> \
{ \
    typedef coordinate_type<Point>::type ct; \
    static inline ct get(Box const& b) { return b. Bottom;  } \
    static inline void set(Box& b, ct const& value) { b. Bottom = value; } \
}; \
template <> struct indexed_access<Box, max_corner, 0> \
{ \
    typedef coordinate_type<Point>::type ct; \
    static inline ct get(Box const& b) { return b. Right;  } \
    static inline void set(Box& b, ct const& value) { b. Right = value; } \
}; \
template <> struct indexed_access<Box, max_corner, 1> \
{ \
    typedef coordinate_type<Point>::type ct; \
    static inline ct get(Box const& b) { return b. Top; } \
    static inline void set(Box& b, ct const& value) { b. Top = value; } \
};




#define BOOST_GEOMETRY_DETAIL_SPECIALIZE_BOX_TRAITS(Box, PointType) \
    template<> struct tag<Box > { typedef box_tag type; }; \
    template<> struct point_type<Box > { typedef PointType type; };

#define BOOST_GEOMETRY_DETAIL_SPECIALIZE_BOX_TRAITS_TEMPLATIZED(Box) \
    template<typename P> struct tag<Box<P> > { typedef box_tag type; }; \
    template<typename P> struct point_type<Box<P> > { typedef P type; };

#endif // DOXYGEN_NO_SPECIALIZATIONS



#define BOOST_GEOMETRY_REGISTER_BOX(Box, PointType, MinCorner, MaxCorner) \
namespace boost { namespace geometry { namespace traits {  \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_BOX_TRAITS(Box, PointType) \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_BOX_ACCESS(Box, PointType, MinCorner, MaxCorner) \
}}}


#define BOOST_GEOMETRY_REGISTER_BOX_TEMPLATIZED(Box, MinCorner, MaxCorner) \
namespace boost { namespace geometry { namespace traits {  \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_BOX_TRAITS_TEMPLATIZED(Box) \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_BOX_ACCESS_TEMPLATIZED(Box, MinCorner, MaxCorner) \
}}}

#define BOOST_GEOMETRY_REGISTER_BOX_2D_4VALUES(Box, PointType, Left, Bottom, Right, Top) \
namespace boost { namespace geometry { namespace traits {  \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_BOX_TRAITS(Box, PointType) \
    BOOST_GEOMETRY_DETAIL_SPECIALIZE_BOX_ACCESS_4VALUES(Box, PointType, Left, Bottom, Right, Top) \
}}}



// CONST versions are for boxes probably not that common. Postponed.


#endif // BOOST_GEOMETRY_GEOMETRIES_REGISTER_BOX_HPP
