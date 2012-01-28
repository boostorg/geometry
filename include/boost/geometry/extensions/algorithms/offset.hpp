// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_ALGORITHMS_OFFSET_HPP
#define BOOST_GEOMETRY_EXTENSIONS_ALGORITHMS_OFFSET_HPP

#include <boost/config.hpp>

#include <boost/range/functions.hpp>

#include <boost/geometry/core/point_type.hpp>
#include <boost/geometry/extensions/algorithms/buffer/line_line_intersection.hpp>
#include <boost/geometry/extensions/algorithms/buffer/range_buffer.hpp>
#include <boost/geometry/extensions/algorithms/offset_appender.hpp>
#include <boost/geometry/algorithms/detail/disjoint.hpp>
#include <boost/geometry/geometries/concepts/check.hpp>

#include <boost/geometry/geometries/segment.hpp>

namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace offset
{


template
<
    typename Range,
    typename RangeOut,
    typename JoinStrategy,
    typename DistanceStrategy
>
struct offset_range
    : public geometry::detail::buffer::range_buffer
        <
            RangeOut, 
            DistanceStrategy, 
            JoinStrategy,
            linestring_tag
        >
{
    template <typename Appender>
    static inline void apply(Range const& range,
                Appender& appender,
                DistanceStrategy const& distance,
                JoinStrategy const& join)
    {
        iterate(appender, boost::begin(range), boost::end(range), 
            buffer_side_left,
            distance, join);
    }
};

}} // namespace detail::offset
#endif



#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template
<
    typename GeometryTag,
    typename GeometryOutTag,
    typename Geometry,
    typename GeometryOut,
    typename JoinStrategy,
    typename DistanceStrategy
>
struct offset
{};


template
<
    typename Geometry,
    typename GeometryOut,
    typename JoinStrategy,
    typename DistanceStrategy
>
struct offset
    <
        linestring_tag,
        linestring_tag,
        Geometry,
        GeometryOut,
        JoinStrategy,
        DistanceStrategy
    >
    : detail::offset::offset_range
        <
            Geometry,
            GeometryOut,
            JoinStrategy,
            DistanceStrategy
        >
{};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


template
<
    typename Geometry,
    typename GeometryOut,
    typename JoinStrategy,
    typename Distance
>
inline void offset(Geometry const& geometry, GeometryOut& out,
            JoinStrategy const& join,
            Distance const& distance)
{
    concept::check<Geometry const>();
    concept::check<GeometryOut>();

    typedef strategy::buffer::distance_assymetric
        <
            typename geometry::coordinate_type<Geometry>::type
        > distance_strategy_type;
    distance_strategy_type distance_strategy(distance, distance);


    typedef detail::offset::offset_appender
        <
            GeometryOut
        > appender_type;
    
    appender_type appender(out);

    dispatch::offset
    <
        typename tag<Geometry>::type,
        typename tag<GeometryOut>::type,
        Geometry,
        GeometryOut,
        JoinStrategy,
        distance_strategy_type
    >::apply(geometry, appender, distance_strategy, join);
}


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_EXTENSIONS_ALGORITHMS_OFFSET_HPP
