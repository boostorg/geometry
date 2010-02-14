// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DISJOINT_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DISJOINT_HPP


/*!
\defgroup disjoint disjoint: detect if geometries are not spatially related
\details disjoint means spatially disjoint, there is no overlap of interiors
    and boundaries, the intersection of interiors or boundaries is empty.

\par Geometries:
- \b point + \b point (= ! equals)
- \b point + \b box (= not within or on border)
- \b box + \b box
- \b ring + \b box
- \b polygon + \b box
- \b polygon + \b ring
- \b polygon + \b polygon

*/

#include <deque>


#include <boost/mpl/if.hpp>
#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>
#include <boost/static_assert.hpp>

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/coordinate_dimension.hpp>
#include <boost/geometry/core/is_multi.hpp>
#include <boost/geometry/core/reverse_dispatch.hpp>

#include <boost/geometry/algorithms/detail/disjoint.hpp>
#include <boost/geometry/algorithms/detail/point_on_border.hpp>
#include <boost/geometry/algorithms/overlay/get_turns.hpp>
#include <boost/geometry/algorithms/within.hpp>

#include <boost/geometry/geometries/concepts/check.hpp>

#include <boost/geometry/util/math.hpp>



namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace disjoint 
{


struct disjoint_interrupt_policy
{
    static bool const enabled = true;
    bool has_intersections;

    inline disjoint_interrupt_policy()
        : has_intersections(false)
    {}

    template <typename Range>
    inline bool apply(Range const& range)
    {
        // If there is any IP in the range, it is NOT disjoint
        if (boost::size(range) > 0)
        {
            has_intersections = true;
            return true;
        }
        return false;
    }
};


template <typename Geometry1, typename Geometry2>
struct general
{
    static inline bool apply(Geometry1 const& geometry1, Geometry2 const& geometry2)
    {
        typedef typename geometry::point_type<Geometry1>::type point_type;

        typedef overlay::turn_info<point_type> turn_info;
        std::deque<turn_info> turns;

        // Get (and stop on) any intersection
        disjoint_interrupt_policy policy;
        geometry::get_turns
            <
                overlay::assign_null_policy
            >(geometry1, geometry2, turns, policy);
        if (policy.has_intersections)
        {
            return false;
        }

        // If there is no intersection of segments, they might located
        // inside each other
        point_type p1;
        geometry::point_on_border(p1, geometry1);
        if (geometry::within(p1, geometry2))
        {
            return false;
        }

        typename geometry::point_type<Geometry1>::type p2;
        geometry::point_on_border(p2, geometry2);
        if (geometry::within(p2, geometry1))
        {
            return false;
        }

        return true;
    }
};


}} // namespace detail::disjoint
#endif // DOXYGEN_NO_DETAIL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


template
<
    typename GeometryTag1, typename GeometryTag2,
    typename Geometry1, typename Geometry2,
    bool IsMulti1, bool IsMulti2,
    std::size_t DimensionCount
>
struct disjoint
    : detail::disjoint::general<Geometry1, Geometry2>
{};


template <typename Point1, typename Point2, std::size_t DimensionCount>
struct disjoint<point_tag, point_tag, Point1, Point2, false, false, DimensionCount>
    : detail::disjoint::point_point<Point1, Point2, 0, DimensionCount>
{};


template <typename Box1, typename Box2, std::size_t DimensionCount>
struct disjoint<box_tag, box_tag, Box1, Box2, false, false, DimensionCount>
    : detail::disjoint::box_box<Box1, Box2, 0, DimensionCount>
{};


template <typename Point, typename Box, std::size_t DimensionCount>
struct disjoint<point_tag, box_tag, Point, Box, false, false, DimensionCount>
    : detail::disjoint::point_box<Point, Box, 0, DimensionCount>
{};


template
<
    typename GeometryTag1, typename GeometryTag2,
    typename G1, typename G2,
    bool IsMulti1, bool IsMulti2,
    std::size_t DimensionCount
>
struct disjoint_reversed
{
    static inline bool apply(G1 const& g1, G2 const& g2)
    {
        return disjoint
            <
                GeometryTag2, GeometryTag1,
                G2, G1,
                IsMulti2, IsMulti1,
                DimensionCount
            >::apply(g2, g1);
    }
};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH



/*!
    \brief Calculate if two geometries are disjoint
    \ingroup disjoint
    \tparam Geometry1 first geometry type
    \tparam Geometry2 second geometry type
    \param geometry1 first geometry
    \param geometry2 second geometry
    \return true if disjoint, else false
 */
template <typename Geometry1, typename Geometry2>
inline bool disjoint(const Geometry1& geometry1,
            const Geometry2& geometry2)
{
    concept::check_concepts_and_equal_dimensions
        <
            const Geometry1,
            const Geometry2
        >();

    typedef typename boost::remove_const<Geometry1>::type ncg1_type;
    typedef typename boost::remove_const<Geometry2>::type ncg2_type;

    return boost::mpl::if_c
        <
            reverse_dispatch<Geometry1, Geometry2>::type::value,
            dispatch::disjoint_reversed
            <
                typename tag<ncg1_type>::type,
                typename tag<ncg2_type>::type,
                ncg1_type,
                ncg2_type,
                is_multi<ncg1_type>::type::value,
                is_multi<ncg2_type>::type::value,
                dimension<ncg1_type>::type::value
            >,
            dispatch::disjoint
            <
                typename tag<ncg1_type>::type,
                typename tag<ncg2_type>::type,
                ncg1_type,
                ncg2_type,
                is_multi<ncg1_type>::type::value,
                is_multi<ncg2_type>::type::value,
                dimension<ncg1_type>::type::value
            >
        >::type::apply(geometry1, geometry2);
}


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DISJOINT_HPP
