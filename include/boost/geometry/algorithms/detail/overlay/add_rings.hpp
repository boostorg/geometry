// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2017 Adam Wulkiewicz, Lodz, Poland.

// This file was modified by Oracle on 2017-2020.
// Modifications copyright (c) 2017-2020, Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_ADD_RINGS_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_ADD_RINGS_HPP

#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/range/value_type.hpp>
#include <boost/throw_exception.hpp>

#include <boost/geometry/core/closure.hpp>
#include <boost/geometry/core/exception.hpp>
#include <boost/geometry/algorithms/area.hpp>
#include <boost/geometry/algorithms/detail/overlay/convert_ring.hpp>
#include <boost/geometry/algorithms/detail/overlay/get_ring.hpp>
#include <boost/geometry/algorithms/detail/overlay/ring_properties.hpp>
#include <boost/geometry/strategies/relate/intersection_types.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace overlay
{

template
<
    typename GeometryOut,
    typename Geometry1,
    typename Geometry2,
    typename RingCollection,
    typename Strategy
>
inline void convert_and_add(GeometryOut& result,
            Geometry1 const& geometry1, Geometry2 const& geometry2,
            RingCollection const& collection,
            ring_identifier id,
            bool reversed, bool append, Strategy const& strategy)
{
    using tag1 = geometry::tag_t<Geometry1>;
    using tag2 = geometry::tag_t<Geometry2>;
    using tag_out = geometry::tag_t<GeometryOut>;

    if (id.source_index == 0)
    {
        convert_ring<tag_out>::apply(result,
                    get_ring<tag1>::apply(id, geometry1),
                    append, reversed, strategy);
    }
    else if (id.source_index == 1)
    {
        convert_ring<tag_out>::apply(result,
                    get_ring<tag2>::apply(id, geometry2),
                    append, reversed, strategy);
    }
    else if (id.source_index == 2)
    {
        convert_ring<tag_out>::apply(result,
                    get_ring<void>::apply(id, collection),
                    append, reversed, strategy);
    }
}

enum add_rings_error_handling
{
    add_rings_ignore_unordered,
    add_rings_add_unordered,
    add_rings_throw_if_reversed
};

template <typename GeometryOut, typename Geometry, typename OutputIterator,
          typename Strategy, bool Topological>
inline OutputIterator add_validated_ring(Geometry const& result, OutputIterator out,
        Strategy const& strategy, add_rings_error_handling error_handling,
        std::bool_constant<Topological>)
{
    bool valid, positive;
    if constexpr (Topological)
    {
        auto const status = strategy.ring_topology().analyze(result);
        valid = status.valid;
        positive = status.direction > 0;
    }
    else
    {
        auto const area = geometry::area(result, strategy);
        using strategies::detail::classify_ring_measure;
        using strategies::detail::ring_direction;
        auto const direction = classify_ring_measure(strategy, area);
        valid = direction != ring_direction::zero;
        positive = direction == ring_direction::positive;
    }
    if (valid)
    {
        if (error_handling == add_rings_add_unordered || positive)
        {
            GeometryOut output;
            strategies::detail::convert_geometry(strategy, result, output);
            *out++ = std::move(output);
        }
        else if (error_handling == add_rings_throw_if_reversed)
        {
            BOOST_THROW_EXCEPTION(invalid_output_exception());
        }
    }
    return out;
}

template
<
    typename GeometryOut,
    typename SelectionMap,
    typename Geometry1,
    typename Geometry2,
    typename RingCollection,
    typename OutputIterator,
    typename Strategy
>
inline OutputIterator add_rings(SelectionMap const& map,
            Geometry1 const& geometry1, Geometry2 const& geometry2,
            RingCollection const& collection,
            OutputIterator out,
            Strategy const& strategy,
            add_rings_error_handling error_handling = add_rings_ignore_unordered)
{
    std::size_t const min_num_points = core_detail::closure::minimum_ring_size
        <
            geometry::closure
                <
                    typename boost::range_value
                        <
                            RingCollection const
                        >::type
                >::value
        >::value;


    for (auto const& pair : map)
    {
        if (! pair.second.discarded
            && pair.second.parent.source_index == -1)
        {
            using working_geometry_type = typename strategies::relate::services::overlay_geometry_type
                <Strategy, GeometryOut>::type;
            working_geometry_type result;
            convert_and_add(result, geometry1, geometry2, collection,
                    pair.first, pair.second.reversed, false, strategy);

            // Add children
            for (auto const& child : pair.second.children)
            {
                auto mit = map.find(child);
                if (mit != map.end() && ! mit->second.discarded)
                {
                    convert_and_add(result, geometry1, geometry2, collection,
                            child, mit->second.reversed, true, strategy);
                }
            }

            // Only add rings if they satisfy minimal requirements.
            // This cannot be done earlier (during traversal), not
            // everything is figured out yet (sum of positive/negative rings)
            if (geometry::num_points(result) >= min_num_points)
            {
                out = add_validated_ring<GeometryOut>(result, out, strategy, error_handling,
                    ring_properties_detail::is_topological<typename SelectionMap::mapped_type>());
            }
        }
    }
    return out;
}


template
<
    typename GeometryOut,
    typename SelectionMap,
    typename Geometry,
    typename RingCollection,
    typename OutputIterator,
    typename Strategy
>
inline OutputIterator add_rings(SelectionMap const& map,
            Geometry const& geometry,
            RingCollection const& collection,
            OutputIterator out,
            Strategy const& strategy)
{
    Geometry empty;
    return add_rings<GeometryOut>(map, geometry, empty, collection, out, strategy);
}


}} // namespace detail::overlay
#endif // DOXYGEN_NO_DETAIL


}} // namespace geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_ADD_RINGS_HPP
