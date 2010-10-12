// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_INTERSECTION_HPP
#define BOOST_GEOMETRY_ALGORITHMS_INTERSECTION_HPP


#include <cstddef>

#include <boost/mpl/if.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/range/metafunctions.hpp>


#include <boost/geometry/core/is_areal.hpp>
#include <boost/geometry/core/point_order.hpp>
#include <boost/geometry/core/reverse_dispatch.hpp>
#include <boost/geometry/geometries/concepts/check.hpp>
#include <boost/geometry/algorithms/detail/overlay/clip_linestring.hpp>
#include <boost/geometry/algorithms/detail/overlay/get_intersection_points.hpp>
#include <boost/geometry/algorithms/detail/overlay/overlay.hpp>
#include <boost/geometry/ranges/segment_range.hpp>


namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace intersection
{

template
<
    typename Segment1, typename Segment2,
    typename OutputIterator, typename PointOut,
    typename Strategy
>
struct intersection_segment_segment_point
{
    static inline OutputIterator apply(Segment1 const& segment1,
            Segment2 const& segment2, OutputIterator out,
            Strategy const& strategy)
    {
        typedef typename point_type<PointOut>::type point_type;

        // Get the intersection point (or two points)
        segment_intersection_points<point_type> is
            = strategy::intersection::relate_cartesian_segments
            <
                policies::relate::segments_intersection_points
                    <
                        Segment1,
                        Segment2,
                        segment_intersection_points<point_type>
                    >
            >::apply(segment1, segment2);

        for (std::size_t i = 0; i < is.count; i++)
        {
            PointOut p;
            geometry::copy_coordinates(is.intersections[i], p);
            *out++ = p;
        }
        return out;
    }
};

template
<
    typename Linestring1, typename Linestring2,
    typename OutputIterator, typename PointOut,
    typename Strategy
>
struct intersection_linestring_linestring_point
{
    static inline OutputIterator apply(Linestring1 const& linestring1,
            Linestring2 const& linestring2, OutputIterator out,
            Strategy const& strategy)
    {
        typedef typename point_type<PointOut>::type point_type;

        typedef detail::overlay::turn_info<point_type> turn_info;
        std::deque<turn_info> turns;

        geometry::get_intersection_points(linestring1, linestring2, turns);

        for (typename boost::range_iterator<std::deque<turn_info> const>::type
            it = boost::begin(turns); it != boost::end(turns); ++it)
        {
            PointOut p;
            geometry::copy_coordinates(it->point, p);
            *out++ = p;
        }
        return out;
    }
};



}} // namespace detail::intersection
#endif // DOXYGEN_NO_DETAIL



#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template
<
    // tag dispatching:
    typename TagIn1, typename TagIn2, typename TagOut,
    // orientation 
    order_selector Order1, order_selector Order2, order_selector OrderOut,
    // metafunction finetuning helpers:
    bool Areal1, bool Areal2, bool ArealOut, 
    // real types
    typename Geometry1, typename Geometry2,
    typename OutputIterator,
    typename GeometryOut,
    typename Strategy
>
struct intersection_inserter
{
    BOOST_MPL_ASSERT_MSG
        (
            false, NOT_OR_NOT_YET_IMPLEMENTED_FOR_THIS_GEOMETRY_TYPES_OR_ORIENTATIONS
            , (types<Geometry1, Geometry2, GeometryOut>)
        );
};


template
<
    typename TagIn1, typename TagIn2, typename TagOut,
    typename Geometry1, typename Geometry2,
    typename OutputIterator,
    typename GeometryOut,
    typename Strategy
>
struct intersection_inserter
    <
        TagIn1, TagIn2, TagOut,
        clockwise, clockwise, clockwise,
        true, true, true,
        Geometry1, Geometry2,
        OutputIterator, GeometryOut,
        Strategy
    > : detail::overlay::overlay
        <Geometry1, Geometry2, OutputIterator, GeometryOut, -1, clockwise, Strategy>
{};


// All counter clockwise:
template
<
    typename TagIn1, typename TagIn2, typename TagOut,
    typename Geometry1, typename Geometry2,
    typename OutputIterator,
    typename GeometryOut,
    typename Strategy
>
struct intersection_inserter
    <
        TagIn1, TagIn2, TagOut,
        counterclockwise, counterclockwise, counterclockwise,
        true, true, true,
        Geometry1, Geometry2,
        OutputIterator, GeometryOut,
        Strategy
    > : detail::overlay::overlay
        <Geometry1, Geometry2, OutputIterator, GeometryOut, -1, counterclockwise, Strategy>
{};


