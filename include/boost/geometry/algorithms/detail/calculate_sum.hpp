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
class calculate_polygon_sum
{
    template <typename Rings>
    static inline ReturnType sum_interior_rings(Rings const& rings, Strategy const& strategy)
    {
        ReturnType sum = ReturnType();
        for (BOOST_AUTO(it, boost::begin(rings)); it != boost::end(rings); ++it)
        {
            sum += Policy::apply(*it, strategy);
        }
        return sum;
    }

public :
    static inline ReturnType apply(Polygon const& poly, Strategy const& strategy)
    {
        return Policy::apply(exterior_ring(poly), strategy)
            + sum_interior_rings(bg::interior_rings(poly), strategy)
            ;
    }
};


} // namespace detail
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_CALCULATE_SUM_HPP
