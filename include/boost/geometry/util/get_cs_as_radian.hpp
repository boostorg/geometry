// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Bruno Lalande 2008, 2009
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef BOOST_GEOMETRY_UTIL_GET_CS_AS_RADIAN_HPP
#define BOOST_GEOMETRY_UTIL_GET_CS_AS_RADIAN_HPP

// obsolete? It is not used anymore (get_as_radian is usually OK)

#include <boost/geometry/core/cs.hpp>

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail
{

    /*!
        \brief Small meta-function defining the specified coordinate system,
        but then in radian units
    */
    template <typename CoordinateSystem>
    struct get_cs_as_radian {};

    template <typename Units>
    struct get_cs_as_radian<cs::geographic<Units> >
    {
        typedef cs::geographic<radian> type;
    };

    template <typename Units>
    struct get_cs_as_radian<cs::spherical<Units> >
    {
        typedef cs::spherical<radian> type;
    };

} // namespace detail
#endif


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_UTIL_GET_CS_AS_RADIAN_HPP
