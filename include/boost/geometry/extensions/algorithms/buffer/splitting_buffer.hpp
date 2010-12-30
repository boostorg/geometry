// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_SPLITTING_BUFFER_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_SPLITTING_BUFFER_HPP

#include <cstddef>

#include <boost/foreach.hpp>
#include <boost/numeric/conversion/cast.hpp>

#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>

#include <boost/geometry/core/point_type.hpp>

#include <boost/geometry/strategies/buffer_side.hpp>
#include <boost/geometry/algorithms/detail/buffer/intersecting_inserter.hpp>
#include <boost/geometry/algorithms/detail/buffer/linestring_buffer.hpp>
#include <boost/geometry/algorithms/detail/buffer/line_line_intersection.hpp>


#include <boost/geometry/algorithms/detail/overlay/dissolver.hpp>
#include <boost/geometry/algorithms/detail/overlay/split_rings.hpp>



namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace buffer
{


template
<
    typename GeometryOut, typename Geometry,
    typename DistanceStrategy,
    typename JoinStrategy
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
    , typename Mapper
#endif
>
inline void splitting_buffer(Geometry const& geometry,
    std::vector<GeometryOut>& buffered,
    DistanceStrategy const& distance_strategy,
    JoinStrategy const& join_strategy

    , int option
    )
{
    typedef typename ring_type<GeometryOut>::type ring_type;
    typedef detail::buffer::intersecting_inserter
        <
            std::vector<GeometryOut>
        > inserter_type;


    inserter_type inserter(buffered);

    detail::buffer::linestring_buffer
        <
            Geometry, GeometryOut, DistanceStrategy, JoinStrategy
        >::apply(geometry, inserter, distance_strategy, join_strategy);

    if (option == 0)
    {
        return;
    }

    std::vector<ring_type> rings;
    BOOST_FOREACH(GeometryOut const& polygon, buffered)
    {
//std::cout << bg::wkt(polygon) << " ; POLYGON" << std::endl;
        geometry::split_rings(polygon, rings);
    }

    if (option == 1)
    {
        buffered.resize(rings.size());
        int i = 0;
        BOOST_FOREACH(ring_type const& ring, rings)
        {
            exterior_ring(buffered[i++]) = ring;
        }
        return;
    }

    std::vector<GeometryOut> buffered_and_unioned;
    geometry::dissolver(rings, buffered_and_unioned);

    std::vector<GeometryOut> buffered_and_assembled;
    detail::overlay::assemble<GeometryOut>(buffered_and_unioned,
            std::map<ring_identifier, int>(),
            buffered_and_unioned[0], buffered_and_unioned[0], 1, true, true,
            std::back_inserter(buffered_and_assembled));

    buffered = buffered_and_assembled;
}



}} // namespace detail::buffer
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_SPLITTING_BUFFER_HPP
