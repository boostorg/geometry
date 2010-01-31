// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_CALCULATE_NULL_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_CALCULATE_NULL_HPP

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail
{

template<typename ReturnType, typename Geometry, typename Strategy>
struct calculate_null
{
    static inline ReturnType apply(Geometry const& , Strategy const&)
    {
        return ReturnType();
    }
};

} // namespace detail
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_CALCULATE_NULL_HPP
