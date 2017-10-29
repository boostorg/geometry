// Boost.Geometry

// Copyright (c) 2017, Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_SRS_PROJECTIONS_PROJ4_HPP
#define BOOST_GEOMETRY_SRS_PROJECTIONS_PROJ4_HPP


#include <string>

#include <boost/tuple/tuple.hpp>


namespace boost { namespace geometry { namespace srs
{


struct dynamic {};


struct proj4
{
    explicit proj4(const char* s)
        : str(s)
    {}

    explicit proj4(std::string const& s)
        : str(s)
    {}

    std::string str;
};


template
<
    // null_type -> void?
    typename P0 = boost::tuples::null_type,
    typename P1 = boost::tuples::null_type,
    typename P2 = boost::tuples::null_type,
    typename P3 = boost::tuples::null_type,
    typename P4 = boost::tuples::null_type,
    typename P5 = boost::tuples::null_type,
    typename P6 = boost::tuples::null_type,
    typename P7 = boost::tuples::null_type,
    typename P8 = boost::tuples::null_type,
    typename P9 = boost::tuples::null_type
>
struct static_proj4
    : boost::tuple<P0, P1, P2, P3, P4, P5, P6, P7, P8, P9>
{
    typedef boost::tuple<P0, P1, P2, P3, P4, P5, P6, P7, P8, P9> tuple_type;

    static_proj4()
    {}

    explicit static_proj4(const char* s)
        : str(s)
    {}

    explicit static_proj4(std::string const& s)
        : str(s)
    {}

    explicit static_proj4(P0 const& p0)
        : tuple_type(p0)
    {}

    static_proj4(P0 const& p0, const char* s)
        : tuple_type(p0)
        , str(s)
    {}

    static_proj4(P0 const& p0, std::string const& s)
        : tuple_type(p0)
        , str(s)
    {}

    static_proj4(P0 const& p0, P1 const& p1)
        : tuple_type(p0, p1)
    {}

    static_proj4(P0 const& p0, P1 const& p1, const char* s)
        : tuple_type(p0, p1)
        , str(s)
    {}

    static_proj4(P0 const& p0, P1 const& p1, std::string const& s)
        : tuple_type(p0, p1)
        , str(s)
    {}

    static_proj4(P0 const& p0, P1 const& p1, P2 const& p2)
        : tuple_type(p0, p1, p2)
    {}

    static_proj4(P0 const& p0, P1 const& p1, P2 const& p2, const char* s)
        : tuple_type(p0, p1, p2)
        , str(s)
    {}

    static_proj4(P0 const& p0, P1 const& p1, P2 const& p2, std::string const& s)
        : tuple_type(p0, p1, p2)
        , str(s)
    {}

    static_proj4(P0 const& p0, P1 const& p1, P2 const& p2, P3 const& p3)
        : tuple_type(p0, p1, p2, p3)
    {}

    static_proj4(P0 const& p0, P1 const& p1, P2 const& p2, P3 const& p3, const char* s)
        : tuple_type(p0, p1, p2, p3)
        , str(s)
    {}

    static_proj4(P0 const& p0, P1 const& p1, P2 const& p2, P3 const& p3, std::string const& s)
        : tuple_type(p0, p1, p2, p3)
        , str(s)
    {}

    static_proj4(P0 const& p0, P1 const& p1, P2 const& p2, P3 const& p3, P4 const& p4)
        : tuple_type(p0, p1, p2, p3, p4)
    {}

    static_proj4(P0 const& p0, P1 const& p1, P2 const& p2, P3 const& p3, P4 const& p4, const char* s)
        : tuple_type(p0, p1, p2, p3, p4)
        , str(s)
    {}

    static_proj4(P0 const& p0, P1 const& p1, P2 const& p2, P3 const& p3, P4 const& p4, std::string const& s)
        : tuple_type(p0, p1, p2, p3, p4)
        , str(s)
    {}

    static_proj4(P0 const& p0, P1 const& p1, P2 const& p2, P3 const& p3, P4 const& p4, P5 const& p5)
        : tuple_type(p0, p1, p2, p3, p4, p5)
    {}

    static_proj4(P0 const& p0, P1 const& p1, P2 const& p2, P3 const& p3, P4 const& p4, P5 const& p5, const char* s)
        : tuple_type(p0, p1, p2, p3, p4, p5)
        , str(s)
    {}

    static_proj4(P0 const& p0, P1 const& p1, P2 const& p2, P3 const& p3, P4 const& p4, P5 const& p5, std::string const& s)
        : tuple_type(p0, p1, p2, p3, p4, p5)
        , str(s)
    {}

    static_proj4(P0 const& p0, P1 const& p1, P2 const& p2, P3 const& p3, P4 const& p4, P5 const& p5, P6 const& p6)
        : tuple_type(p0, p1, p2, p3, p4, p5, p6)
    {}

    static_proj4(P0 const& p0, P1 const& p1, P2 const& p2, P3 const& p3, P4 const& p4, P5 const& p5, P6 const& p6, const char* s)
        : tuple_type(p0, p1, p2, p3, p4, p5, p6)
        , str(s)
    {}

    static_proj4(P0 const& p0, P1 const& p1, P2 const& p2, P3 const& p3, P4 const& p4, P5 const& p5, P6 const& p6, std::string const& s)
        : tuple_type(p0, p1, p2, p3, p4, p5, p6)
        , str(s)
    {}

    static_proj4(P0 const& p0, P1 const& p1, P2 const& p2, P3 const& p3, P4 const& p4, P5 const& p5, P6 const& p6, P7 const& p7)
        : tuple_type(p0, p1, p2, p3, p4, p5, p6, p7)
    {}

    static_proj4(P0 const& p0, P1 const& p1, P2 const& p2, P3 const& p3, P4 const& p4, P5 const& p5, P6 const& p6, P7 const& p7, const char* s)
        : tuple_type(p0, p1, p2, p3, p4, p5, p6, p7)
        , str(s)
    {}

    static_proj4(P0 const& p0, P1 const& p1, P2 const& p2, P3 const& p3, P4 const& p4, P5 const& p5, P6 const& p6, P7 const& p7, std::string const& s)
        : tuple_type(p0, p1, p2, p3, p4, p5, p6, p7)
        , str(s)
    {}

    static_proj4(P0 const& p0, P1 const& p1, P2 const& p2, P3 const& p3, P4 const& p4, P5 const& p5, P6 const& p6, P7 const& p7, P8 const& p8)
        : tuple_type(p0, p1, p2, p3, p4, p5, p6, p7, p8)
    {}

    static_proj4(P0 const& p0, P1 const& p1, P2 const& p2, P3 const& p3, P4 const& p4, P5 const& p5, P6 const& p6, P7 const& p7, P8 const& p8, const char* s)
        : tuple_type(p0, p1, p2, p3, p4, p5, p6, p7, p8)
        , str(s)
    {}

    static_proj4(P0 const& p0, P1 const& p1, P2 const& p2, P3 const& p3, P4 const& p4, P5 const& p5, P6 const& p6, P7 const& p7, P8 const& p8, std::string const& s)
        : tuple_type(p0, p1, p2, p3, p4, p5, p6, p7, p8)
        , str(s)
    {}

    static_proj4(P0 const& p0, P1 const& p1, P2 const& p2, P3 const& p3, P4 const& p4, P5 const& p5, P6 const& p6, P7 const& p7, P8 const& p8, P9 const& p9)
        : tuple_type(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9)
    {}

    static_proj4(P0 const& p0, P1 const& p1, P2 const& p2, P3 const& p3, P4 const& p4, P5 const& p5, P6 const& p6, P7 const& p7, P8 const& p8, P9 const& p9, const char* s)
        : tuple_type(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9)
        , str(s)
    {}

    static_proj4(P0 const& p0, P1 const& p1, P2 const& p2, P3 const& p3, P4 const& p4, P5 const& p5, P6 const& p6, P7 const& p7, P8 const& p8, P9 const& p9, std::string const& s)
        : tuple_type(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9)
        , str(s)
    {}
    
    std::string str;
};

#define BOOST_GEOMETRY_PROJECTIONS_DETAIL_TYPENAME_PX \
typename P0, typename P1, typename P2, typename P3, typename P4, \
typename P5, typename P6, typename P7, typename P8, typename P9

#define BOOST_GEOMETRY_PROJECTIONS_DETAIL_PX \
P0, P1, P2, P3, P4, P5, P6, P7, P8, P9


}}} // namespace boost::geometry::srs

#ifndef DOXYGEN_NO_DETAIL
namespace boost { namespace geometry { namespace projections { namespace detail
{

template<typename R> struct function_argument_type;
template<typename R, typename A> struct function_argument_type<R(A)> { typedef A type; };

}}}} // namespace boost::geometry::projections::detail
#endif // DOXYGEN_NO_DETAIL

#endif // BOOST_GEOMETRY_SRS_PROJECTIONS_PROJ4_HPP
