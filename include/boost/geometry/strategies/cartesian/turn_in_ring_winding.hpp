// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2020 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2023 Adam Wulkiewicz, Lodz, Poland.

// This file was modified by Oracle on 2023.
// Modifications copyright (c) 2023 Oracle and/or its affiliates.
// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_TURN_IN_RING_WINDING_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_TURN_IN_RING_WINDING_HPP

#include <type_traits>

#include <boost/geometry/arithmetic/infinite_line_functions.hpp>
#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/config.hpp>
#include <boost/geometry/util/math.hpp>

#include <boost/geometry/strategies/cartesian/side_rounded_input.hpp>

namespace boost { namespace geometry
{

namespace strategy { namespace buffer
{

#ifndef DOXYGEN_NO_DETAIL

enum place_on_ring_type
{
    // +----offsetted----> (offsetted is considered as outside)
    // |                 |
    // |                 |
    // left              right (first point outside, rest inside)
    // |                 |
    // |                 |
    // <-----original----+ (original is considered as inside)
    place_on_ring_offsetted,
    place_on_ring_original,
    place_on_ring_to_offsetted,
    place_on_ring_from_offsetted,
};

template <typename CalculationType, typename Strategy = void>
class turn_in_ring_winding
{

    // Implements the winding rule.
    // Basic calculations (on a clockwise ring of 5 segments)
    // (as everywhere in BG, -1 = right, 0 = on segment, +1 = left)
    // +--------2--------+  // P : For 1/3, nothing happens, it returns
    // |                 |  //     For 2, side is right (-1), multiplier=2, -2
    // |        P        |  //     For 4, side is right (-1), multiplier=1, -1
    // 1                 3  //     For 5, side is right (-1), multiplier=1, -1, total -4
    // |             Q   |  // Q : For 2: -2, for 4: -2, total -4
    // |                 |  // R : For 2: -2, for 5: +2, total 0
    // +----5---*----4---+  // S : For 2: -1, 3: nothing, 4: +1, total 0
    //
    //     R             S
    //

    template <typename Point, typename PointOfSegment>
    static bool near_tip(Point const& point, PointOfSegment const& s1,
                         PointOfSegment const& s2, bool at_second)
    {
        CalculationType value1, value2;
        if constexpr (uses_default_order)
        {
            auto const line1 = arithmetic::make_perpendicular_line<CalculationType>(s1, s2, s1);
            auto const line2 = arithmetic::make_perpendicular_line<CalculationType>(s2, s1, s2);
            value1 = arithmetic::side_value(line1, point);
            value2 = arithmetic::side_value(line2, point);
        }
        else
        {
            CalculationType const ax(get<0>(s1)), ay(get<1>(s1));
            CalculationType const bx(get<0>(s2)), by(get<1>(s2));
            CalculationType const px(get<0>(point)), py(get<1>(point));
            value1 = (bx - ax) * (px - ax) + (by - ay) * (py - ay);
            value2 = (bx - ax) * (bx - px) + (by - ay) * (by - py);
        }
        if (!(value1 >= 0 && value2 >= 0)) return false;

        auto const length_value = value1 + value2;
        if (!(length_value > 0)) return false;

        auto const& projection = at_second ? value2 : value1;
        // Preserve the stock division and its rounding at the cutoff.
        if constexpr (uses_default_order)
            return projection / length_value < 1.0e-3;
        else
            return projection < length_value * CalculationType(1.0e-3);
    }

public:
    static constexpr bool uses_default_order = std::is_void<Strategy>::value;
    template <std::size_t D, typename A, typename B>
    static int order(A const& a, B const& b)
    {
        if constexpr (std::is_void<Strategy>::value)
        {
            CalculationType const av = get<D>(a), bv = get<D>(b);
            return (av > bv) - (av < bv);
        }
        else
        {
            return Strategy::topology().template compare_coordinate<D>(a, b);
        }
    }

    struct counter
    {
        //! Counter, is increased if point is left of a segment (outside),
        //! and decreased if point is right of a segment (inside)
        int count{0};

        int count_on_offsetted{0};
        int count_on_origin{0};
        int count_on_edge{0};

        bool near_helper_tip = false;

        inline bool is_inside() const
        {
            return count < 0 || count_on_origin > 0;
        }

        inline bool is_on_boundary() const
        {
            return count_on_origin == 0
                && (count_on_offsetted > 0
                || near_helper_tip
                );
        }

    };

    using state_type = counter;

    template <typename Point, typename PointOfSegment>
    static inline bool is_in_vertical_range(Point const& point,
                             PointOfSegment const& s1,
                             PointOfSegment const& s2)
    {
        return order<1>(s1, point) * order<1>(s2, point) <= 0;
    }

    template <typename Point, typename PointOfSegment>
    static inline void apply_on_boundary(Point const& point,
                             PointOfSegment const& s1,
                             PointOfSegment const& s2,
                             place_on_ring_type place_on_ring,
                             counter& the_state)
    {
        if (place_on_ring == place_on_ring_offsetted)
        {
            the_state.count_on_offsetted++;
        }
        else if (place_on_ring == place_on_ring_to_offsetted
            || place_on_ring == place_on_ring_from_offsetted)
        {
            the_state.count_on_edge++;

            if (!the_state.near_helper_tip)
                the_state.near_helper_tip = near_tip(point, s1, s2, place_on_ring == place_on_ring_to_offsetted);
        }
        else
        {
            the_state.count_on_origin++;
        }
    }

    template <typename Point, typename PointOfSegment>
    static inline bool apply(Point const& point,
                             PointOfSegment const& s1,
                             PointOfSegment const& s2,
                             place_on_ring_type place_on_ring,
                             bool is_convex,
                             counter& the_state)
    {
        int const side = [&]
        {
            if constexpr (std::is_void<Strategy>::value)
                return strategy::side::side_rounded_input<CalculationType>::apply(s1, s2, point);
            else
                return Strategy::side().apply(s1, s2, point);
        }();

        if (is_convex && side > 0)
        {
            // If the point is left of this segment of a convex piece, it can never be inside.
            // Stop further processing
            the_state.count = 1;
            return false;
        }

        int const x1 = order<0>(s1, point), x2 = order<0>(s2, point);
        bool const in_horizontal_range = x1 * x2 <= 0;
        bool const vertical = order<0>(s1, s2) == 0;

        if (vertical ? is_in_vertical_range(point, s1, s2) : in_horizontal_range)
        {
            if (side == 0)
            {
                apply_on_boundary(point, s1, s2, place_on_ring, the_state);
            }
        }

        if (in_horizontal_range)
        {
            auto const on_boundary = the_state.count_on_offsetted + the_state.count_on_edge + the_state.count_on_origin;
            if (on_boundary == 0)
            {
                // Use only absolute comparisons, because the ring is continuous -
                // what was missed is there earlier or later, and turns should
                // not be counted twice (which can happen if an epsilon is used).
                bool const eq1 = x1 == 0;
                bool const eq2 = x2 == 0;

                // Account for  1 or  2 for left side (outside)
                //     and for -1 or -2 for right side (inside)
                int const multiplier = eq1 || eq2 ? 1 : 2;

                the_state.count += side * multiplier;
            }
        }

        return true;
    }
};

#endif // DOXYGEN_NO_DETAIL

}} // namespace strategy::buffer

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_CARTESIAN_TURN_IN_RING_WINDING_HPP

