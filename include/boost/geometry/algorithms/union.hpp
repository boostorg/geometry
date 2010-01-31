// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_UNION_HPP
#define BOOST_GEOMETRY_ALGORITHMS_UNION_HPP


#include <boost/mpl/if.hpp>

#include <boost/geometry/core/interior_rings.hpp>
#include <boost/geometry/core/reverse_dispatch.hpp>
#include <boost/geometry/geometries/concepts/check.hpp>
#include <boost/geometry/algorithms/overlay/assemble.hpp>
#include <boost/geometry/algorithms/within.hpp>



/*!
\defgroup union union: calculate new geometry containing geometries A or B
\details The union of two geometries A and B is the geometry containing
    all points belong to either of A or B, but no other elements.
\par Source description:
- OGC: Returns a geometric object that represents the Point set union of
    this geometric object with another Geometry.
\see http://en.wikipedia.org/wiki/Union_(set_theory)
\note A union of two rings can result in a polygon having a hole

\par Geometries:
- \b polygon + \b box -> \b polygon(s)
\image html svg_union_polygon_box.png
- \b ring + \b box -> \b polygon(s)
\image html svg_union_ring_box.png
- \b ring + \b ring -> \b polygon(s)
\image html svg_union_ring_ring.png
- \b polygon + \b ring -> \b polygon(s)
\image html svg_union_polygon_ring.png
- combinations above -> \b ring(s).
    <i>If the output is an ouput iterator of rings, holes are omitted</i>

*/

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template
<
    typename Tag1, typename Tag2, typename Tag3,
    typename G1, typename G2,
    typename OutputIterator,
    typename GeometryOut,
    typename Strategy
>
struct union_inserter
    : detail::overlay::overlay_and_assemble
        <G1, G2, OutputIterator, GeometryOut, 1, Strategy>
{
};


template
<
    typename GeometryTag1, typename GeometryTag2, typename GeometryTag3,
    typename Geometry1, typename Geometry2,
    typename OutputIterator, typename GeometryOut,
    typename Strategy
>
struct union_inserter_reversed
{
    static inline OutputIterator apply(Geometry1 const& g1,
            Geometry2 const& g2, OutputIterator out,
            Strategy const& strategy)
    {
        return union_inserter
            <
                GeometryTag2, GeometryTag1, GeometryTag3,
                Geometry2, Geometry1,
                OutputIterator, GeometryOut,
                Strategy
            >::apply(g2, g1, out, strategy);
    }
};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


/*!
    \brief Combines two geometries which each other
    \ingroup union
    \tparam GeometryOut output geometry type, must be specified
    \tparam Geometry1 first geometry type
    \tparam Geometry2 second geometry type
    \tparam OutputIterator output iterator
    \tparam Strategy compound strategy for intersection
    \param geometry1 first geometry
    \param geometry2 second geometry
    \param out the output iterator, outputting polygons
    \param strategy the strategy
    \return the output iterator
*/
template
<
    typename GeometryOut,
    typename Geometry1,
    typename Geometry2,
    typename OutputIterator,
    typename Strategy
>
inline OutputIterator union_inserter(Geometry1 const& geometry1,
            Geometry2 const& geometry2,
            OutputIterator out,
            Strategy const& strategy)
{
    concept::check<const Geometry1>();
    concept::check<const Geometry2>();

    return boost::mpl::if_c
        <
            reverse_dispatch<Geometry1, Geometry2>::type::value,
            dispatch::union_inserter_reversed
            <
                typename tag<Geometry1>::type,
                typename tag<Geometry2>::type,
                typename tag<GeometryOut>::type,
                Geometry1,
                Geometry2,
                OutputIterator, GeometryOut,
                Strategy
            >,
            dispatch::union_inserter
            <
                typename tag<Geometry1>::type,
                typename tag<Geometry2>::type,
                typename tag<GeometryOut>::type,
                Geometry1,
                Geometry2,
                OutputIterator, GeometryOut,
                Strategy
            >
        >::type::apply(geometry1, geometry2, out, strategy);
}

/*!
    \brief Combines two geometries which each other
    \ingroup union
    \tparam GeometryOut output geometry type, must be specified
    \tparam Geometry1 first geometry type
    \tparam Geometry2 second geometry type
    \tparam OutputIterator output iterator
    \param geometry1 first geometry
    \param geometry2 second geometry
    \param out the output iterator, outputting polygons
    \return the output iterator
*/
template
<
    typename GeometryOut,
    typename Geometry1,
    typename Geometry2,
    typename OutputIterator
>
inline OutputIterator union_inserter(Geometry1 const& geometry1,
            Geometry2 const& geometry2,
            OutputIterator out)
{
    concept::check<const Geometry1>();
    concept::check<const Geometry2>();

    typedef typename geometry::point_type<GeometryOut>::type point_type;
    typedef detail::intersection::intersection_point<point_type> ip_type;

    typedef strategy_intersection
        <
            typename cs_tag<point_type>::type,
            Geometry1,
            Geometry2,
            ip_type
        > strategy;

    return union_inserter<GeometryOut>(geometry1, geometry2, out, strategy());
}


}} // namespace boost::geometry


#endif //GGL_ALGORITHMS_UNION_HPP
