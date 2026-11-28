// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_BOX_PRIMITIVES_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_BOX_PRIMITIVES_HPP

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <type_traits>
#include <boost/assert.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/geometry/core/access.hpp>
#include <boost/geometry/algorithms/assign.hpp>
#include <boost/geometry/algorithms/detail/envelope/initialize.hpp>
#include <boost/geometry/algorithms/detail/expand/indexed.hpp>
#include <boost/geometry/algorithms/detail/partition.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/pointing_segment.hpp>
#include <boost/geometry/geometries/segment.hpp>
#include <boost/geometry/strategy/cartesian/envelope_box.hpp>
#include <boost/geometry/strategy/cartesian/envelope_boxes.hpp>
#include <boost/geometry/strategy/cartesian/envelope_point.hpp>
#include <boost/geometry/strategy/cartesian/expand_box.hpp>
#include <boost/geometry/strategy/cartesian/expand_point.hpp>
#include <boost/geometry/util/detail/binary_rounding.hpp>
#include <boost/geometry/util/numeric_cast.hpp>
#include <boost/geometry/views/detail/indexed_point_view.hpp>
#include <boost/geometry/strategies/cartesian/exact/coordinate_conversion.hpp>
#include <boost/geometry/strategies/cartesian/exact/native_predicates.hpp>
#include <boost/geometry/strategies/cartesian/exact/predicate_dispatch.hpp>

#include <boost/geometry/util/detail/precise_fp.hpp>

BOOST_GEOMETRY_DETAIL_PRECISE_FP_BEGIN

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian
{

using search_point = model::d2::point_xy<double>;

// This enclosure is only for candidate searches, never a rounded geometry.
struct search_box : model::box<search_point>
{
    using base_type = model::box<search_point>;
    using base_type::base_type;
};

}}}}}

