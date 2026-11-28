// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Licensed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_BOX_CLIPPING_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_BOX_CLIPPING_HPP

#include <cmath>
#include <cstddef>
#include <type_traits>
#include <utility>
#include <boost/range/size.hpp>
#include <boost/geometry/algorithms/clear.hpp>
#include <boost/geometry/geometries/segment.hpp>
#include <boost/geometry/strategies/cartesian/exact/box_relations.hpp>
#include <boost/geometry/strategies/cartesian/exact/relate_primitives.hpp>
#include <boost/geometry/strategies/cartesian/exact/box_primitives.hpp>
#include <boost/geometry/util/numeric_cast.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian
{

template <typename Coordinate, box_relation Relation>
using box_predicate = box_relation_predicate<static_box_access<Coordinate>, Relation>;

template <typename Coordinate>
struct box_topology
{
    static auto equals() { return box_predicate<Coordinate, box_relation::equals>(); }

    static auto touches()
    {
        return box_predicate<Coordinate, box_relation::touches>();
    }

    static auto overlaps()
    {
        return box_predicate<Coordinate, box_relation::overlaps>();
    }
};

template <typename Coordinate, typename OutputPolicy = strategies::relate::approximate_output>
struct box_intersection
{
private:
    template <std::size_t Dimension, typename Box1, typename Box2, typename Output>
    static bool apply_dimension(Box1 const& first, Box2 const& second, Output& output)
    {
        auto const& max1 = static_box_detail::corner_operand<max_corner>(first);
        auto const& min2 = static_box_detail::corner_operand<min_corner>(second);
        if (static_box_detail::compare_coordinate<Dimension, Coordinate>(max1, min2) < 0)
        {
            return false;
        }
        auto const& max2 = static_box_detail::corner_operand<max_corner>(second);
        auto const& min1 = static_box_detail::corner_operand<min_corner>(first);
        if (static_box_detail::compare_coordinate<Dimension, Coordinate>(max2, min1) < 0)
        {
            return false;
        }

        if (static_box_detail::compare_coordinate<Dimension, Coordinate>(min1, min2) < 0)
            static_box_detail::assign_bound<min_corner, Dimension, Coordinate>(min2, output);
        else static_box_detail::assign_bound<min_corner, Dimension, Coordinate>(min1, output);
        if (static_box_detail::compare_coordinate<Dimension, Coordinate>(max1, max2) > 0)
            static_box_detail::assign_bound<max_corner, Dimension, Coordinate>(max2, output);
        else static_box_detail::assign_bound<max_corner, Dimension, Coordinate>(max1, output);
        return true;
    }

public:
    template <typename Box1, typename Box2, typename Output>
    static bool apply(Box1 const& first, Box2 const& second, Output& output)
    {
        // Commit each overlapping dimension without reading untouched output.
        return apply_dimension<0>(first, second, output)
            && apply_dimension<1>(first, second, output);
    }
};

namespace box_clip_detail
{
template <typename Point, typename Cross>
bool clip_halfplane(int ca, int cb, Point& a, Point& b,
                    bool& clipped_a, bool& clipped_b, Cross const& cross)
{
    if (ca < 0 && cb < 0) return false;
    if (ca >= 0 && cb >= 0) return true;
    if (ca == 0) { b = a; clipped_b = true; return true; }
    if (cb == 0) { a = b; clipped_a = true; return true; }
    if (ca < 0) { a = cross(); clipped_a = true; }
    else { b = cross(); clipped_b = true; }
    return true;
}

template <bool Construct, typename Point>
void publish(model::referring_segment<Point>& segment, Point& a, Point& b,
             bool clipped_a, bool clipped_b, bool& first_clipped, bool& second_clipped)
{
    if (Construct)
    {
        first_clipped = clipped_a;
        second_clipped = clipped_b;
        if (clipped_a) segment.first = std::move(a);
        if (clipped_b) segment.second = std::move(b);
    }
}
} // namespace box_clip_detail

template <typename Coordinate>
struct segment_box
{
    template <typename Segment, typename Box>
    static bool apply(Segment const& segment, Box const& box)
    {
        auto const& a = static_box_detail::corner<0>(segment);
        auto const& b = static_box_detail::corner<1>(segment);
        auto const& lower = static_box_detail::corner_operand<min_corner>(box);
        auto const& upper = static_box_detail::corner_operand<max_corner>(box);
        auto const compare_x = [](auto const& first, auto const& second)
        { return static_box_detail::compare_coordinate<0, Coordinate>(first, second); };
        auto const compare_y = [](auto const& first, auto const& second)
        { return static_box_detail::compare_coordinate<1, Coordinate>(first, second); };
        if (compare_x(lower, upper) > 0 || compare_y(lower, upper) > 0
            || (compare_x(a, lower) < 0 && compare_x(b, lower) < 0)
            || (compare_x(a, upper) > 0 && compare_x(b, upper) > 0)
            || (compare_y(a, lower) < 0 && compare_y(b, lower) < 0)
            || (compare_y(a, upper) > 0 && compare_y(b, upper) > 0))
            return true;

        if constexpr (std::is_floating_point<geometry::coordinate_type_t<Box>>::value)
        {
            if (static_box_detail::infinity<0>(lower, std::true_type{})
                || static_box_detail::infinity<1>(lower, std::true_type{})
                || static_box_detail::infinity<0>(upper, std::true_type{})
                || static_box_detail::infinity<1>(upper, std::true_type{}))
            {
                int const dx = compare_x(b, a), dy = compare_y(b, a);
                if (dx == 0 || dy == 0) return false;
                // The linear orientation functional reaches its extrema at
                // opposite corners. An unbounded extremum has a known sign.
                if (dy > 0)
                    return dx > 0 ? unbounded_disjoint<max_corner, min_corner>(a, b, box)
                                  : unbounded_disjoint<max_corner, max_corner>(a, b, box);
                return dx > 0 ? unbounded_disjoint<min_corner, min_corner>(a, b, box)
                              : unbounded_disjoint<min_corner, max_corner>(a, b, box);
            }
        }

        // The remaining separating axis is normal to the supporting line.
        // Strict signs retain closed-boundary contacts, including degeneracies.
        int const side = static_box_detail::corner_orientation<Coordinate, min_corner, min_corner>(a, b, box);
        return side != 0
            && static_box_detail::corner_orientation<Coordinate, max_corner, min_corner>(a, b, box) == side
            && static_box_detail::corner_orientation<Coordinate, max_corner, max_corner>(a, b, box) == side
            && static_box_detail::corner_orientation<Coordinate, min_corner, max_corner>(a, b, box) == side;
    }

private:
    template <std::size_t X, std::size_t Y, typename A, typename B, typename Box>
    static int extreme_side(A const& a, B const& b, Box const& box, int infinite_sign)
    {
        return std::isinf(geometry::get<X, 0>(box)) || std::isinf(geometry::get<Y, 1>(box))
            ? infinite_sign : static_box_detail::corner_orientation<Coordinate, X, Y>(a, b, box);
    }

    template <std::size_t MinX, std::size_t MinY, typename A, typename B, typename Box>
    static bool unbounded_disjoint(A const& a, B const& b, Box const& box)
    {
        return extreme_side<MinX, MinY>(a, b, box, -1) > 0
            || extreme_side<1 - MinX, 1 - MinY>(a, b, box, 1) < 0;
    }
};

}}}}} // namespace boost::geometry::strategies::detail::exact_cartesian

#endif
