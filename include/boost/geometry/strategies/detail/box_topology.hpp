// Boost.Geometry
// Copyright (c) 2007-2015 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2015 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2015 Mateusz Loskot, London, UK.
// Copyright (c) 2013-2022 Adam Wulkiewicz, Lodz, Poland.
// Modifications copyright (c) 2013-2024 Oracle and/or its affiliates.
// Licensed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_GEOMETRY_STRATEGIES_DETAIL_BOX_TOPOLOGY_HPP
#define BOOST_GEOMETRY_STRATEGIES_DETAIL_BOX_TOPOLOGY_HPP

#include <cstddef>
#include <type_traits>

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/coordinate_dimension.hpp>
#include <boost/geometry/core/coordinate_system.hpp>
#include <boost/geometry/core/coordinate_type.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{

namespace box_topology_detail
{

template <std::size_t Dimension, std::size_t DimensionCount>
struct touches_loop
{
    template <typename Box1, typename Box2>
    static inline bool apply(Box1 const& b1, Box2 const& b2, bool& touch)
    {
        using coordinate_type1 = coordinate_type_t<Box1>;
        using coordinate_type2 = coordinate_type_t<Box2>;

        coordinate_type1 const& min1 = get<min_corner, Dimension>(b1);
        coordinate_type1 const& max1 = get<max_corner, Dimension>(b1);
        coordinate_type2 const& min2 = get<min_corner, Dimension>(b2);
        coordinate_type2 const& max2 = get<max_corner, Dimension>(b2);

        // TODO assert or exception?
        //BOOST_GEOMETRY_ASSERT(min1 <= max1 && min2 <= max2);

        if (max1 < min2 || max2 < min1)
        {
            return false;
        }

        if (max1 == min2 || max2 == min1)
        {
            touch = true;
        }

        return touches_loop<Dimension + 1, DimensionCount>::apply(b1, b2, touch);
    }
};

template <std::size_t DimensionCount>
struct touches_loop<DimensionCount, DimensionCount>
{
    template <typename Box1, typename Box2>
    static inline bool apply(Box1 const&, Box2 const&, bool&) { return true; }
};

template <std::size_t Dimension, std::size_t DimensionCount>
struct overlaps_loop
{
    template <typename Box1, typename Box2>
    static inline void apply(Box1 const& b1, Box2 const& b2,
            bool& overlaps, bool& one_in_two, bool& two_in_one)
    {
        assert_dimension_equal<Box1, Box2>();

        using coordinate_type1 = coordinate_type_t<Box1>;
        using coordinate_type2 = coordinate_type_t<Box2>;

        coordinate_type1 const& min1 = get<min_corner, Dimension>(b1);
        coordinate_type1 const& max1 = get<max_corner, Dimension>(b1);
        coordinate_type2 const& min2 = get<min_corner, Dimension>(b2);
        coordinate_type2 const& max2 = get<max_corner, Dimension>(b2);

        // We might use the (not yet accepted) Boost.Interval
        // submission in the future

        // If:
        // B1: |-------|
        // B2:           |------|
        // in any dimension -> no overlap
        if (max1 <= min2 || min1 >= max2)
        {
            overlaps = false;
            return;
        }

        // If:
        // B1: |--------------------|
        // B2:   |-------------|
        // in all dimensions -> within, then no overlap
        // B1: |--------------------|
        // B2: |-------------|
        // this is "within-touch" -> then no overlap. So use < and >
        if (min1 < min2 || max1 > max2)
        {
            one_in_two = false;
        }

        // Same other way round
        if (min2 < min1 || max2 > max1)
        {
            two_in_one = false;
        }

        overlaps_loop<Dimension + 1, DimensionCount>
            ::apply(b1, b2, overlaps, one_in_two, two_in_one);
    }
};

template <std::size_t DimensionCount>
struct overlaps_loop<DimensionCount, DimensionCount>
{
    template <typename Box1, typename Box2>
    static inline void apply(Box1 const&, Box2 const&, bool&, bool&, bool&) {}
};

} // namespace box_topology_detail

struct box_touches
{
    template <typename Box1, typename Box2>
    static inline bool apply(Box1 const& b1, Box2 const& b2)
    {
        static_assert(std::is_same<geometry::coordinate_system_t<Box1>,
                                  geometry::coordinate_system_t<Box2>>::value,
                      "Touches can only be evaluated for boxes of same coordinate system.");
        assert_dimension_equal<Box1, Box2>();

        bool touches = false;
        bool ok = box_topology_detail::touches_loop<0, dimension<Box1>::type::value>
            ::apply(b1, b2, touches);
        return ok && touches;
    }
};

struct box_overlaps
{
    template <typename Box1, typename Box2>
    static inline bool apply(Box1 const& b1, Box2 const& b2)
    {
        bool overlaps = true;
        bool within1 = true;
        bool within2 = true;
        box_topology_detail::overlaps_loop<0, dimension<Box1>::type::value>
            ::apply(b1, b2, overlaps, within1, within2);

        // Containment, including shared boundary segments, is not overlap.
        return overlaps && !within1 && !within2;
    }
};

struct coordinate_box_topology
{
    static box_touches touches() { return {}; }
    static box_overlaps overlaps() { return {}; }
};

namespace box_topology_detail
{

template <typename Strategy>
inline auto get_box_topology(Strategy const& strategy, int)
    -> decltype(strategy.box_topology())
{
    return strategy.box_topology();
}

template <typename Strategy>
inline coordinate_box_topology get_box_topology(Strategy const&, long) { return {}; }

} // namespace box_topology_detail

template <typename Strategy>
inline decltype(auto) get_box_topology(Strategy const& strategy)
{
    return box_topology_detail::get_box_topology(strategy, 0);
}

}}}} // namespace boost::geometry::strategies::detail

#endif // BOOST_GEOMETRY_STRATEGIES_DETAIL_BOX_TOPOLOGY_HPP
