// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_CENTROID_HPP
#define BOOST_GEOMETRY_STRATEGIES_CENTROID_HPP


#include <boost/geometry/strategies/tags.hpp>


namespace boost { namespace geometry
{


/*!
    \brief Traits class binding a centroid calculation strategy to a coordinate system
    \ingroup centroid
    \tparam CsTag tag of coordinate system, for specialization
    \tparam GeometryTag tag of geometry, for specialization
    \tparam Dimension dimension of geometry, for specialization
    \tparam Point point-type
    \tparam Geometry
*/
template
<
    typename CsTag,
    typename GeometryTag,
    std::size_t Dimension,
    typename Point,
    typename Geometry
>
struct strategy_centroid
{
    typedef strategy::not_implemented type;
};


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_CENTROID_HPP