// Boxes are never counter clockwise.
// Any counter areal type with box:
template
<
    typename TagIn, typename TagOut,
    typename Geometry, typename Box,
    typename OutputIterator,
    typename GeometryOut,
    typename Strategy
>
struct intersection_inserter
    <
        TagIn, box_tag, TagOut,
        counterclockwise, clockwise, counterclockwise,
        true, true, true,
        Geometry, Box,
        OutputIterator, GeometryOut,
        Strategy
    > : detail::overlay::overlay
        <Geometry, Box, OutputIterator, GeometryOut, -1, counterclockwise, Strategy>
{};


template
<
    typename Segment1, typename Segment2,
    typename OutputIterator, typename GeometryOut,
    typename Strategy
>
struct intersection_inserter
    <
        segment_tag, segment_tag, point_tag,
        clockwise, clockwise, clockwise,
        false, false, false,
        Segment1, Segment2,
        OutputIterator, GeometryOut,
        Strategy
    > : detail::intersection::intersection_segment_segment_point
            <
                Segment1, Segment2,
                OutputIterator, GeometryOut,
                Strategy
            >
{};


template
<
    typename Linestring1, typename Linestring2,
    typename OutputIterator, typename GeometryOut,
    typename Strategy
>
struct intersection_inserter
    <
        linestring_tag, linestring_tag, point_tag,
        clockwise, clockwise, clockwise,
        false, false, false,
        Linestring1, Linestring2,
        OutputIterator, GeometryOut,
        Strategy
    > : detail::intersection::intersection_linestring_linestring_point
            <
                Linestring1, Linestring2,
                OutputIterator, GeometryOut,
                Strategy
            >
{};


template
<
    typename Linestring, typename Box,
    typename OutputIterator, typename GeometryOut,
    typename Strategy
>
struct intersection_inserter
    <
        linestring_tag, box_tag, linestring_tag,
        clockwise, clockwise, clockwise,
        false, true, false,
        Linestring, Box,
        OutputIterator, GeometryOut,
        Strategy
    >
{
    static inline OutputIterator apply(Linestring const& linestring,
            Box const& box, OutputIterator out, Strategy const& strategy)
    {
        typedef typename point_type<GeometryOut>::type point_type;
        strategy::intersection::liang_barsky<Box, point_type> lb_strategy;
        return detail::intersection::clip_range_with_box
            <GeometryOut>(box, linestring, out, lb_strategy);
    }
};

template
<
    typename Segment, typename Box,
    typename OutputIterator, typename GeometryOut,
    typename Strategy
>
struct intersection_inserter
    <
        segment_tag, box_tag, linestring_tag,
        clockwise, clockwise, clockwise,
        false, true, false,
        Segment, Box,
        OutputIterator, GeometryOut,
        Strategy
    >
{
    static inline OutputIterator apply(Segment const& segment,
            Box const& box, OutputIterator out, Strategy const& strategy)
    {
        typedef boost::geometry::segment_range<Segment> range_type;
        range_type range(segment);

        typedef typename point_type<GeometryOut>::type point_type;
        strategy::intersection::liang_barsky<Box, point_type> lb_strategy;
        return detail::intersection::clip_range_with_box
            <GeometryOut>(box, range, out, lb_strategy);
    }
};


template
<
    typename GeometryTag1, typename GeometryTag2, typename GeometryTag3,
    order_selector Order1, order_selector Order2, order_selector OrderOut,
    bool Areal1, bool Areal2, bool ArealOut, 
    typename Geometry1, typename Geometry2,
    typename OutputIterator, typename GeometryOut,
    typename Strategy
>
struct intersection_inserter_reversed
{
    static inline OutputIterator apply(Geometry1 const& g1,
                Geometry2 const& g2, OutputIterator out,
                Strategy const& strategy)
    {
        return intersection_inserter
            <
                GeometryTag2, GeometryTag1, GeometryTag3,
                Order2, Order1, OrderOut,
                Areal2, Areal1, ArealOut, 
                Geometry2, Geometry1,
                OutputIterator, GeometryOut,
                Strategy
            >::apply(g2, g1, out, strategy);
    }
};



} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH

/*!
\brief \brief_calc2{intersection} \brief_strategy
\ingroup intersection
\details \details_calc2{intersection_inserter, spatial set theoretic intersection}
    \brief_strategy. \details_inserter{intersection}
\tparam GeometryOut \tparam_geometry{\p_l_or_c}
\tparam Geometry1 \tparam_geometry
\tparam Geometry2 \tparam_geometry
\tparam OutputIterator \tparam_out{\p_l_or_c}
\tparam Strategy Compound strategy for intersection
\param geometry1 \param_geometry
\param geometry2 \param_geometry
\param out \param_out{intersection}
\param strategy The strategy
\return \return_out

\qbk{distinguish,with strategy}
\qbk{behavior,[qbk_out __point__]:Calculates intersection points of input geometries}
\qbk{behavior,[qbk_out __linestring__]:Calculates intersection linestrings of input geometries (NYI)}
\qbk{behavior,[qbk_out __polygon__]:Calculates intersection polygons input (multi)polygons and/or boxes}
*/
template
<
    typename GeometryOut,
    typename Geometry1,
    typename Geometry2,
    typename OutputIterator,
    typename Strategy
