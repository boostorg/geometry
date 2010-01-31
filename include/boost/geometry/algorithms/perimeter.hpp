// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_PERIMETER_HPP
#define BOOST_GEOMETRY_ALGORITHMS_PERIMETER_HPP


#include <boost/geometry/core/cs.hpp>

#include <boost/geometry/geometries/concepts/check.hpp>

#include <boost/geometry/strategies/length_result.hpp>

#include <boost/geometry/algorithms/length.hpp>
#include <boost/geometry/algorithms/detail/calculate_null.hpp>
#include <boost/geometry/algorithms/detail/calculate_sum.hpp>


/*!
\defgroup perimeter perimeter: calculate perimeter of a geometry
\par Geometries:
- \b polygon
- \b box
- \b linear_ring
- \b multi_polygon
*/

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

// Default perimeter is 0.0, specializations implement calculated values
template <typename Tag, typename Geometry, typename Strategy>
struct perimeter : detail::calculate_null
    <
        typename length_result<Geometry>::type,
        Geometry,
        Strategy
    >
{};

template <typename Geometry, typename Strategy>
struct perimeter<ring_tag, Geometry, Strategy>
    : detail::length::range_length<Geometry, Strategy>
{};

template <typename Polygon, typename Strategy>
struct perimeter<polygon_tag, Polygon, Strategy>
    : detail::calculate_polygon_sum
        <
            typename length_result<Polygon>::type,
            Polygon,
            Strategy,
            detail::length::range_length
                <
                    typename ring_type<Polygon>::type,
                    Strategy
                >
        >
{};


// box,n-sphere: to be implemented

} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


/*!
    \brief Calculate perimeter of a geometry
    \ingroup perimeter
    \details The function perimeter returns the perimeter of a geometry,
        using the default distance-calculation-strategy
    \param geometry the geometry, be it a geometry::ring, vector, iterator pair,
        or any other boost compatible range
    \return the perimeter
 */
template<typename Geometry>
inline typename length_result<Geometry>::type perimeter(
        Geometry const& geometry)
{
    concept::check<const Geometry>();

    typedef typename point_type<Geometry>::type point_type;
    typedef typename cs_tag<point_type>::type cs_tag;
    typedef typename strategy_distance
        <
            cs_tag,
            cs_tag,
            point_type,
            point_type
        >::type strategy_type;

    return dispatch::perimeter
        <
            typename tag<Geometry>::type,
            Geometry,
            strategy_type
        >::apply(geometry, strategy_type());
}

/*!
    \brief Calculate perimeter of a geometry
    \ingroup perimeter
    \details The function perimeter returns the perimeter of a geometry,
        using specified strategy
    \param geometry the geometry, be it a geometry::ring, vector, iterator pair,
        or any other boost compatible range
    \param strategy strategy to be used for distance calculations.
    \return the perimeter
 */
template<typename Geometry, typename Strategy>
inline typename length_result<Geometry>::type perimeter(
        Geometry const& geometry, Strategy const& strategy)
{
    concept::check<const Geometry>();

    return dispatch::perimeter
        <
            typename tag<Geometry>::type,
            Geometry,
            Strategy
        >::apply(geometry, strategy);
}

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_PERIMETER_HPP

