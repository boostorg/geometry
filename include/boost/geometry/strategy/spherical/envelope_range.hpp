// Boost.Geometry

// Copyright (c) 2021, Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_STRATEGY_SPHERICAL_ENVELOPE_RANGE_HPP
#define BOOST_GEOMETRY_STRATEGY_SPHERICAL_ENVELOPE_RANGE_HPP

#include <boost/geometry/algorithms/assign.hpp>
#include <boost/geometry/algorithms/detail/envelope/initialize.hpp>
#include <boost/geometry/geometries/segment.hpp>
#include <boost/geometry/strategy/spherical/envelope_point.hpp>
#include <boost/geometry/strategy/spherical/envelope_segment.hpp>
#include <boost/geometry/strategy/spherical/expand_segment.hpp>
#include <boost/geometry/views/closeable_view.hpp>

// TEMP - get rid of these dependencies
#include <boost/geometry/algorithms/detail/within/point_in_geometry.hpp>
#include <boost/geometry/strategies/spherical/point_in_poly_winding.hpp>


namespace boost { namespace geometry
{

namespace strategy { namespace envelope
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail
{

template <typename Range, typename Box, typename EnvelopeStrategy, typename ExpandStrategy>
inline void spheroidal_linestring(Range const& range, Box& mbr,
                                  EnvelopeStrategy const& envelope_strategy,
                                  ExpandStrategy const& expand_strategy)
{
    auto it = boost::begin(range);
    auto const end = boost::end(range);
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


template <typename Ring, typename WithinStrategy>
inline bool pole_within(bool north_pole, Ring const& ring, WithinStrategy const& within_strategy)
{
    if (boost::size(ring) < core_detail::closure::minimum_ring_size
                                <
                                    geometry::closure<Ring>::value
                                >::value)
    {
        return false;
    }

    using point_t = typename geometry::point_type<Ring>::type;
    using coord_t = typename geometry::coordinate_type<point_t>::type;
    using units_t = typename geometry::detail::cs_angular_units<point_t>::type;
    using constants_t = math::detail::constants_on_spheroid<coord_t, units_t>;
    point_t point;
    geometry::assign_zero(point);
    if (north_pole)
    {
        geometry::set<1>(point, constants_t::max_latitude());
    }
    else
    {
        geometry::set<1>(point, constants_t::min_latitude());
    }
    geometry::detail::closed_clockwise_view<Ring const> view(ring);
    return geometry::detail::within::point_in_range(point, view, within_strategy) > 0;
}

template <typename Range, typename Box, typename EnvelopeStrategy, typename ExpandStrategy, typename WithinStrategy>
inline void spheroidal_ring(Range const& range, Box& mbr,
                            EnvelopeStrategy const& envelope_strategy,
                            ExpandStrategy const& expand_strategy,
                            WithinStrategy const& within_strategy)
{
    geometry::detail::closed_view<Range const> closed_range(range);

    spheroidal_linestring(closed_range, mbr, envelope_strategy, expand_strategy);
    
    using coord_t = typename geometry::coordinate_type<Box>::type;
    using point_t = typename geometry::point_type<Box>::type;    
    using units_t = typename geometry::detail::cs_angular_units<point_t>::type;
    using constants_t = math::detail::constants_on_spheroid<coord_t, units_t>;
    coord_t const two_pi = constants_t::period();
    coord_t const lon_min = geometry::get<0, 0>(mbr);
    coord_t const lon_max = geometry::get<1, 0>(mbr);
    // If box covers the whole longitude range it is possible that the ring contains
    // one of the poles.
    // TODO: Technically it is possible that a reversed ring may cover more than
    // half of the globe and mbr of it's linear ring may be small and not cover the
    // longitude range.
    if (lon_max - lon_min >= two_pi)
    {
        coord_t const lat_n_pole = constants_t::max_latitude();
        coord_t const lat_s_pole = constants_t::min_latitude();
        coord_t lat_min = geometry::get<0, 1>(mbr);
        coord_t lat_max = geometry::get<1, 1>(mbr);
        // Normalize box latitudes, just in case
        if (math::equals(lat_min, lat_s_pole))
        {
            lat_min = lat_s_pole;
        }
        if (math::equals(lat_max, lat_n_pole))
        {
            lat_max = lat_n_pole;
        }

        // TODO - implement something simpler than within strategy because here
        // we know that neither min nor max is a pole so there is no segment which
        // contains a pole, no endpoint, no vertex at pole, there are no antipodal
        // points. So many special cases can be ignored.
        if (lat_max < lat_n_pole)
        {
            if (pole_within(true, range, within_strategy))
            {
                lat_max = lat_n_pole;
            }
        }
        if (lat_min > lat_s_pole)
        {
            if (pole_within(false, range, within_strategy))
            {
                lat_min = lat_s_pole;
            }
        }

        geometry::set<0, 1>(mbr, lat_min);
        geometry::set<1, 1>(mbr, lat_max);
    }
}

} // namespace detail
#endif // DOXYGEN_NO_DETAIL

template <typename CalculationType = void>
class spherical_linestring
{
public:
    template <typename Range, typename Box>
    static inline void apply(Range const& range, Box& mbr)
    {
        detail::spheroidal_linestring(range, mbr,
                                      envelope::spherical_segment<CalculationType>(),
                                      expand::spherical_segment<CalculationType>());
    }
};

template <typename CalculationType = void>
class spherical_ring
{
public:
    template <typename Range, typename Box>
    static inline void apply(Range const& range, Box& mbr)
    {
        detail::spheroidal_ring(range, mbr,
                                envelope::spherical_segment<CalculationType>(),
                                expand::spherical_segment<CalculationType>(),
                                within::spherical_winding<void, void, CalculationType>());
    }
};

}} // namespace strategy::envelope

}} //namepsace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGY_SPHERICAL_ENVELOPE_RANGE_HPP