>
inline OutputIterator intersection_inserter(Geometry1 const& geometry1,
            Geometry2 const& geometry2,
            OutputIterator out,
            Strategy const& strategy)
{
    concept::check<Geometry1 const>();
    concept::check<Geometry2 const>();

    return boost::mpl::if_c
        <
            reverse_dispatch<Geometry1, Geometry2>::type::value,
            dispatch::intersection_inserter_reversed
            <
                typename tag<Geometry1>::type,
                typename tag<Geometry2>::type,
                typename tag<GeometryOut>::type,
                point_order<Geometry1>::value,
                point_order<Geometry2>::value,
                point_order<GeometryOut>::value,
                is_areal<Geometry1>::value,
                is_areal<Geometry2>::value,
                is_areal<GeometryOut>::value,
                Geometry1,
                Geometry2,
                OutputIterator, GeometryOut,
                Strategy
            >,
            dispatch::intersection_inserter
            <
                typename tag<Geometry1>::type,
                typename tag<Geometry2>::type,
                typename tag<GeometryOut>::type,
                point_order<Geometry1>::value,
                point_order<Geometry2>::value,
                point_order<GeometryOut>::value,
                is_areal<Geometry1>::value,
                is_areal<Geometry2>::value,
                is_areal<GeometryOut>::value,
                Geometry1,
                Geometry2,
                OutputIterator, GeometryOut,
                Strategy
            >
        >::type::apply(geometry1, geometry2, out, strategy);
}


/*!
\brief \brief_calc2{intersection}
\ingroup intersection
\details \details_calc2{intersection_inserter, spatial set theoretic intersection}. \details_inserter{intersection}
\tparam GeometryOut \tparam_geometry{\p_l_or_c}
\tparam Geometry1 \tparam_geometry
\tparam Geometry2 \tparam_geometry
\tparam OutputIterator \tparam_out{\p_l_or_c}
\param geometry1 \param_geometry
\param geometry2 \param_geometry
\param out \param_out{intersection}
\return \return_out

\qbk{example,intersection_segment_inserter}
*/
template
<
    typename GeometryOut,
    typename Geometry1,
    typename Geometry2,
    typename OutputIterator
>
inline OutputIterator intersection_inserter(Geometry1 const& geometry1,
            Geometry2 const& geometry2,
            OutputIterator out)
{
    concept::check<Geometry1 const>();
    concept::check<Geometry2 const>();

    typedef strategy_intersection
        <
            typename cs_tag<GeometryOut>::type,
            Geometry1,
            Geometry2,
            typename geometry::point_type<GeometryOut>::type
        > strategy;

    return intersection_inserter<GeometryOut>(geometry1, geometry2, out,
                strategy());
}


/*!
\brief \brief_calc2{intersection}
\ingroup intersection
\details \details_calc2{intersection, spatial set theoretic intersection}.
\tparam Geometry1 \tparam_geometry
\tparam Geometry2 \tparam_geometry
\tparam Collection Collection of geometries (e.g. std::vector, std::deque, boost::geometry::multi*) of which
    the value_type (below referred to as 'GeometryOut') fulfills a \p_l_or_c concept.
\param geometry1 \param_geometry
\param geometry2 \param_geometry
\param output_collection The collection being filled or extended by the algorithm

\qbk{example,intersection_linestring}
\qbk{example,intersection_segment}
*/
template
<
    typename Geometry1,
    typename Geometry2,
    typename Collection
>
inline void intersection(Geometry1 const& geometry1,
            Geometry2 const& geometry2,
            Collection& output_collection)
{
    concept::check<Geometry1 const>();
    concept::check<Geometry2 const>();

    typedef typename boost::range_value<Collection>::type geometry_out;
    concept::check<geometry_out>();

    typedef strategy_intersection
        <
            typename cs_tag<geometry_out>::type,
            Geometry1,
            Geometry2,
            typename geometry::point_type<geometry_out>::type
        > strategy;


    intersection_inserter<geometry_out>(geometry1, geometry2,
                std::back_inserter(output_collection),
                strategy());
}


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_INTERSECTION_HPP
