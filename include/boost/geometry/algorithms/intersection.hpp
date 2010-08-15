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
#include <boost/range/metafunctions.hpp>


#include <boost/geometry/core/reverse_dispatch.hpp>
#include <boost/geometry/geometries/concepts/check.hpp>
#include <boost/geometry/algorithms/detail/overlay/clip_linestring.hpp>
#include <boost/geometry/algorithms/detail/overlay/assemble.hpp>
#include <boost/geometry/ranges/segment_range.hpp>


namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template
<
    typename TagIn1, typename TagIn2, typename TagOut,
    typename Geometry1, typename Geometry2,
    typename OutputIterator,
    typename GeometryOut,
    typename Strategy
>
struct intersection_inserter
    : detail::overlay::overlay
        <Geometry1, Geometry2, OutputIterator, GeometryOut, -1, Strategy>
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
        Segment1, Segment2,
        OutputIterator, GeometryOut,
        Strategy
    >
{
    static inline OutputIterator apply(Segment1 const& segment1,
            Segment2 const& segment2, OutputIterator out,
            Strategy const& strategy)
    {
        typedef typename point_type<GeometryOut>::type point_type;

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
            GeometryOut p;
            geometry::copy_coordinates(is.intersections[i], p);
            *out = p;
            out++;
        }
        return out;
    }
};


template
<
    typename Linestring, typename Box,
    typename OutputIterator, typename GeometryOut,
    typename Strategy
>
struct intersection_inserter
    <
        linestring_tag, box_tag, linestring_tag,
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
                Geometry2, Geometry1,
                OutputIterator, GeometryOut,
                Strategy
            >::apply(g2, g1, out, strategy);
    }
};



} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH

/*!
    \brief Intersects two geometries
    \ingroup intersection
    \details The two input geometries are intersected and the resulting
        linestring(s), ring(s) or polygon(s) are sent
        to the specified output operator.
    \tparam GeometryOut output geometry type, must be specified
    \tparam Geometry1 first geometry type
    \tparam Geometry2 second geometry type
    \tparam OutputIterator output iterator
    \tparam Strategy compound strategy for intersection
    \param geometry1 first geometry
    \param geometry2 second geometry
    \param out the output iterator, outputting linestrings or polygons
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
                Geometry1,
                Geometry2,
                OutputIterator, GeometryOut,
                Strategy
            >
        >::type::apply(geometry1, geometry2, out, strategy);
}


/*!
    \brief Intersects two geometries
    \ingroup intersection
    \details The two input geometries are intersected and the resulting
        linestring(s), ring(s) or polygon(s) are sent
        to the specified output operator.
    \tparam GeometryOut output geometry type, must be specified
    \tparam Geometry1 first geometry type
    \tparam Geometry2 second geometry type
    \tparam OutputIterator output iterator
    \param geometry1 first geometry
    \param geometry2 second geometry
    \param out the output iterator, outputting linestrings or polygons
    \return the output iterator
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
    \brief Intersects two geometries
    \ingroup intersection
    \details The two input geometries are intersected and the resulting
        linestring(s), ring(s) or polygon(s) are added
        to the specified collection.
    \tparam Geometry1 first geometry type
    \tparam Geometry2 second geometry type
    \tparam Collection collection of rings, polygons (e.g. a vector<polygon> or a multi_polygon)
    \param geometry1 first geometry
    \param geometry2 second geometry
    \param output_collection the collection
    \return true if successful
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
