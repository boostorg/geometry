// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_CALCULATE_SUM_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_CALCULATE_SUM_HPP


#include <boost/range.hpp>
#include <boost/typeof/typeof.hpp>

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail
{


template
<
    typename ReturnType,
    typename Polygon,
    typename Strategy,
    typename Policy
>
struct calculate_polygon_sum
{
    static inline ReturnType apply(Polygon const& poly, Strategy const& strategy)
    {
        ReturnType sum = Policy::apply(exterior_ring(poly), strategy);

        typename interior_return_type<Polygon const>::type rings = interior_rings(poly);
        for (BOOST_AUTO(it, boost::begin(rings)); it != boost::end(rings); ++it)
        {
            sum += Policy::apply(*it, strategy);
        }
        return sum;
    }
};


} // namespace detail
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_CALCULATE_SUM_HPP
