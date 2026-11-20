// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_STATIC_BOX_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_STATIC_BOX_HPP

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <type_traits>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/geometry/core/access.hpp>
#include <boost/geometry/algorithms/assign.hpp>
#include <boost/geometry/algorithms/detail/assign_indexed_point.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/segment.hpp>
#include <boost/geometry/util/algorithm.hpp>
#include <boost/geometry/util/numeric_cast.hpp>
#include <boost/geometry/strategies/cartesian/exact/point.hpp>
#include <boost/geometry/strategies/cartesian/exact/native_predicates.hpp>
#include <boost/geometry/strategies/cartesian/exact/static_predicates.hpp>
#include <boost/geometry/strategies/detail/partition.hpp>

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

namespace static_box_detail
{
template <typename Geometry, std::size_t XIndex, std::size_t YIndex = XIndex>
struct indexed_point
{
    Geometry const& source;
};
}

}}}}}

namespace boost { namespace geometry { namespace traits
{
template <> struct tag<strategies::detail::exact_cartesian::search_box> { using type = box_tag; };
template <> struct point_type<strategies::detail::exact_cartesian::search_box>
{ using type = strategies::detail::exact_cartesian::search_point; };
template <std::size_t Index, std::size_t Dimension>
struct indexed_access<strategies::detail::exact_cartesian::search_box, Index, Dimension>
{
    using box = strategies::detail::exact_cartesian::search_box;
    static double get(box const& value)
    { return geometry::get<Dimension>(Index == min_corner ? value.min_corner() : value.max_corner()); }
    static void set(box& value, double coordinate)
    { geometry::set<Dimension>(Index == min_corner ? value.min_corner() : value.max_corner(), coordinate); }
};

template <typename Geometry, std::size_t X, std::size_t Y>
struct tag<strategies::detail::exact_cartesian::static_box_detail::indexed_point<Geometry, X, Y>>
{ using type = point_tag; };
template <typename Geometry, std::size_t X, std::size_t Y>
struct coordinate_type<strategies::detail::exact_cartesian::static_box_detail::indexed_point<Geometry, X, Y>>
{ using type = geometry::coordinate_type_t<Geometry>; };
template <typename Geometry, std::size_t X, std::size_t Y>
struct coordinate_system<strategies::detail::exact_cartesian::static_box_detail::indexed_point<Geometry, X, Y>>
{ using type = typename geometry::coordinate_system<Geometry>::type; };
template <typename Geometry, std::size_t X, std::size_t Y>
struct dimension<strategies::detail::exact_cartesian::static_box_detail::indexed_point<Geometry, X, Y>>
    : geometry::dimension<Geometry> {};
template <typename Geometry, std::size_t X, std::size_t Y, std::size_t Dimension>
struct access<strategies::detail::exact_cartesian::static_box_detail::indexed_point<Geometry, X, Y>, Dimension>
{
    using view = strategies::detail::exact_cartesian::static_box_detail::indexed_point<Geometry, X, Y>;
    static geometry::coordinate_type_t<Geometry> get(view const& point)
    { return geometry::get<Dimension == 0 ? X : Y, Dimension>(point.source); }
};
}}}

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian
{

namespace static_box_detail
{
template <typename T> using remove_cref = std::remove_cv_t<std::remove_reference_t<T>>;
template <typename Box> struct is_search_box : std::is_same<remove_cref<Box>, search_box> {};

inline void initialize(search_box& box)
{
    double const infinity = (std::numeric_limits<double>::infinity)();
    box.min_corner() = search_point(infinity, infinity);
    box.max_corner() = search_point(-infinity, -infinity);
}

template <std::size_t Dimension, typename Coordinate, typename Point>
inline interval bounds(Point const& point, std::true_type)
{
    return point_detail::input_interval(geometry::get<Dimension>(point),
        std::is_integral<geometry::coordinate_type_t<Point>>());
}

template <std::size_t Dimension, typename Coordinate, typename Point>
inline interval bounds(Point const& input, std::false_type)
{
    if constexpr (!std::is_void<typename static_predicates_detail::root_of<Point>::type>::value)
    {
        auto result = input.approximation(Dimension);
        if (!result.valid()) result = input.refine(Dimension);
        return result;
    }
    else
    {
        auto const point = make_point<Coordinate>(input);
        auto result = point.approximation(Dimension);
        if (!result.valid()) result = point.refined_approximation(Dimension);
        return result;
    }
}

template <std::size_t Dimension, typename Coordinate, typename Point>
inline interval bounds(Point const& point)
{ return bounds<Dimension, Coordinate>(point, fixed_native_supported<geometry::coordinate_type_t<Point>>()); }

inline double lower(interval const& value)
{
    if (!value.valid()) return -(std::numeric_limits<double>::infinity)();
    if (value.error == 0) return value.approximate;
    return arithmetic_detail::downward(arithmetic_detail::rounded_sub(value.approximate, value.error));
}
inline double upper(interval const& value)
{
    if (!value.valid()) return (std::numeric_limits<double>::infinity)();
    if (value.error == 0) return value.approximate;
    return arithmetic_detail::upward(arithmetic_detail::rounded_add(value.approximate, value.error));
}

// Merge bounds by role. An inverse (empty) search box must not be expanded
// as two points: doing so would make its infinities cover the entire plane.
template <std::size_t Dimension>
inline void expand_bounds(search_box& box, double low, double high)
{
    geometry::set<min_corner, Dimension>(box, (std::min)(geometry::get<min_corner, Dimension>(box), low));
    geometry::set<max_corner, Dimension>(box, (std::max)(geometry::get<max_corner, Dimension>(box), high));
}

template <typename Coordinate, typename Point>
inline void expand(search_box& box, Point const& point)
{
    auto const x = bounds<0, Coordinate>(point), y = bounds<1, Coordinate>(point);
    expand_bounds<0>(box, lower(x), upper(x));
    expand_bounds<1>(box, lower(y), upper(y));
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

template <std::size_t Index, typename Box>
inline model::d2::point_xy<geometry::coordinate_type_t<Box>> corner(Box const& box, long)
{ return {geometry::get<Index, 0>(box), geometry::get<Index, 1>(box)}; }

template <std::size_t Index, typename Box>
inline decltype(auto) corner(Box const& box) { return corner<Index>(box, 0); }

template <std::size_t Index, typename Geometry>
inline decltype(auto) borrowed_corner(Geometry const& source)
{
    using point = typename geometry::point_type<Geometry>::type;
    if constexpr (static_predicates_detail::exact_input<remove_cref<point>>::value)
        return corner<Index>(source);
    else
        return indexed_point<Geometry, Index>{source};
}

template <typename Point, typename Function>
inline int with_homogeneous(Point const& point, Function const& function)
{
    if constexpr (!std::is_void<typename static_predicates_detail::root_of<Point>::type>::value)
        return function(point.homogeneous());
    else if constexpr (fixed_native_supported<geometry::coordinate_type_t<Point>>::value)
        return function(fixed_input_homogeneous(geometry::get<0>(point), geometry::get<1>(point)));
    else
        return point.has_fixed_representation() ? function(point.fixed_homogeneous())
                                               : function(point.precision_homogeneous());
}

template <typename Coordinate, typename A, typename B, typename Box, std::size_t X, std::size_t Y>
inline int corner_orientation(A const& a, B const& b, indexed_point<Box, X, Y> const& point)
{
    using selected = std::conditional_t<fixed_native_supported<geometry::coordinate_type_t<A>>::value
        && fixed_native_supported<geometry::coordinate_type_t<B>>::value
        && fixed_native_supported<geometry::coordinate_type_t<Box>>::value, void, Coordinate>;
    if constexpr (X == Y)
        return static_orientation<selected>(a, b, borrowed_corner<X>(point.source));
    else if constexpr (fixed_native_supported<geometry::coordinate_type_t<Box>>::value)
        return static_orientation<selected>(a, b, point);
    else
    {
        // A mixed corner borrows its two implicit bounds. Splicing homogeneous
        // coordinates is predicate scratch, not a stored constructed point.
        auto const& first = static_predicates_detail::predicate_operand<Coordinate>(a);
        auto const& second = static_predicates_detail::predicate_operand<Coordinate>(b);
        auto const& x = static_predicates_detail::predicate_operand<Coordinate>(corner<X>(point.source));
        auto const& y = static_predicates_detail::predicate_operand<Coordinate>(corner<Y>(point.source));
        return with_homogeneous(first, [&](auto const& av)
        {
            return with_homogeneous(second, [&](auto const& bv)
            {
                return with_homogeneous(x, [&](auto const& xv)
                {
                    return with_homogeneous(y, [&](auto const& yv)
                    { return fixed_orientation(av, bv, fixed_splice(xv, 0, yv, 1)); });
                });
            });
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

template <typename Output, bool Native = fixed_native_supported<Output>::value>
struct coordinate_output
{
    template <std::size_t Dimension, typename Coordinate>
    static Output apply(exact_point<Coordinate> const& point)
    { return point_detail::output_rounder<Output>::apply(point.exact_coordinate(Dimension)); }
};

template <typename Output>
struct coordinate_output<Output, true>
{
    template <std::size_t Dimension, typename Coordinate>
    static Output apply(exact_point<Coordinate> const& point)
    {
        Coordinate native;
        if (point.native_coordinate(Dimension, native))
            return geometry::util::numeric_cast<Output>(native);
        std::array<double, 8> values;
        Output rounded;
        if (point.native_producers(values) && try_round_intersection(values, Dimension, rounded)) return rounded;
        if (point.has_fixed_representation())
        {
            if (try_fixed_round(point.fixed_homogeneous(), Dimension, rounded)) return rounded;
        }
        else if (try_fixed_round(point.precision_homogeneous(), Dimension, rounded)) return rounded;
        throw std::overflow_error("Exact Cartesian box output cannot be represented");
    }
};

template <std::size_t Dimension, typename Coordinate>
inline void replace_coordinate(exact_point<Coordinate>& output, exact_point<Coordinate> const& source)
{
    Coordinate first, second;
    auto const& x = Dimension == 0 ? source : output;
    auto const& y = Dimension == 1 ? source : output;
    if (x.native_coordinate(0, first) && y.native_coordinate(1, second))
        output = exact_point<Coordinate>(first, second);
    else output = exact_point<Coordinate>::splice_coordinates(x, 0, y, 1);
}

template <std::size_t Index, std::size_t Dimension, typename Coordinate, typename Output>
inline void assign_bound(exact_point<Coordinate> const& source, Output& output, std::false_type)
{
    using target = geometry::coordinate_type_t<Output>;
    geometry::set<Index, Dimension>(output, coordinate_output<target>::template apply<Dimension>(source));
}

template <std::size_t Index, std::size_t Dimension, typename Coordinate, typename Output>
inline void assign_bound(exact_point<Coordinate> const& source, Output& output, std::true_type)
{ replace_coordinate<Dimension>(Index == min_corner ? output.min_corner() : output.max_corner(), source); }

template <std::size_t Index, std::size_t Dimension, typename Coordinate, typename Point, typename Output>
inline void assign_bound(Point const& source, Output& output)
{
    if constexpr (fixed_native_supported<geometry::coordinate_type_t<Point>>::value
                  && fixed_native_supported<geometry::coordinate_type_t<Output>>::value)
    {
        geometry::set<Index, Dimension>(output, geometry::util::numeric_cast<
            geometry::coordinate_type_t<Output>>(geometry::get<Dimension>(source)));
    }
    else
    {
        using output_point = typename geometry::point_type<Output>::type;
        assign_bound<Index, Dimension>(make_point<Coordinate>(source), output,
            std::is_same<output_point, exact_point<Coordinate>>());
    }
}

template <std::size_t Index, typename Coordinate, typename Point, typename Output>
inline void assign_corner(Point const& point, Output& output)
{
    if constexpr (fixed_native_supported<geometry::coordinate_type_t<Point>>::value
                  && fixed_native_supported<geometry::coordinate_type_t<Output>>::value)
        geometry::detail::assign_point_to_index<Index>(point, output);
    else
    {
        assign_bound<Index, 0, Coordinate>(point, output);
        assign_bound<Index, 1, Coordinate>(point, output);
    }
}

template <typename Coordinate, typename Box, typename Output>
inline void convert_box(Box const& input, Output& output)
{
    // Known model boxes expose whole corners, preserving implicit provenance.
    auto const& a = borrowed_corner<min_corner>(input);
    auto const& b = borrowed_corner<max_corner>(input);
    assign_corner<min_corner, Coordinate>(a, output);
    assign_corner<max_corner, Coordinate>(b, output);
}

template <typename Coordinate, typename Point, typename Box>
inline void assign_point(Point const& point, Box& box)
{
    assign_corner<min_corner, Coordinate>(point, box);
    assign_corner<max_corner, Coordinate>(point, box);
}

template <std::size_t Dimension, typename Coordinate, typename Point, typename Box>
inline void expand_dimension(Point const& point, Box& box)
{
    if (compare_coordinate<Dimension, Coordinate>(point, borrowed_corner<min_corner>(box)) < 0)
        assign_bound<min_corner, Dimension, Coordinate>(point, box);
    if (compare_coordinate<Dimension, Coordinate>(point, borrowed_corner<max_corner>(box)) > 0)
        assign_bound<max_corner, Dimension, Coordinate>(point, box);
}

template <typename Coordinate, typename Point, typename Box>
inline void expand_exact(Point const& point, Box& box)
{
    expand_dimension<0, Coordinate>(point, box);
    expand_dimension<1, Coordinate>(point, box);
}
} // namespace static_box_detail

template <typename Coordinate>
struct static_envelope_point
{
    using cs_tag = cartesian_tag;
    template <typename Point>
    static void apply(Point const& point, search_box& box)
    { static_box_detail::initialize(box); static_box_detail::expand<Coordinate>(box, point); }
    template <typename Point, typename Box>
    static void apply(Point const& point, Box& box)
    { static_box_detail::assign_point<Coordinate>(point, box); }
};

template <typename Coordinate>
struct static_expand_point
{
    template <typename Point>
    static void apply(search_box& box, Point const& point) { static_box_detail::expand<Coordinate>(box, point); }
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
        for (auto const& point : range) static_box_detail::expand<Coordinate>(box, point);
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

template <typename Coordinate>
struct static_envelope_box
{
    using cs_tag = cartesian_tag;
    static void apply(search_box const& input, search_box& box) { box = input; }
    template <typename Box>
    static void apply(Box const& input, search_box& box)
    {
        static_box_detail::initialize(box);
        static_box_detail::expand<Coordinate>(box, static_box_detail::borrowed_corner<min_corner>(input));
        static_box_detail::expand<Coordinate>(box, static_box_detail::borrowed_corner<max_corner>(input));
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
        geometry::detail::for_each_dimension<search_box>([&](auto d)
        {
            static_box_detail::expand_bounds<d>(box, geometry::get<min_corner, d>(input),
                                                      geometry::get<max_corner, d>(input));
        });
    }
    template <typename Box>
    static void apply(search_box& box, Box const& input)
    {
        static_box_detail::expand<Coordinate>(box, static_box_detail::borrowed_corner<min_corner>(input));
        static_box_detail::expand<Coordinate>(box, static_box_detail::borrowed_corner<max_corner>(input));
    }
    template <typename Output, typename Input>
    static void apply(Output& box, Input const& input)
    {
        static_box_detail::expand_exact<Coordinate>(static_box_detail::borrowed_corner<min_corner>(input), box);
        static_box_detail::expand_exact<Coordinate>(static_box_detail::borrowed_corner<max_corner>(input), box);
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
    { apply(static_box_detail::borrowed_corner<0>(segment), static_box_detail::borrowed_corner<1>(segment), box); }
};

template <typename Coordinate>
struct static_expand_segment
{
    template <typename Box, typename Segment>
    static void apply(Box& box, Segment const& segment)
    {
        static_expand_point<Coordinate>::apply(box, static_box_detail::borrowed_corner<0>(segment));
        static_expand_point<Coordinate>::apply(box, static_box_detail::borrowed_corner<1>(segment));
    }
};

template <typename Coordinate>
struct static_envelope_boxes
{
    template <typename Box> struct state { Box box; bool initialized = false; };
    template <typename Box, typename Input>
    static void apply(state<Box>& current, Input const& box)
    {
        if (!current.initialized)
        {
            static_envelope_box<Coordinate>::apply(box, current.box);
            current.initialized = true;
        }
        else static_expand_box<Coordinate>::apply(current.box, box);
    }
    template <typename Box>
    static void result(state<Box> const& current, Box& box)
    {
        if (current.initialized) box = current.box;
        else geometry::assign_inverse(box);
    }
};

template <typename Coordinate>
struct static_box_predicates
{
    template <typename Point, typename Box>
    static bool point_covered_by(Point const& point, Box const& box, bool strict = false)
    {
        auto const& lower = static_box_detail::borrowed_corner<min_corner>(box);
        auto const& upper = static_box_detail::borrowed_corner<max_corner>(box);
        int const lx = static_box_detail::compare_coordinate<0, Coordinate>(point, lower);
        int const ly = static_box_detail::compare_coordinate<1, Coordinate>(point, lower);
        int const ux = static_box_detail::compare_coordinate<0, Coordinate>(point, upper);
        int const uy = static_box_detail::compare_coordinate<1, Coordinate>(point, upper);
        return strict ? lx > 0 && ly > 0 && ux < 0 && uy < 0
                      : lx >= 0 && ly >= 0 && ux <= 0 && uy <= 0;
    }
    template <typename First, typename Second>
    static bool disjoint(First const& first, Second const& second)
    {
        auto const& a = static_box_detail::borrowed_corner<min_corner>(first);
        auto const& b = static_box_detail::borrowed_corner<max_corner>(first);
        auto const& c = static_box_detail::borrowed_corner<min_corner>(second);
        auto const& d = static_box_detail::borrowed_corner<max_corner>(second);
        return static_box_detail::compare_coordinate<0, Coordinate>(b, c) < 0
            || static_box_detail::compare_coordinate<0, Coordinate>(d, a) < 0
            || static_box_detail::compare_coordinate<1, Coordinate>(b, c) < 0
            || static_box_detail::compare_coordinate<1, Coordinate>(d, a) < 0;
    }
    template <typename First, typename Second>
    static bool covered_by(First const& first, Second const& second)
    {
        return point_covered_by(static_box_detail::borrowed_corner<min_corner>(first), second)
            && point_covered_by(static_box_detail::borrowed_corner<max_corner>(first), second);
    }
    template <typename First, typename Second>
    static bool within(First const& first, Second const& second)
    {
        auto const& a = static_box_detail::borrowed_corner<min_corner>(first);
        auto const& b = static_box_detail::borrowed_corner<max_corner>(first);
        return covered_by(first, second)
            && static_box_detail::compare_coordinate<0, Coordinate>(a, b) < 0
            && static_box_detail::compare_coordinate<1, Coordinate>(a, b) < 0;
    }
    template <typename First, typename Second>
    static bool overlaps(First const& first, Second const& second)
    {
        if (covered_by(first, second) || covered_by(second, first)) return false;
        auto const& a = static_box_detail::borrowed_corner<min_corner>(first);
        auto const& b = static_box_detail::borrowed_corner<max_corner>(first);
        auto const& c = static_box_detail::borrowed_corner<min_corner>(second);
        auto const& d = static_box_detail::borrowed_corner<max_corner>(second);
        return static_box_detail::compare_coordinate<0, Coordinate>(b, c) > 0
            && static_box_detail::compare_coordinate<0, Coordinate>(d, a) > 0
            && static_box_detail::compare_coordinate<1, Coordinate>(b, c) > 0
            && static_box_detail::compare_coordinate<1, Coordinate>(d, a) > 0;
    }
    template <typename First, typename Second>
    static bool touches(First const& first, Second const& second)
    {
        if (disjoint(first, second)) return false;
        auto const& a = static_box_detail::borrowed_corner<min_corner>(first);
        auto const& b = static_box_detail::borrowed_corner<max_corner>(first);
        auto const& c = static_box_detail::borrowed_corner<min_corner>(second);
        auto const& d = static_box_detail::borrowed_corner<max_corner>(second);
        return static_box_detail::compare_coordinate<0, Coordinate>(b, c) == 0
            || static_box_detail::compare_coordinate<0, Coordinate>(d, a) == 0
            || static_box_detail::compare_coordinate<1, Coordinate>(b, c) == 0
            || static_box_detail::compare_coordinate<1, Coordinate>(d, a) == 0;
    }
};

}}}}}

namespace boost { namespace geometry { namespace strategies { namespace detail { namespace partition
{
template <std::size_t Dimension>
struct divide_box_policy<Dimension, exact_cartesian::search_box>
{
    static void apply(exact_cartesian::search_box const& box,
                      exact_cartesian::search_box& lower, exact_cartesian::search_box& upper)
    {
        double const a = geometry::get<min_corner, Dimension>(box);
        double const b = geometry::get<max_corner, Dimension>(box);
        double midpoint;
        if (!std::isfinite(a) && !std::isfinite(b)) midpoint = 0;
        else if (!std::isfinite(a)) midpoint = -(std::numeric_limits<double>::max)();
        else if (!std::isfinite(b)) midpoint = (std::numeric_limits<double>::max)();
        else midpoint = (std::max)(a, (std::min)(b, a / 2 + b / 2));
        lower = upper = box;
        geometry::set<max_corner, Dimension>(lower, midpoint);
        geometry::set<min_corner, Dimension>(upper, midpoint);
    }
};
}}}}}

#endif
