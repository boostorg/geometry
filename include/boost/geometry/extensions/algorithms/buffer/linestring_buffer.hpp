// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_LINESTRING_BUFFER_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_LINESTRING_BUFFER_HPP

#include <cstddef>

#include <boost/numeric/conversion/cast.hpp>

#include <boost/range.hpp>

#include <boost/geometry/extensions/strategies/buffer_side.hpp>

#include <boost/geometry/extensions/algorithms/buffer/buffer_appender.hpp>
#include <boost/geometry/extensions/algorithms/buffer/range_buffer.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace buffer
{

template
<
    typename Linestring,
    typename Polygon,
    typename DistanceStrategy,
    typename JoinStrategy
>
struct linestring_buffer 
    : public range_buffer
        <
            typename ring_type<Polygon>::type, 
            DistanceStrategy, 
            JoinStrategy,
            linestring_tag
        >
{

    template
    <
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
        typename Mapper
#endif
    >
    static inline void apply(Linestring const& linestring, Polygon& buffered,
            DistanceStrategy const& distance,
            JoinStrategy const& join_strategy
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
            , Mapper& mapper
#endif
            )
    {
        buffer_appender<typename geometry::ring_type<Polygon>::type> appender
            (
                geometry::exterior_ring(buffered)
            );

        iterate(appender, boost::begin(linestring), boost::end(linestring),
            buffer_side_left,
            distance, join_strategy
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
            , mapper
#endif
            );

        iterate(appender, boost::rbegin(linestring), boost::rend(linestring),
            buffer_side_right,
            distance, join_strategy
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
            , mapper
#endif
            );

    }
};


}} // namespace detail::buffer
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_LINESTRING_BUFFER_HPP
