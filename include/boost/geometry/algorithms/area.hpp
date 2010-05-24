// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_AREA_HPP
#define BOOST_GEOMETRY_ALGORITHMS_AREA_HPP

#include <boost/concept_check.hpp>
#include <boost/mpl/if.hpp>
#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>


#include <boost/geometry/core/closure.hpp>
#include <boost/geometry/core/exterior_ring.hpp>
#include <boost/geometry/core/interior_rings.hpp>
#include <boost/geometry/core/point_order.hpp>
#include <boost/geometry/core/ring_type.hpp>

#include <boost/geometry/geometries/concepts/check.hpp>

#include <boost/geometry/algorithms/detail/calculate_null.hpp>
#include <boost/geometry/algorithms/detail/calculate_sum.hpp>

#include <boost/geometry/strategies/area.hpp>
#include <boost/geometry/strategies/area_result.hpp>

#include <boost/geometry/strategies/concepts/area_concept.hpp>

#include <boost/geometry/util/closure_as_bool.hpp>
#include <boost/geometry/util/math.hpp>
#include <boost/geometry/util/order_as_direction.hpp>
#include <boost/geometry/util/closeable_view.hpp>
#include <boost/geometry/util/reversible_view.hpp>


/*!
\defgroup area area: calculate area of a geometry

\par Performance
2776 * 1000 area calculations are done in 0.11 seconds
(http://trac.osgeo.org/ggl/wiki/Performance#Area1)

\par Coordinate systems and strategies
Area calculation can be done in Cartesian and in spherical/geographic
coordinate systems.

\par Geometries
The area algorithm calculates the surface area of all geometries
having a surface: box, polygon, multi_polygon. The units are the square of
the units used for the points defining the surface. If the polygon is defined
in meters, the area is in square meters.

\par Example:
Example showing area calculation of polygons built
\dontinclude doxygen_1.cpp
\skip example_area_polygon()
\line {
\until }

*/
namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace area
{

template<typename Box, typename Strategy>
struct box_area
{
    typedef typename coordinate_type<Box>::type return_type;

    static inline return_type apply(Box const& box, Strategy const&)
    {
        // Currently only works for 2D Cartesian boxes
        assert_dimension<Box, 2>();

        return_type const dx = get<max_corner, 0>(box)
                - get<min_corner, 0>(box);
        return_type const dy = get<max_corner, 1>(box)
                - get<min_corner, 1>(box);

        return dx * dy;
    }
};


template
<
    typename Ring, 
    iterate_direction Direction, 
    closure_selector Closure,
    typename Strategy
>
struct ring_area
{
    BOOST_CONCEPT_ASSERT( (geometry::concept::AreaStrategy<Strategy>) );

    typedef typename Strategy::return_type type;

    static inline type apply(Ring const& ring, Strategy const& strategy)
    {
        assert_dimension<Ring, 2>();

        // Ignore warning (because using static method sometimes) on strategy
        boost::ignore_unused_variable_warning(strategy);

        // A closed linear_ring has at least four points,
        // if not, there is no (zero) area
        if (boost::size(ring) < 4)
        {
            return type();
        }


        typedef reversible_view<Ring const, Direction> rview_type;
        typedef closeable_view
            <
                rview_type const,
                Closure == closed
            > view_type;
        typedef typename boost::range_iterator<view_type const>::type iterator_type;

        rview_type rview(ring);
        view_type view(rview);
        typename Strategy::state_type state;
        iterator_type it = boost::begin(view);
        iterator_type end = boost::end(view);

        for (iterator_type previous = it++;
            it != end;
            ++previous, ++it)
        {
            strategy.apply(*previous, *it, state);
        }

        return strategy.result(state);
    }
};


}} // namespace detail::area


#endif // DOXYGEN_NO_DETAIL



#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template
<
    typename Tag,
    typename Geometry,
    order_selector Order,
    closure_selector Closure,
    typename Strategy
>
struct area
    : detail::calculate_null
        <
            typename Strategy::return_type,
            Geometry,
            Strategy
        > {};


template <typename Geometry, order_selector Order, closure_selector Closure, typename Strategy>
struct area<box_tag, Geometry, Order, Closure, Strategy>
    : detail::area::box_area<Geometry, Strategy>
{};


template <typename Geometry, order_selector Order, closure_selector Closure, typename Strategy>
struct area<ring_tag, Geometry, Order, Closure, Strategy>
    : detail::area::ring_area
        <
            Geometry, 
            order_as_direction<Order>::value, 
            Closure, //closure_as_bool<Closure>::value,
            Strategy
        >
{};


template <typename Polygon, order_selector Order, closure_selector Closure, typename Strategy>
struct area<polygon_tag, Polygon, Order, Closure, Strategy>
    : detail::calculate_polygon_sum
        <
            typename Strategy::return_type,
            Polygon,
            Strategy,
            detail::area::ring_area
                <
                    typename ring_type<Polygon>::type,
                    order_as_direction<Order>::value,
                    Closure, //closure_as_bool<Closure>::value,
                    Strategy
                >
        >
{};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH



/*!
    \brief Calculate area of a geometry
    \ingroup area
    \details The function area returns the area of a polygon, ring, box
    using the default area-calculation strategy. Strategies are
    provided for cartesian and spherical coordinate systems
    The geometries should correct, polygons should be closed
    and according to the specified orientation (clockwise/counter clockwise)
    \param geometry a geometry
    \return the area
 */
template <typename Geometry>
inline typename area_result<Geometry>::type area(Geometry const& geometry)
{
    concept::check<Geometry const>();

    typedef typename area_result<Geometry>::strategy_type strategy_type;

    return dispatch::area
        <
            typename tag<Geometry>::type,
            Geometry,
            geometry::point_order<Geometry>::value,
            geometry::closure<Geometry>::value,
            strategy_type
        >::apply(geometry, strategy_type());
}

/*!
    \brief Calculate area of a geometry using a specified strategy
    \ingroup area
    \details This version of area calculation takes a strategy
    \param geometry a geometry
    \param strategy the strategy to calculate area.
        Especially for spherical areas there are some approaches.
    \return the area
 */
template <typename Geometry, typename Strategy>
inline typename Strategy::return_type area(
        Geometry const& geometry, Strategy const& strategy)
{
    concept::check<Geometry const>();

    return dispatch::area
        <
            typename tag<Geometry>::type,
            Geometry,
            geometry::point_order<Geometry>::value,
            geometry::closure<Geometry>::value,
            Strategy
        >::apply(geometry, strategy);
}


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_AREA_HPP
