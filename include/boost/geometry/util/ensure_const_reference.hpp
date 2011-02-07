// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_UTIL_ENSURE_CONST_REFERENCE_HPP
#define BOOST_GEOMETRY_UTIL_ENSURE_CONST_REFERENCE_HPP

#ifdef OBSOLETE

#include <boost/mpl/if.hpp>
#include <boost/type_traits.hpp>


namespace boost { namespace geometry
{


/*!
    \brief Meta-function to change a & into a const&, if it is a reference,
        otherwise it is the original
    \ingroup utility
    \note This traits class is completely independant from Boost.Geometry
        and might be a separate addition to Boost
    \note Used in a.o. interior_rings, exterior_ring
*/
template <typename T>
struct ensure_const_reference
{
    typedef typename mpl::if_
        <
            typename boost::is_reference<T>::type,
            typename boost::add_reference
                <
                    typename boost::add_const
                        <
                            typename boost::remove_reference<T>::type
                        >::type
                >::type,
            T
        >::type type;
};


}} // namespace boost::geometry

#endif

#endif // BOOST_GEOMETRY_UTIL_ENSURE_CONST_REFERENCE_HPP
