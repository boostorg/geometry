// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2012 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_MULTI_BUFFER_INSERTER_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_MULTI_BUFFER_INSERTER_HPP

#include <boost/range.hpp>
#include <boost/typeof/typeof.hpp>

#include <boost/geometry/multi/core/point_type.hpp>

#include <boost/geometry/extensions/algorithms/buffer/buffer_inserter.hpp>


namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace buffer
{

template <>
struct check_original<multi_polygon_tag>
{
    template <typename Point, typename Geometry>
    static inline int apply(Point const& point, Geometry const& geometry)
    {
        return geometry::covered_by(point, geometry) ? 1 : -1;
    }
};

}} // namespace detail::buffer
#endif // DOXYGEN_NO_DETAIL



#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


template
<
    typename Multi,
    typename PolygonOutput
>
struct buffer_inserter<multi_polygon_tag, Multi, PolygonOutput>
{
    template
    <
        typename Collection, typename DistanceStrategy, typename JoinStrategy
    >
    static inline void apply(Multi const& multi,
            Collection& collection,
            DistanceStrategy const& distance,
            JoinStrategy const& join_strategy)
    {
        typedef typename ring_type<PolygonOutput>::type output_ring_type;
        typedef buffer_inserter
            <
                typename single_tag_of
                            <
                                typename tag<Multi>::type
                            >::type,
                typename boost::range_value<Multi const>::type, 
                output_ring_type
            > policy;
            
        for (typename boost::range_iterator<Multi const>::type
                it = boost::begin(multi);
            it != boost::end(multi);
            ++it)
        {
            policy::apply(*it, collection, distance, join_strategy);
        }
    }
};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_MULTI_BUFFER_INSERTER_HPP
