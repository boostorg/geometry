// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Licensed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_BOX_CLIPPING_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_BOX_CLIPPING_HPP

#include <cstddef>
#include <type_traits>
#include <utility>
#include <boost/assert.hpp>
#include <boost/range/size.hpp>
#include <boost/geometry/algorithms/clear.hpp>
#include <boost/geometry/geometries/segment.hpp>
#include <boost/geometry/strategies/cartesian/exact/point_conversion.hpp>
#include <boost/geometry/strategies/cartesian/exact/relate_primitives.hpp>
#include <boost/geometry/strategies/cartesian/exact/static_box.hpp>
#include <boost/geometry/util/numeric_cast.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian
{

enum class box_relation { within, covered_by, disjoint, touches, overlaps };

template <typename Coordinate, box_relation Relation>
struct box_predicate
{
    using cs_tag = cartesian_tag;
private:
    template <typename Point, typename Box>
    static bool apply_impl(Point const& point, Box const& box, point_tag)
    {
        using predicates = static_box_predicates<Coordinate>;
        if (Relation == box_relation::within) return predicates::point_covered_by(point, box, true);
        if (Relation == box_relation::covered_by) return predicates::point_covered_by(point, box);
        if (Relation == box_relation::disjoint) return !predicates::point_covered_by(point, box);
        if (Relation == box_relation::touches) return predicates::point_covered_by(point, box)
            && !predicates::point_covered_by(point, box, true);
        return false;
    }
    template <typename First, typename Second>
    static bool apply_impl(First const& first, Second const& second, box_tag)
    {
        using predicates = static_box_predicates<Coordinate>;
        if (Relation == box_relation::within) return predicates::within(first, second);
        if (Relation == box_relation::covered_by) return predicates::covered_by(first, second);
        if (Relation == box_relation::disjoint) return predicates::disjoint(first, second);
        if (Relation == box_relation::touches) return predicates::touches(first, second);
        return predicates::overlaps(first, second);
    }
public:
    template <typename Geometry1, typename Geometry2>
    static bool apply(Geometry1 const& first, Geometry2 const& second)
    {
        return apply_impl(first, second, geometry::tag_t<Geometry1>());
    }
};

template <typename Coordinate>
struct box_topology
{
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
    template <std::size_t Index, std::size_t Dimension, typename Point, typename Output>
    static void assign_bound(Point const& source, Output& output)
    {
        using target_point = typename geometry::point_type<Output>::type;
        if constexpr (std::is_same<target_point, exact_point<Coordinate>>::value)
        {
            static_box_detail::assign_bound<Index, Dimension, Coordinate>(source, output);
        }
        else
        {
            using target = geometry::coordinate_type_t<Output>;
            if constexpr (static_predicates_detail::exact_input<std::remove_cv_t<Point>>::value)
                geometry::set<Index, Dimension>(output, output_coordinate<target, OutputPolicy>(source, Dimension));
            else
                geometry::set<Index, Dimension>(output, geometry::util::numeric_cast<target>(geometry::get<Dimension>(source)));
        }
    }

    template <std::size_t Dimension, typename Box1, typename Box2, typename Output>
    static bool apply_dimension(Box1 const& first, Box2 const& second, Output& output)
    {
        auto const& max1 = static_box_detail::borrowed_corner<max_corner>(first);
        auto const& min2 = static_box_detail::borrowed_corner<min_corner>(second);
        if (static_box_detail::compare_coordinate<Dimension, Coordinate>(max1, min2) < 0)
        {
            return false;
        }
        auto const& max2 = static_box_detail::borrowed_corner<max_corner>(second);
        auto const& min1 = static_box_detail::borrowed_corner<min_corner>(first);
        if (static_box_detail::compare_coordinate<Dimension, Coordinate>(max2, min1) < 0)
        {
            return false;
        }

        if (static_box_detail::compare_coordinate<Dimension, Coordinate>(min1, min2) < 0)
            assign_bound<min_corner, Dimension>(min2, output);
        else assign_bound<min_corner, Dimension>(min1, output);
        if (static_box_detail::compare_coordinate<Dimension, Coordinate>(max1, max2) > 0)
            assign_bound<max_corner, Dimension>(max2, output);
        else assign_bound<max_corner, Dimension>(max1, output);
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

template <typename Coordinate, typename OutputPolicy = strategies::relate::approximate_output>
struct clipping
{
    using point_type = exact_point<Coordinate>;

    static auto get_equals_point_point_strategy() { return point_equal<Coordinate>(); }

    template <typename Source, typename Destination>
    static void convert_geometry(Source const& source, Destination& destination)
    {
        point_converter<Coordinate, OutputPolicy>::apply(source, destination);
    }

    template <bool Construct = true, typename Box>
    bool clip_segment(Box const& box, model::referring_segment<point_type>& segment,
                      bool& first_clipped, bool& second_clipped) const
    {
        auto a = segment.first;
        auto b = segment.second;
        auto const lower = make_point<Coordinate>(static_box_detail::corner<min_corner>(box));
        auto const upper = make_point<Coordinate>(static_box_detail::corner<max_corner>(box));
        bool clipped_a = false, clipped_b = false;
        if (!clip_halfplane<0, 1>(lower, segment.first, segment.second, a, b, clipped_a, clipped_b)
            || !clip_halfplane<0, -1>(upper, segment.first, segment.second, a, b, clipped_a, clipped_b)
            || !clip_halfplane<1, 1>(lower, segment.first, segment.second, a, b, clipped_a, clipped_b)
            || !clip_halfplane<1, -1>(upper, segment.first, segment.second, a, b, clipped_a, clipped_b)) return false;
        if (Construct)
        {
            first_clipped = clipped_a;
            second_clipped = clipped_b;
            if (clipped_a) segment.first = std::move(a);
            if (clipped_b) segment.second = std::move(b);
        }
        return true;
    }

private:
    template <std::size_t Dimension, int Direction>
    static bool clip_halfplane(point_type const& bound, point_type const& support_a,
                               point_type const& support_b, point_type& a, point_type& b,
                               bool& clipped_a, bool& clipped_b)
    {
        int const ca = Direction * static_compare_coordinate<Dimension>(a, bound);
        int const cb = Direction * static_compare_coordinate<Dimension>(b, bound);
        if (ca < 0 && cb < 0) return false;
        if (ca >= 0 && cb >= 0) return true;
        if (ca == 0) { b = a; clipped_b = true; return true; }
        if (cb == 0) { a = b; clipped_a = true; return true; }
        point_type first, second;
        Coordinate native;
        if (bound.native_coordinate(Dimension, native))
        {
            first = Dimension == 0 ? point_type(native, 0) : point_type(0, native);
            second = Dimension == 0 ? point_type(native, 1) : point_type(1, native);
        }
        else
        {
            point_type const zero(0, 0), one(1, 1);
            first = Dimension == 0 ? point_type::splice_coordinates(bound, 0, zero, 1)
                                   : point_type::splice_coordinates(zero, 0, bound, 1);
            second = Dimension == 0 ? point_type::splice_coordinates(bound, 0, one, 1)
                                    : point_type::splice_coordinates(one, 0, bound, 1);
        }
        auto intersection = construct_intersection(support_a, support_b, first, second);
        if (ca < 0) { a = std::move(intersection); clipped_a = true; }
        else { b = std::move(intersection); clipped_b = true; }
        return true;
    }

public:

    template <typename Linestring, typename Iterator>
    static void apply(Linestring& line, Iterator& output)
    {
        if (boost::size(line) > 1)
        {
            *output++ = line;
        }
        geometry::clear(line);
    }
};

template <typename Coordinate>
struct segment_box
{
    static auto get_disjoint_point_box_strategy()
    {
        return box_predicate<Coordinate, box_relation::covered_by>();
    }

    template <typename Segment, typename Box>
    static bool apply(Segment const& segment, Box const& box)
    {
        auto const& a = endpoint<0>(segment, 0);
        auto const& b = endpoint<1>(segment, 0);
        auto const& lower = static_box_detail::borrowed_corner<min_corner>(box);
        auto const& upper = static_box_detail::borrowed_corner<max_corner>(box);
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
        int const side = static_box_detail::corner_orientation<Coordinate>(a, b,
            static_box_detail::indexed_point<Box, min_corner, min_corner>{box});
        return side != 0
            && static_box_detail::corner_orientation<Coordinate>(a, b,
                static_box_detail::indexed_point<Box, max_corner, min_corner>{box}) == side
            && static_box_detail::corner_orientation<Coordinate>(a, b,
                static_box_detail::indexed_point<Box, max_corner, max_corner>{box}) == side
            && static_box_detail::corner_orientation<Coordinate>(a, b,
                static_box_detail::indexed_point<Box, min_corner, max_corner>{box}) == side;
    }

private:
    template <std::size_t Index, typename Segment>
    static auto endpoint(Segment const& segment, int)
        -> std::enable_if_t<std::is_pointer<decltype(segment.first)>::value, decltype(*segment.first)>
    {
        // segment_iterator yields pointing_segment: retain the actual implicit
        // point instead of extracting its coordinates into a rational snapshot.
        BOOST_ASSERT(segment.first != nullptr && segment.second != nullptr);
        return Index == 0 ? *segment.first : *segment.second;
    }

    template <std::size_t Index, typename Segment>
    static decltype(auto) endpoint(Segment const& segment, long)
    { return static_box_detail::borrowed_corner<Index>(segment); }

    template <std::size_t X, std::size_t Y, typename A, typename B, typename Box>
    static int extreme_side(A const& a, B const& b, Box const& box, int infinite_sign)
    {
        static_box_detail::indexed_point<Box, X, Y> const corner{box};
        return static_box_detail::infinity<0>(corner, std::true_type{})
            || static_box_detail::infinity<1>(corner, std::true_type{})
            ? infinite_sign : static_box_detail::corner_orientation<Coordinate>(a, b, corner);
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