namespace boost { namespace geometry { namespace traits
{
using exact_search_box = strategies::detail::exact_cartesian::search_box;
template <> struct tag<exact_search_box> : tag<exact_search_box::base_type> {};
template <> struct point_type<exact_search_box> : point_type<exact_search_box::base_type> {};
template <std::size_t Index, std::size_t Dimension>
struct indexed_access<exact_search_box, Index, Dimension>
    : indexed_access<exact_search_box::base_type, Index, Dimension> {};
}}}

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian
{

namespace static_box_detail
{
template <typename A, typename B>
constexpr bool same_native_coordinates = fixed_native_supported<geometry::coordinate_type_t<A>>::value
    && std::is_same<geometry::coordinate_type_t<A>, geometry::coordinate_type_t<B>>::value;

template <typename Box>
inline void initialize(Box& box)
{
    double const infinity = (std::numeric_limits<double>::infinity)();
    geometry::detail::envelope::initialize<Box>::apply(box, infinity, -infinity);
}

template <std::size_t Dimension, typename Point>
inline scalar_approximation approximation(Point const& input)
{
    if constexpr (!std::is_void<typename root_of<Point>::type>::value)
    {
        auto result = input.approximation(Dimension);
        if (!result.valid()) result = input.refine(Dimension);
        return result;
    }
    else if constexpr (fixed_native_supported<geometry::coordinate_type_t<Point>>::value)
    {
        return coordinate_conversion::input_approximation(geometry::get<Dimension>(input),
            std::is_integral<geometry::coordinate_type_t<Point>>());
    }
    else
    {
        auto const rounded = geometry::detail::binary_rounding::round<double>(exact_value(geometry::get<Dimension>(input)));
        return {rounded.value, rounded.exact ? 0
            : std::abs(rounded.value) * std::numeric_limits<double>::epsilon()
                + (std::numeric_limits<double>::min)()};
    }
}

inline double lower(scalar_approximation const& value)
{
    if (!value.valid()) return -(std::numeric_limits<double>::infinity)();
    if (value.error == 0) return value.value;
    return std::nextafter(value.value - value.error, -(std::numeric_limits<double>::infinity)());
}
inline double upper(scalar_approximation const& value)
{
    if (!value.valid()) return (std::numeric_limits<double>::infinity)();
    if (value.error == 0) return value.value;
    return std::nextafter(value.value + value.error, (std::numeric_limits<double>::infinity)());
}

using merge_bounds = geometry::detail::expand::expand_indexed<0, dimension<search_box>::value, true>;

template <typename Point>
inline search_box point_enclosure(Point const& point)
{
    auto const x = approximation<0>(point), y = approximation<1>(point);
    return {search_point(lower(x), lower(y)), search_point(upper(x), upper(y))};
}

template <std::size_t Index, typename Box>
inline auto corner(Box const& box, int) -> decltype(box.min_corner())
{ return Index == min_corner ? box.min_corner() : box.max_corner(); }

template <std::size_t Index, typename Point>
inline Point const& corner(model::segment<Point> const& segment, int)
{ return Index == 0 ? segment.first : segment.second; }

template <std::size_t Index, typename Point>
inline Point const& corner(model::referring_segment<Point> const& segment, int)
{ return Index == 0 ? segment.first : segment.second; }

template <std::size_t Index, typename Point>
inline Point const& corner(model::pointing_segment<Point> const& segment, int)
{
    BOOST_ASSERT(segment.first && segment.second);
    return Index == 0 ? *segment.first : *segment.second;
}

template <std::size_t Index, typename Box>
inline geometry::detail::indexed_point_view<Box const, Index> corner(Box const& box, long)
{ return geometry::detail::indexed_point_view<Box const, Index>(box); }

template <std::size_t Index, typename Box>
inline decltype(auto) corner(Box const& box) { return corner<Index>(box, 0); }

template <std::size_t Index, typename Geometry>
inline decltype(auto) corner_operand(Geometry const& source)
{
    using point = typename geometry::point_type<Geometry>::type;
    if constexpr (!std::is_void<typename root_of<point>::type>::value)
        return corner<Index>(source);
    else
        return geometry::detail::indexed_point_view<Geometry const, Index>(source);
}


template <typename Coordinate, std::size_t X, std::size_t Y, typename A, typename B, typename Box>
inline int corner_orientation(A const& a, B const& b, Box const& box)
{
    using selected = std::conditional_t<fixed_native_supported<geometry::coordinate_type_t<A>>::value
        && fixed_native_supported<geometry::coordinate_type_t<B>>::value
        && fixed_native_supported<geometry::coordinate_type_t<Box>>::value, void, Coordinate>;
    if constexpr (X == Y)
        return static_orientation<selected>(a, b, corner_operand<X>(box));
    else if constexpr (fixed_native_supported<geometry::coordinate_type_t<Box>>::value)
    {
        using corner_type = model::d2::point_xy<geometry::coordinate_type_t<Box>, geometry::coordinate_system_t<Box>>;
        return static_orientation<selected>(a, b, corner_type(geometry::get<X, 0>(box), geometry::get<Y, 1>(box)));
    }
    else
    {
        auto const& x = corner<X>(box);
        auto const& y = corner<Y>(box);
        return with_homogeneous(a, b, [&](auto const& av, auto const& bv)
        {
            return with_homogeneous(x, y, [&](auto const& xv, auto const& yv)
            { return fixed_orientation(av, bv, fixed_splice(xv, 0, yv, 1)); });
        });
    }
}

template <std::size_t Dimension, typename Point>
inline int infinity(Point const& point, std::true_type)
{
    double const value = static_cast<double>(geometry::get<Dimension>(point));
    return std::isinf(value) ? value < 0 ? -1 : 1 : 0;
}
template <std::size_t Dimension, typename Point>
inline int infinity(Point const&, std::false_type) { return 0; }

template <std::size_t Dimension, typename Coordinate, typename A, typename B>
inline int compare_coordinate(A const& a, B const& b)
{
    int const ai = infinity<Dimension>(a, fixed_native_supported<geometry::coordinate_type_t<A>>());
    int const bi = infinity<Dimension>(b, fixed_native_supported<geometry::coordinate_type_t<B>>());
    if (ai || bi) return (ai > bi) - (ai < bi);
    using selected = std::conditional_t<fixed_native_supported<geometry::coordinate_type_t<A>>::value
        && fixed_native_supported<geometry::coordinate_type_t<B>>::value, void, Coordinate>;
    return static_compare_coordinate<Dimension, selected>(a, b);
}

template <std::size_t Index, std::size_t Dimension, typename Coordinate, typename Point, typename Output>
inline void assign_bound(Point const& source, Output& output)
{
    using target = geometry::coordinate_type_t<Output>;
    if constexpr (fixed_native_supported<geometry::coordinate_type_t<Point>>::value
                  && fixed_native_supported<target>::value)
        geometry::set<Index, Dimension>(output, geometry::util::numeric_cast<target>(geometry::get<Dimension>(source)));
    else
        geometry::set<Index, Dimension>(output, coordinate_conversion::output_rounder<target>::apply(
            exact_value(geometry::get<Dimension>(source))));
}

template <std::size_t Index, typename Coordinate, typename Point, typename Output>
inline void assign_corner(Point const& point, Output& output)
{
    assign_bound<Index, 0, Coordinate>(point, output);
    assign_bound<Index, 1, Coordinate>(point, output);
}

template <typename Coordinate, typename Box, typename Output>
inline void convert_box(Box const& input, Output& output)
{
    if constexpr (fixed_native_supported<geometry::coordinate_type_t<Box>>::value
               && fixed_native_supported<geometry::coordinate_type_t<Output>>::value)
    {
        strategy::envelope::cartesian_box::apply(input, output);
    }
    else
    {
        assign_corner<min_corner, Coordinate>(corner_operand<min_corner>(input), output);
        assign_corner<max_corner, Coordinate>(corner_operand<max_corner>(input), output);
    }
}

template <typename Coordinate, typename Point, typename Box>
inline void assign_point(Point const& point, Box& box)
{
    if constexpr (fixed_native_supported<geometry::coordinate_type_t<Point>>::value
               && fixed_native_supported<geometry::coordinate_type_t<Box>>::value)
    {
        strategy::envelope::cartesian_point::apply(point, box);
    }
    else
    {
        assign_corner<min_corner, Coordinate>(point, box);
        assign_corner<max_corner, Coordinate>(point, box);
    }
}

template <std::size_t Dimension, typename Coordinate, typename Point, typename Box>
inline void expand_dimension(Point const& point, Box& box)
{
    if (compare_coordinate<Dimension, Coordinate>(point, corner_operand<min_corner>(box)) < 0)
        assign_bound<min_corner, Dimension, Coordinate>(point, box);
    if (compare_coordinate<Dimension, Coordinate>(point, corner_operand<max_corner>(box)) > 0)
        assign_bound<max_corner, Dimension, Coordinate>(point, box);
}

template <typename Coordinate, typename Point, typename Box>
inline void expand_exact(Point const& point, Box& box)
{
    if constexpr (same_native_coordinates<Point, Box>)
    {
        strategy::expand::cartesian_point::apply(box, point);
    }
    else
    {
        expand_dimension<0, Coordinate>(point, box);
        expand_dimension<1, Coordinate>(point, box);
    }
}
} // namespace static_box_detail

template <typename Coordinate>
struct static_envelope_point
{
    using cs_tag = cartesian_tag;
    template <typename Point>
    static void apply(Point const& point, search_box& box)
    { box = static_box_detail::point_enclosure(point); }
    template <typename Point, typename Box>
    static void apply(Point const& point, Box& box)
    { static_box_detail::assign_point<Coordinate>(point, box); }
};

template <typename Coordinate>
struct static_expand_point
{
    template <typename Point>
    static void apply(search_box& box, Point const& point)
    { static_box_detail::merge_bounds::apply(box, static_box_detail::point_enclosure(point)); }
    template <typename Box, typename Point>
    static void apply(Box& box, Point const& point) { static_box_detail::expand_exact<Coordinate>(point, box); }
};

template <typename Coordinate>
struct static_envelope_range
{
    template <typename Range>
    static void apply(Range const& range, search_box& box)
    {
        static_box_detail::initialize(box);
        for (auto const& point : range) static_expand_point<Coordinate>::apply(box, point);
    }
    template <typename Range, typename Box>
    static void apply(Range const& range, Box& box)
    {
        auto current = boost::begin(range), end = boost::end(range);
        if (current == end) { geometry::assign_inverse(box); return; }
        // Select coordinate origins before output conversion; iterators also
        // remain valid when dereferencing the source returns a point proxy.
        auto min_x = current, min_y = current, max_x = current, max_y = current;
        for (++current; current != end; ++current)
        {
            if (static_compare_coordinate<0, Coordinate>(*current, *min_x) < 0) min_x = current;
            if (static_compare_coordinate<1, Coordinate>(*current, *min_y) < 0) min_y = current;
            if (static_compare_coordinate<0, Coordinate>(*current, *max_x) > 0) max_x = current;
            if (static_compare_coordinate<1, Coordinate>(*current, *max_y) > 0) max_y = current;
        }
        static_box_detail::assign_bound<min_corner, 0, Coordinate>(*min_x, box);
        static_box_detail::assign_bound<min_corner, 1, Coordinate>(*min_y, box);
        static_box_detail::assign_bound<max_corner, 0, Coordinate>(*max_x, box);
        static_box_detail::assign_bound<max_corner, 1, Coordinate>(*max_y, box);
    }
};

template <typename Coordinate> struct static_expand_box;

template <typename Coordinate>
struct static_envelope_box
{
    using cs_tag = cartesian_tag;
    static void apply(search_box const& input, search_box& box) { box = input; }
    template <typename Box>
    static void apply(Box const& input, search_box& box)
    {
        static_box_detail::initialize(box);
        static_expand_box<Coordinate>::apply(box, input);
    }
    template <typename Input, typename Output>
    static void apply(Input const& input, Output& output)
    { static_box_detail::convert_box<Coordinate>(input, output); }
};

template <typename Coordinate>
struct static_expand_box
{
    static void apply(search_box& box, search_box const& input)
    {
        static_box_detail::merge_bounds::apply(box, input);
    }
    template <typename Box>
    static void apply(search_box& box, Box const& input)
    {
        if constexpr (std::is_same<geometry::coordinate_type_t<Box>, double>::value)
        {
            strategy::expand::cartesian_box::apply(box, input);
        }
        else
        {
            static_expand_point<Coordinate>::apply(box, static_box_detail::corner_operand<min_corner>(input));
            static_expand_point<Coordinate>::apply(box, static_box_detail::corner_operand<max_corner>(input));
        }
    }
    template <typename Output, typename Input>
    static void apply(Output& box, Input const& input)
    {
        if constexpr (static_box_detail::same_native_coordinates<Input, Output>)
        {
            if constexpr (std::is_same<Input, search_box>::value)
                static_box_detail::merge_bounds::apply(box, input);
            else
                strategy::expand::cartesian_box::apply(box, input);
        }
        else
        {
            static_box_detail::expand_exact<Coordinate>(static_box_detail::corner_operand<min_corner>(input), box);
            static_box_detail::expand_exact<Coordinate>(static_box_detail::corner_operand<max_corner>(input), box);
        }
    }
};

template <typename Coordinate>
struct static_envelope_segment
{
    template <typename First, typename Second, typename Box>
    static void apply(First const& first, Second const& second, Box& box)
    {
        static_envelope_point<Coordinate>::apply(first, box);
        static_expand_point<Coordinate>::apply(box, second);
    }
    template <typename Segment, typename Box>
    static void apply(Segment const& segment, Box& box)
    { apply(static_box_detail::corner_operand<0>(segment), static_box_detail::corner_operand<1>(segment), box); }
};

template <typename Coordinate>
struct static_expand_segment
{
    template <typename Box, typename Segment>
    static void apply(Box& box, Segment const& segment)
    {
        static_expand_point<Coordinate>::apply(box, static_box_detail::corner_operand<0>(segment));
        static_expand_point<Coordinate>::apply(box, static_box_detail::corner_operand<1>(segment));
    }
};

template <typename Coordinate>
struct static_box_accumulation
{
    template <typename Box, typename Input>
    static void start(Box& box, Input const& input)
    { static_envelope_box<Coordinate>::apply(input, box); }

    template <typename Box, typename Input>
    static void merge(Box& box, Input const& input)
    { static_expand_box<Coordinate>::apply(box, input); }

    template <typename Box>
    static void initialize(Box& box) { geometry::assign_inverse(box); }
};

template <typename Coordinate>
using static_envelope_boxes = strategy::envelope::box_accumulation_detail::box_accumulator
    <static_box_accumulation<Coordinate>>;

template <typename Coordinate>
struct static_box_access
{
    // Public boxes may have inverted corner ordinates.
    static constexpr bool check_both_corners = true;

    template <typename Geometry>
    static bool empty(Geometry const&) { return false; }

    template <std::size_t D, std::size_t I, typename Point, typename Box>
    static int compare_point(Point const& point, Box const& box)
    {
        return static_box_detail::compare_coordinate<D, Coordinate>(
            point, static_box_detail::corner_operand<I>(box));
    }

    template <std::size_t D, std::size_t I, std::size_t J, typename A, typename B>
    static int compare_bounds(A const& a, B const& b)
    {
        return static_box_detail::compare_coordinate<D, Coordinate>(
            static_box_detail::corner_operand<I>(a), static_box_detail::corner_operand<J>(b));
    }
};

}}}}}

namespace boost { namespace geometry { namespace detail { namespace partition
{
template <>
struct divide_box_midpoint<strategies::detail::exact_cartesian::search_box>
{
    static double apply(double a, double b)
    {
        if (!std::isfinite(a) && !std::isfinite(b)) return 0;
        if (!std::isfinite(a)) return -(std::numeric_limits<double>::max)();
        if (!std::isfinite(b)) return (std::numeric_limits<double>::max)();
        return (std::max)(a, (std::min)(b, a / 2 + b / 2));
    }
};
}}}}

BOOST_GEOMETRY_DETAIL_PRECISE_FP_END

#endif
