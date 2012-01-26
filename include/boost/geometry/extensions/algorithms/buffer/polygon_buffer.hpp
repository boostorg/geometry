// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_POLYGON_BUFFER_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_POLYGON_BUFFER_HPP

#include <cstddef>

#include <boost/numeric/conversion/cast.hpp>

#include <boost/range.hpp>
#include <boost/typeof/typeof.hpp>

#include <boost/geometry/core/exterior_ring.hpp>
#include <boost/geometry/core/interior_rings.hpp>

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
    typename RingInput,
    typename RingOutput,
    typename DistanceStrategy,
    typename JoinStrategy
>
struct ring_buffer
    : public range_buffer
        <
            RingOutput, 
            DistanceStrategy, 
            JoinStrategy,
            ring_tag
        >
{

    template
    <
        typename BufferAppender
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
        , typename Mapper
#endif
    >
    static inline void apply(RingInput const& ring,
            BufferAppender& appender,
            DistanceStrategy const& distance,
            JoinStrategy const& join_strategy
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
            , Mapper& mapper
#endif
            )
    {
        iterate(appender, boost::begin(ring), boost::end(ring),
                buffer_side_left,
                distance, join_strategy
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
            , mapper
#endif
            );
    }
};



template
<
    typename PolygonInput,
    typename PolygonOutput,
    typename DistanceStrategy,
    typename JoinStrategy
>
struct polygon_buffer
{
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
    template <typename Mapper>
#endif
    static inline void apply(PolygonInput const& polygon, PolygonOutput& buffered,
            DistanceStrategy const& distance,
            JoinStrategy const& join_strategy
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
            , Mapper& mapper
#endif
            )
    {
        geometry::clear(buffered);

        typedef typename ring_type<PolygonInput>::type input_ring_type;
        typedef typename ring_type<PolygonOutput>::type output_ring_type;

        typedef buffer_appender
            <
                output_ring_type
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
                , Mapper
#endif
            > appender_type;

        typedef ring_buffer<input_ring_type, output_ring_type, DistanceStrategy, JoinStrategy> policy;

        {
            appender_type appender(geometry::exterior_ring(buffered)
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
                    , mapper
#endif
                );
            policy::apply(exterior_ring(polygon), appender,
                    distance, join_strategy
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
                    , mapper
#endif
                    );
        }

        typename interior_return_type<PolygonInput const>::type rings
                    = interior_rings(polygon);
        for (BOOST_AUTO_TPL(it, boost::begin(rings)); it != boost::end(rings); ++it)
        {
            output_ring_type ring;

            appender_type appender(ring
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
                , mapper
#endif
                );

            policy::apply(*it, appender, distance, join_strategy
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
                , mapper
#endif
                );
            interior_rings(buffered).push_back(ring);
        }
    }
};


}} // namespace detail::buffer
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_POLYGON_BUFFER_HPP
