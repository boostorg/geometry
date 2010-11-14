// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_WITHIN_HPP
#define BOOST_GEOMETRY_STRATEGIES_WITHIN_HPP

#include <boost/mpl/assert.hpp>

#include <boost/geometry/strategies/tags.hpp>

namespace boost { namespace geometry
{

namespace strategy { namespace within
{


namespace services
{

/*!
    \brief Traits class binding a within determination strategy to a coordinate system
    \ingroup within
    \tparam CsTagContained tag of coordinate system of point-type
    \tparam CsTagContained tag of coordinate system of segment-type
    \tparam Point point-type of input points
    \tparam PointContaining point-type of input segment-points
*/
template
<
    typename TagContained,
    typename TagContaining,
    typename CsTagContained,
    typename CsTagContaining,
    typename Point,
    typename PointContaining
>
struct default_strategy
{
    // If we would assert here, we would have to implement
    // default strategies for all combinations, all CS, etc.
    // This explosion is not convenient.
    // Another option is tag inheritance / grouping (so point-in-polygon will apply for point-in-ring, point-in-polygon, point-in-multi-polygon but not for point-in-box...)
    // TODO: decide about this.

    BOOST_MPL_ASSERT_MSG
        (
            false, NOT_IMPLEMENTED_FOR_THIS_TYPES
            , (types<Point, PointContaining>)
        );

};


} // namespace services


}} // namespace strategy::within


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_STRATEGIES_WITHIN_HPP

