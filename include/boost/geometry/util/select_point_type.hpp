// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2020, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_UTIL_SELECT_POINT_TYPE_HPP
#define BOOST_GEOMETRY_UTIL_SELECT_POINT_TYPE_HPP


#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/util/select_most_precise.hpp>


namespace boost { namespace geometry
{


/*!
    \brief Meta-function selecting the most precise point type
        of two geometries
    \ingroup utility
 */
template <typename T1, typename T2>
struct select_point_type
{
    typedef typename geometry::select_most_precise
                    <
                        typename coordinate_type<T1>::type,
                        typename coordinate_type<T2>::type
                    >::type most_precise_type;

    typedef typename boost::mpl::if_c
            <
                boost::is_same
                <
                    most_precise_type,
                    typename coordinate_type<T1>::type
                >::value,
                typename point_type<T1>::type,
                typename point_type<T2>::type
            >::type type;
};

}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_UTIL_SELECT_POINT_TYPE_HPP
