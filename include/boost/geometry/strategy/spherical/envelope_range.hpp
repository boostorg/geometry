// Boost.Geometry

// Copyright (c) 2021, Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_STRATEGY_SPHERICAL_ENVELOPE_RANGE_HPP
#define BOOST_GEOMETRY_STRATEGY_SPHERICAL_ENVELOPE_RANGE_HPP

#include <boost/geometry/algorithms/detail/envelope/initialize.hpp>
#include <boost/geometry/geometries/segment.hpp>
#include <boost/geometry/strategy/spherical/envelope_point.hpp>
#include <boost/geometry/strategy/spherical/envelope_segment.hpp>
#include <boost/geometry/strategy/spherical/expand_segment.hpp>
#include <boost/geometry/views/closeable_view.hpp>

namespace boost { namespace geometry
{

namespace strategy { namespace envelope
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail
{

template <typename Range, typename Box, typename EnvelopeStrategy, typename ExpandStrategy>
inline void spheroidal_range(Range const& range, Box& mbr,
                             EnvelopeStrategy const& envelope_strategy,
                             ExpandStrategy const& expand_strategy)
{
    geometry::detail::closed_view<Range const> closed_range(range);

    auto it = boost::begin(closed_range);
    auto const end = boost::end(closed_range);
    if (it == end)
    {
        // initialize box (assign inverse)
        geometry::detail::envelope::initialize<Box>::apply(mbr);
        return;
    }

    auto prev = it;
    ++it;
    if (it == end)
    {
        // initialize box with the first point
        envelope::spherical_point::apply(*prev, mbr);
        return;
    }

    // initialize box with the first segment
    envelope_strategy.apply(*prev, *it, mbr);

    // consider now the remaining segments in the range (if any)
    prev = it;
    ++it;
    while (it != end)
    {
        using point_t = typename boost::range_value<Range>::type;
        geometry::model::referring_segment<point_t const> const seg(*prev, *it);
        expand_strategy.apply(mbr, seg);
        prev = it;
        ++it;
    }
}

} // namespace detail
#endif // DOXYGEN_NO_DETAIL

// TODO: divide into spherical_linestring and spherical_ring

template <typename CalculationType = void>
class spherical_range
{
public:
    template <typename Range, typename Box>
    static inline void apply(Range const& range, Box& mbr)
    {
        detail::spheroidal_range(range, mbr,
                                 envelope::spherical_segment<CalculationType>(),
                                 expand::spherical_segment<CalculationType>());
    }
};

}} // namespace strategy::envelope

}} //namepsace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGY_SPHERICAL_ENVELOPE_RANGE_HPP
