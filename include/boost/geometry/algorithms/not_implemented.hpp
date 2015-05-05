// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2008-2015 Bruno Lalande, Paris, France.
// Copyright (c) 2007-2015 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2009-2015 Mateusz Loskot, London, UK.

// This file was modified by Oracle on 2015.
// Modifications copyright (c) 2015, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef BOOST_GEOMETRY_ALGORITHMS_NOT_IMPLEMENTED_HPP
#define BOOST_GEOMETRY_ALGORITHMS_NOT_IMPLEMENTED_HPP

#include <boost/geometry/core/exception.hpp>

#include <boost/mpl/assert.hpp>
#include <boost/mpl/identity.hpp>
#include <boost/geometry/core/tags.hpp>


namespace boost { namespace geometry
{


namespace info
{
    struct UNRECOGNIZED_GEOMETRY_TYPE {};
    struct POINT {};
    struct LINESTRING {};
    struct POLYGON {};
    struct RING {};
    struct BOX {};
    struct SEGMENT {};
    struct MULTI_POINT {};
    struct MULTI_LINESTRING {};
    struct MULTI_POLYGON {};
    struct GEOMETRY_COLLECTION {};
    template <size_t D> struct DIMENSION {};
}


namespace nyi
{


struct not_implemented_tag {};

template
<
    typename Term1,
    typename Term2,
    typename Term3
>
struct not_implemented_error
{

#ifndef BOOST_GEOMETRY_IMPLEMENTATION_STATUS_BUILD
# define BOOST_GEOMETRY_IMPLEMENTATION_STATUS_BUILD false
#endif

    BOOST_MPL_ASSERT_MSG
        (
            BOOST_GEOMETRY_IMPLEMENTATION_STATUS_BUILD,
            THIS_OPERATION_IS_NOT_OR_NOT_YET_IMPLEMENTED,
            (
                types<Term1, Term2, Term3>
            )
        );
};

template <typename Tag>
struct tag_to_term
{
    typedef Tag type;
};

template <> struct tag_to_term<geometry_not_recognized_tag> { typedef info::UNRECOGNIZED_GEOMETRY_TYPE type; };
template <> struct tag_to_term<point_tag>                   { typedef info::POINT type; };
template <> struct tag_to_term<linestring_tag>              { typedef info::LINESTRING type; };
template <> struct tag_to_term<polygon_tag>                 { typedef info::POLYGON type; };
template <> struct tag_to_term<ring_tag>                    { typedef info::RING type; };
template <> struct tag_to_term<box_tag>                     { typedef info::BOX type; };
template <> struct tag_to_term<segment_tag>                 { typedef info::SEGMENT type; };
template <> struct tag_to_term<multi_point_tag>             { typedef info::MULTI_POINT type; };
template <> struct tag_to_term<multi_linestring_tag>        { typedef info::MULTI_LINESTRING type; };
template <> struct tag_to_term<multi_polygon_tag>           { typedef info::MULTI_POLYGON type; };
template <> struct tag_to_term<geometry_collection_tag>     { typedef info::GEOMETRY_COLLECTION type; };
template <int D> struct tag_to_term<boost::mpl::int_<D> >   { typedef info::DIMENSION<D> type; };


} // namespace nyi


template
<
    typename Term1 = void,
    typename Term2 = void,
    typename Term3 = void
>
struct not_implemented
    : nyi::not_implemented_tag,
      nyi::not_implemented_error
      <
          typename boost::mpl::identity
              <
                  typename nyi::tag_to_term<Term1>::type
              >::type,
          typename boost::mpl::identity
              <
                  typename nyi::tag_to_term<Term2>::type
              >::type,
          typename boost::mpl::identity
              <
                  typename nyi::tag_to_term<Term3>::type
              >::type
      >
{};


template
<
    typename Result = void,
    typename Term1 = void,
    typename Term2 = void,
    typename Term3 = void
>
struct lazy_not_implemented
    : nyi::not_implemented_tag
{
#ifndef BOOST_NO_CXX11_VARIADIC_TEMPLATES
    template <typename... T1>
    static inline Result apply(T1 const&...)
    {
        not_implemented<Term1, Term2, Term3>();
        return Result();
    }
#else
    static inline Result apply()
    {
        not_implemented<Term1, Term2, Term3>();
        return Result();
    }

    template <typename T1, typename T2>
    static inline Result apply(T1 const&, T2 const&)
    {
        not_implemented<Term1, Term2, Term3>();
        return Result();
    }

    template <typename T1, typename T2, typename T3>
    static inline Result apply(T1 const&, T2 const&, T3 const&)
    {
        not_implemented<Term1, Term2, Term3>();
        return Result();
    }

    template <typename T1, typename T2, typename T3, typename T4>
    static inline Result apply(T1 const&, T2 const&, T3 const&, T4 const&)
    {
        not_implemented<Term1, Term2, Term3>();
        return Result();
    }
#endif
};

template <typename Term1, typename Term2, typename Term3>
struct lazy_not_implemented<void, Term1, Term2, Term3>
    : nyi::not_implemented_tag
{
#ifndef BOOST_NO_CXX11_VARIADIC_TEMPLATES
    template <typename... T1>
    static inline void apply(T1 const&...)
    {
        not_implemented<Term1, Term2, Term3>();
    }
#else
    static inline void apply()
    {
        not_implemented<Term1, Term2, Term3>();
    }

    template <typename T1, typename T2>
    static inline void apply(T1 const&, T2 const&)
    {
        not_implemented<Term1, Term2, Term3>();
    }

    template <typename T1, typename T2, typename T3>
    static inline void apply(T1 const&, T2 const&, T3 const&)
    {
        not_implemented<Term1, Term2, Term3>();
    }

    template <typename T1, typename T2, typename T3, typename T4>
    static inline void apply(T1 const&, T2 const&, T3 const&, T4 const&)
    {
        not_implemented<Term1, Term2, Term3>();
    }
#endif
};


struct not_implemented_exception
    : boost::geometry::exception
{
    const char * what() const throw()
    {
        return "THIS_OPERATION_IS_NOT_OR_NOT_YET_IMPLEMENTED for passed input types";
    }    
};

}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_NOT_IMPLEMENTED_HPP
