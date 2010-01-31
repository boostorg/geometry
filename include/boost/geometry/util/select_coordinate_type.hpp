// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_UTIL_SELECT_COORDINATE_TYPE_HPP
#define BOOST_GEOMETRY_UTIL_SELECT_COORDINATE_TYPE_HPP


#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/util/select_most_precise.hpp>


/*!
\defgroup utility utility: utility meta-functions and functions
\details The utilities, mostly in folder util, contain several headerfiles
not fitting in one of the other folders.

The following meta-functions are general and do not relate to GGL:
- add_const_if_c
- range_iterator_const_if_c
- select_most_precise

They might fit into boost as a separate trait or utility, or there might
be a Boost equivalent which is yet unnoticed by the authors.


*/

namespace boost { namespace geometry
{


/*!
    \brief Meta-function selecting the most precise coordinate type
        of two geometries
    \ingroup utility
 */
template <typename T1, typename T2>
struct select_coordinate_type
{
    typedef typename select_most_precise
        <
            typename coordinate_type<T1>::type,
            typename coordinate_type<T2>::type
        >::type type;
};


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_UTIL_SELECT_COORDINATE_TYPE_HPP
