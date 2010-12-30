// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_LENGTH_RESULT_HPP
#define BOOST_GEOMETRY_STRATEGIES_LENGTH_RESULT_HPP


#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/util/select_most_precise.hpp>


namespace boost { namespace geometry
{

/*!
    \brief Meta-function defining return type of length function
    \ingroup length
    \note Length of a line of integer coordinates can be double.
        So we take at least a double. If Big Number types are used,
        we take that type.

 */
template <typename Geometry>
struct length_result
{
    typedef typename select_most_precise
        <
            typename coordinate_type<Geometry>::type,
            long double
        >::type type;
};

}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_STRATEGIES_LENGTH_RESULT_HPP
