// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_BORROWED_BOUNDS_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_BORROWED_BOUNDS_HPP

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <exception>
#include <iterator>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <boost/assert.hpp>
#include <boost/iterator/reverse_iterator.hpp>
#include <boost/range.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian
{
template <typename Root> struct root_bounds;
template <typename Root> struct root_envelope;
template <typename Root> struct root_expand;
template <typename Root> struct root_segment_box;
enum class box_relation;
template <typename Root, box_relation Relation> struct root_box_predicate;
namespace borrowed_bounds_detail
{
template <std::size_t Index, std::size_t D, typename Root, typename Box, typename Apply>
decltype(auto) with_bound(Box const&, Apply&&);
}
}}}}}

#include <boost/geometry/strategies/relate/cartesian_exact.hpp>
#include <boost/geometry/strategies/cartesian/exact/borrowed_records.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian
{

// Coordinate-selection metadata, never a geometry corner or edge support.
template <typename Root>
struct root_bounds
{
    using root_type = Root;
    std::array<borrowed_point<Root>, 4> origins;
    bool initialized = false;
};

}}}}}

namespace boost { namespace geometry { namespace traits
{
template <typename Root> struct tag<strategies::detail::exact_cartesian::root_bounds<Root>>
{ using type = box_tag; };
template <typename Root> struct point_type<strategies::detail::exact_cartesian::root_bounds<Root>>
{ using type = strategies::detail::exact_cartesian::borrowed_point<Root>; };
template <typename Root, std::size_t Index, std::size_t D>
struct indexed_access<strategies::detail::exact_cartesian::root_bounds<Root>, Index, D>
{
    using box = strategies::detail::exact_cartesian::root_bounds<Root>;
    static auto get(box const& value)
    {
        static_assert(Index < 2 && D < 2, "Two-dimensional Root bounds required");
        BOOST_ASSERT(value.initialized);
        if (!value.initialized) throw std::logic_error("An empty Root envelope has no coordinate origins");
        return value.origins[2 * Index + D].exact_coordinate(D);
    }
    static void set(box&, strategies::detail::exact_cartesian::exact_number const&)
    { BOOST_ASSERT_MSG(false, "Root bounds retain coordinate origins, not scalar assignments"); std::terminate(); }
};
}}}

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian
{

namespace borrowed_bounds_detail
{
template <typename T> struct is_bounds : std::false_type {};
template <typename Root> struct is_bounds<root_bounds<Root>> : std::true_type {};
template <typename T, typename Tag = geometry::tag_t<T>> struct is_double_box : std::false_type {};
template <typename T> struct is_double_box<T, box_tag>
    : std::is_same<geometry::coordinate_type_t<T>, double> {};
template <typename Root, typename T>
using original_indexed = boost::mp11::mp_contains<typename Root::native_access, borrowed_detail::indexed_vertex<T>>;
template <typename T> struct is_iterator_view : std::false_type {};
template <typename I> struct is_iterator_view<iterator_point_view<I>> : std::true_type {};
template <typename T> struct is_reverse_iterator : std::false_type {};
template <typename I> struct is_reverse_iterator<std::reverse_iterator<I>> : std::true_type {};
template <typename I> struct is_reverse_iterator<boost::reverse_iterator<I>> : std::true_type {};

template <typename Root, typename Iterator>
borrowed_point<Root> from_iterator(Iterator const& iterator)
{
    using point = borrowed_point<Root>;
    using value = typename std::iterator_traits<Iterator>::value_type;
    if constexpr (std::is_same<value, point>::value
        || boost::mp11::mp_contains<typename Root::native_access, iterator_point_view<Iterator>>::value)
        return point::from_iterator(iterator);
    else if constexpr (is_reverse_iterator<Iterator>::value)
    {
        // Envelope's clockwise view may reverse an original native ring. Keep
        // the corresponding original iterator, not an accessor into that view.
        auto original = iterator.base();
        --original;
        return from_iterator<Root>(original);
    }
    else return point::from_iterator(iterator);
}

template <std::size_t Index, typename Segment>
auto segment_endpoint(Segment const& segment, int)
    -> std::enable_if_t<std::is_pointer<decltype(segment.first)>::value, decltype(*segment.first)>
{
    BOOST_ASSERT(segment.first && segment.second);
    return Index == 0 ? *segment.first : *segment.second;
}
template <std::size_t Index, typename Segment>
decltype(auto) segment_endpoint(Segment const& segment, long)
{ return static_box_detail::corner<Index>(segment); }

template <typename Root, typename Geometry, typename Apply>
void for_each_point(Geometry const& source, Apply&& apply)
{
    using tag = geometry::tag_t<Geometry>;
    using point = borrowed_point<Root>;
    if constexpr (std::is_same<tag, point_tag>::value)
    {
        if constexpr (is_iterator_view<Geometry>::value)
            apply(from_iterator<Root>(source.base()));
        else
        {
            static_assert(std::is_same<Geometry, point>::value
                || fixed_native_supported<geometry::coordinate_type_t<Geometry>>::value,
                          "Only geometry points can supply Root bound origins");
            apply(point::borrow(source));
        }
    }
    else if constexpr (std::is_same<tag, linestring_tag>::value || std::is_same<tag, ring_tag>::value
                       || std::is_same<tag, multi_point_tag>::value)
    {
        for (auto it = boost::begin(source); it != boost::end(source); ++it)
            apply(from_iterator<Root>(it));
    }
    else if constexpr (std::is_same<tag, polygon_tag>::value)
    {
        static_assert(std::is_lvalue_reference<decltype(geometry::exterior_ring(source))>::value,
                      "Root bounds cannot retain origins in a temporary exterior ring");
        for_each_point<Root>(geometry::exterior_ring(source), std::forward<Apply>(apply));
    }
    else if constexpr (std::is_same<tag, multi_polygon_tag>::value || std::is_same<tag, multi_linestring_tag>::value)
    {
        static_assert(std::is_lvalue_reference<decltype(*boost::begin(source))>::value,
                      "Root bounds require stable multi-geometry elements");
        for (auto const& geometry : source) for_each_point<Root>(geometry, apply);
    }
    else if constexpr (std::is_same<tag, segment_tag>::value)
    {
        using first_reference = decltype(segment_endpoint<0>(source, 0));
        using second_reference = decltype(segment_endpoint<1>(source, 0));
        if constexpr (std::is_lvalue_reference<first_reference>::value && std::is_lvalue_reference<second_reference>::value)
        {
            apply(point::borrow(segment_endpoint<0>(source, 0)));
            apply(point::borrow(segment_endpoint<1>(source, 0)));
        }
        else
        {
            // A custom indexed original segment need not expose point objects.
            apply(point::borrow(borrowed_detail::indexed_vertex<Geometry>{&source, 0}));
            apply(point::borrow(borrowed_detail::indexed_vertex<Geometry>{&source, 1}));
        }
    }
    else if constexpr (std::is_same<tag, box_tag>::value)
    {
        using input_point = geometry::point_type_t<Geometry>;
        if constexpr (!std::is_void<typename static_predicates_detail::root_of<input_point>::type>::value)
        {
            auto append = [&](auto const& first, auto const& second)
            {
                static_assert(std::is_same<std::decay_t<decltype(first)>, point>::value
                    && std::is_same<std::decay_t<decltype(second)>, point>::value,
                    "Root segments/boxes must expose their actual point references");
                apply(first); apply(second);
            };
            append(static_box_detail::corner<0>(source), static_box_detail::corner<1>(source));
        }
        else
        {
            apply(point::borrow(borrowed_detail::indexed_vertex<Geometry>{&source, 0}));
            apply(point::borrow(borrowed_detail::indexed_vertex<Geometry>{&source,
                2u}));
        }
    }
    else static_assert(std::is_same<tag, void>::value && !std::is_same<tag, void>::value,
                       "This Root envelope does not yet support dynamic geometry collections");
}

inline void initialize(search_box& box) { static_box_detail::initialize(box); }
template <typename Root> void initialize(root_bounds<Root>& box) { box.initialized = false; }
template <typename A, typename B>
void copy_double_bounds(A const& a, B& b)
{
    geometry::set<0, 0>(b, geometry::get<0, 0>(a));
    geometry::set<0, 1>(b, geometry::get<0, 1>(a));
    geometry::set<1, 0>(b, geometry::get<1, 0>(a));
    geometry::set<1, 1>(b, geometry::get<1, 1>(a));
}
template <typename Box, std::enable_if_t<is_double_box<Box>::value
    && !std::is_same<Box, search_box>::value, int> = 0>
void initialize(Box& box)
{
    search_box inverse;
    initialize(inverse);
    copy_double_bounds(inverse, box);
}

template <typename Root>
void expand_point(root_bounds<Root>& box, borrowed_point<Root> const& point)
{
    if (!box.initialized) { box.origins.fill(point); box.initialized = true; return; }
    if (root_compare_coordinate<0, Root>(point, box.origins[0]) < 0) box.origins[0] = point;
    if (root_compare_coordinate<1, Root>(point, box.origins[1]) < 0) box.origins[1] = point;
    if (root_compare_coordinate<0, Root>(point, box.origins[2]) > 0) box.origins[2] = point;
    if (root_compare_coordinate<1, Root>(point, box.origins[3]) > 0) box.origins[3] = point;
}

template <typename Root>
void expand_point(search_box& box, borrowed_point<Root> const& point)
{ static_box_detail::expand<typename Root::coordinate_type>(box, point); }

template <typename Root>
void merge(root_bounds<Root>& box, root_bounds<Root> const& input)
{
    if (!input.initialized) return;
    if (!box.initialized) { box = input; return; }
    if (root_compare_coordinate<0, Root>(input.origins[0], box.origins[0]) < 0) box.origins[0] = input.origins[0];
    if (root_compare_coordinate<1, Root>(input.origins[1], box.origins[1]) < 0) box.origins[1] = input.origins[1];
    if (root_compare_coordinate<0, Root>(input.origins[2], box.origins[2]) > 0) box.origins[2] = input.origins[2];
    if (root_compare_coordinate<1, Root>(input.origins[3], box.origins[3]) > 0) box.origins[3] = input.origins[3];
}
inline void merge(search_box& box, search_box const& input)
{ static_expand_box<double>::apply(box, input); }
template <typename Input, std::enable_if_t<is_double_box<Input>::value
    && !std::is_same<Input, search_box>::value, int> = 0>
void merge(search_box& box, Input const& input)
{ static_expand_box<double>::apply(box, input); }

template <typename Root>
void merge(search_box& box, root_bounds<Root> const& input)
{
    if (!input.initialized) return;
    auto const lx = input.origins[0].approximation(0), ly = input.origins[1].approximation(1);
    auto const ux = input.origins[2].approximation(0), uy = input.origins[3].approximation(1);
    static_box_detail::expand_bounds<0>(box, static_box_detail::lower(lx), static_box_detail::upper(ux));
    static_box_detail::expand_bounds<1>(box, static_box_detail::lower(ly), static_box_detail::upper(uy));
}

template <typename Box, typename Input, std::enable_if_t<is_double_box<Box>::value
    && !std::is_same<Box, search_box>::value, int> = 0>
void merge(Box& box, Input const& input)
{
    search_box current;
    copy_double_bounds(box, current);
    merge(current, input);
    copy_double_bounds(current, box);
}

template <typename Root, typename Geometry, typename Box>
void expand(Geometry const& input, Box& box)
{
    if constexpr (is_double_box<Box>::value && !std::is_same<Box, search_box>::value)
    {
        search_box current;
        copy_double_bounds(box, current);
        expand<Root>(input, current);
        copy_double_bounds(current, box);
    }
    else if constexpr (is_bounds<std::decay_t<Geometry>>::value || std::is_same<Geometry, search_box>::value
        || (is_double_box<Geometry>::value && !original_indexed<Root, Geometry>::value))
        merge(box, input);
    else if constexpr (std::is_same<Box, search_box>::value && std::is_same<geometry::tag_t<Geometry>, point_tag>::value
        && fixed_native_supported<geometry::coordinate_type_t<Geometry>>::value)
        static_box_detail::expand<typename Root::coordinate_type>(box, input);
    else for_each_point<Root>(input, [&](auto const& point) { expand_point(box, point); });
}

// A finite search coordinate is exact binary64 metadata, not a native Root
// leaf. Keep its own grid when Root C is integral; never cast it into Root C.
template <typename Root>
struct search_coordinate
{
    using root_type = Root;
    double value;
    double approximate(std::size_t) const { return value; }
    interval approximation(std::size_t) const { return {value, 0}; }
    interval refine(std::size_t d) const { return approximation(d); }
    auto homogeneous() const { return fixed_input_homogeneous(value, value); }
};

template <std::size_t Index, std::size_t D, typename Root, typename Box, typename Apply>
decltype(auto) with_bound(Box const& box, Apply&& apply)
{
    if constexpr (is_bounds<std::decay_t<Box>>::value)
    {
        static_assert(std::is_same<Box, root_bounds<Root>>::value, "Bounds must share their Root");
        BOOST_ASSERT(box.initialized);
        return apply(box.origins[2 * Index + D]);
    }
    else if constexpr (std::is_same<Box, search_box>::value
        || (is_double_box<Box>::value && !original_indexed<Root, Box>::value))
        return apply(search_coordinate<Root>{geometry::get<Index, D>(box)});
    else return apply(static_box_detail::borrowed_corner<Index>(box));
}

template <typename Root, typename Point>
int infinity(Point const& point)
{
    if constexpr (std::is_same<Point, search_coordinate<Root>>::value)
        return std::isinf(point.value) ? (point.value < 0 ? -1 : 1) : 0;
    else return 0;
}

template <std::size_t D, typename Root, typename A, typename B>
int compare(A const& a, B const& b)
{
    using common = typename static_predicates_detail::common_root<borrowed_point<Root>, A, B>::type;
    static_assert(std::is_same<common, Root>::value, "Bound operands must share their Root");
    if constexpr (std::is_same<A, search_coordinate<Root>>::value
                  && std::is_same<B, search_coordinate<Root>>::value)
    {
        // These are the stored search limits, not approximate geometry
        // coordinates. Direct ordering also covers the unbounded limits.
        return (a.value > b.value) - (a.value < b.value);
    }
    int const ai = infinity<Root>(a), bi = infinity<Root>(b);
    if (ai || bi) return (ai > bi) - (ai < bi);
    return root_compare_coordinate<D, Root>(a, b);
}

template <std::size_t D, std::size_t I, std::size_t J, typename Root, typename A, typename B>
int compare_bounds(A const& a, B const& b)
{
    return with_bound<I, D, Root>(a, [&](auto const& first)
    { return with_bound<J, D, Root>(b, [&](auto const& second) { return compare<D, Root>(first, second); }); });
}
template <typename Box> bool empty(Box const& box)
{
    if constexpr (is_double_box<Box>::value)
        return geometry::get<min_corner, 0>(box) > geometry::get<max_corner, 0>(box)
            || geometry::get<min_corner, 1>(box) > geometry::get<max_corner, 1>(box);
    else return false;
}
template <typename Root> bool empty(root_bounds<Root> const& box) { return !box.initialized; }
inline bool empty(search_box const& box)
{
    return geometry::get<min_corner, 0>(box) > geometry::get<max_corner, 0>(box)
        || geometry::get<min_corner, 1>(box) > geometry::get<max_corner, 1>(box);
}
} // namespace borrowed_bounds_detail

template <typename Root>
struct root_expand
{
    borrowed_context<Root>& context;
    template <typename Box, typename Geometry>
    static void apply(Box& box, Geometry const& input) { borrowed_bounds_detail::expand<Root>(input, box); }
};

template <typename Root>
struct root_envelope
{
    using cs_tag = cartesian_tag;
    borrowed_context<Root>& context;
    template <typename Geometry, typename Box,
        std::enable_if_t<!std::is_void<geometry::tag_t<Geometry>>::value, int> = 0>
    static void apply(Geometry const& input, Box& box)
    {
        borrowed_bounds_detail::initialize(box);
        borrowed_bounds_detail::expand<Root>(input, box);
    }
    template <typename First, typename Second, typename Box>
    static void apply(First const& first, Second const& second, Box& box)
    { apply(first, box); borrowed_bounds_detail::expand<Root>(second, box); }
    template <typename Box> struct state { Box box; bool initialized = false; };
    template <typename Box, typename Input>
    static void apply(state<Box>& current, Input const& input)
    {
        if (!current.initialized) { borrowed_bounds_detail::initialize(current.box); current.initialized = true; }
        borrowed_bounds_detail::merge(current.box, input);
    }
    template <typename Box>
    static void result(state<Box> const& current, Box& box)
    {
        if (current.initialized) box = current.box;
        else borrowed_bounds_detail::initialize(box);
    }
};

template <typename Root, box_relation Relation>
struct root_box_predicate
{
    using cs_tag = cartesian_tag;
private:
    template <std::size_t D, typename Point, typename Box>
    static bool point_axis(Point const& point, Box const& box, bool strict)
    {
        int const low = borrowed_bounds_detail::with_bound<0, D, Root>(box,
            [&](auto const& bound) { return borrowed_bounds_detail::compare<D, Root>(point, bound); });
        int const high = borrowed_bounds_detail::with_bound<1, D, Root>(box,
            [&](auto const& bound) { return borrowed_bounds_detail::compare<D, Root>(point, bound); });
        return strict ? low > 0 && high < 0 : low >= 0 && high <= 0;
    }
    template <std::size_t D, typename A, typename B>
    static bool axis(A const& a, B const& b)
    {
        int const lower = borrowed_bounds_detail::compare_bounds<D, 0, 0, Root>(a, b);
        int const upper = borrowed_bounds_detail::compare_bounds<D, 1, 1, Root>(a, b);
        return lower >= 0 && upper <= 0;
    }
    template <typename A, typename B>
    static bool covered(A const& a, B const& b) { return axis<0>(a, b) && axis<1>(a, b); }
    template <typename A, typename B>
    static bool disjoint(A const& a, B const& b)
    {
        return borrowed_bounds_detail::compare_bounds<0, 1, 0, Root>(a, b) < 0
            || borrowed_bounds_detail::compare_bounds<0, 1, 0, Root>(b, a) < 0
            || borrowed_bounds_detail::compare_bounds<1, 1, 0, Root>(a, b) < 0
            || borrowed_bounds_detail::compare_bounds<1, 1, 0, Root>(b, a) < 0;
    }
public:
    template <typename A, typename B>
    static bool apply(A const& a, B const& b)
    {
        if (borrowed_bounds_detail::empty(a) || borrowed_bounds_detail::empty(b))
            return Relation == box_relation::disjoint;
        if constexpr (std::is_same<geometry::tag_t<A>, point_tag>::value)
        {
            bool const covered = point_axis<0>(a, b, false) && point_axis<1>(a, b, false);
            bool const strict = point_axis<0>(a, b, true) && point_axis<1>(a, b, true);
            if constexpr (Relation == box_relation::within) return strict;
            if constexpr (Relation == box_relation::covered_by) return covered;
            if constexpr (Relation == box_relation::disjoint) return !covered;
            if constexpr (Relation == box_relation::touches) return covered && !strict;
            return false;
        }
        else
        {
            if constexpr (Relation == box_relation::covered_by) return covered(a, b);
            if constexpr (Relation == box_relation::within)
                return covered(a, b) && borrowed_bounds_detail::compare_bounds<0, 0, 1, Root>(a, a) < 0
                    && borrowed_bounds_detail::compare_bounds<1, 0, 1, Root>(a, a) < 0;
            if constexpr (Relation == box_relation::disjoint) return disjoint(a, b);
            if constexpr (Relation == box_relation::touches)
                return !disjoint(a, b) && (borrowed_bounds_detail::compare_bounds<0, 1, 0, Root>(a, b) == 0
                    || borrowed_bounds_detail::compare_bounds<0, 1, 0, Root>(b, a) == 0
                    || borrowed_bounds_detail::compare_bounds<1, 1, 0, Root>(a, b) == 0
                    || borrowed_bounds_detail::compare_bounds<1, 1, 0, Root>(b, a) == 0);
            if constexpr (Relation == box_relation::overlaps)
                return !covered(a, b) && !covered(b, a)
                    && borrowed_bounds_detail::compare_bounds<0, 1, 0, Root>(a, b) > 0
                    && borrowed_bounds_detail::compare_bounds<0, 1, 0, Root>(b, a) > 0
                    && borrowed_bounds_detail::compare_bounds<1, 1, 0, Root>(a, b) > 0
                    && borrowed_bounds_detail::compare_bounds<1, 1, 0, Root>(b, a) > 0;
            return false;
        }
    }
};

template <typename Root>
struct root_segment_box
{
    using cs_tag = cartesian_tag;
    static auto get_disjoint_point_box_strategy()
    { return root_box_predicate<Root, box_relation::covered_by>(); }

    template <typename Segment, typename Box>
    static bool apply(Segment const& segment, Box const& box)
    {
        if (borrowed_bounds_detail::empty(box) || !ordered<0>(box) || !ordered<1>(box)) return true;
        auto const& a = borrowed_bounds_detail::segment_endpoint<0>(segment, 0);
        auto const& b = borrowed_bounds_detail::segment_endpoint<1>(segment, 0);
        if (separated<0>(a, b, box) || separated<1>(a, b, box)) return true;
        if (unbounded(box))
        {
            int const dx = root_compare_coordinate<0, Root>(b, a);
            int const dy = root_compare_coordinate<1, Root>(b, a);
            if (dx == 0 || dy == 0) return false;
            if (dy > 0) return dx > 0 ? unbounded_disjoint<1, 0>(a, b, box)
                                       : unbounded_disjoint<1, 1>(a, b, box);
            return dx > 0 ? unbounded_disjoint<0, 0>(a, b, box)
                          : unbounded_disjoint<0, 1>(a, b, box);
        }
        int const first = side<0, 0>(a, b, box);
        return first != 0 && side<1, 0>(a, b, box) == first
            && side<1, 1>(a, b, box) == first && side<0, 1>(a, b, box) == first;
    }

private:
    template <std::size_t D, typename Box>
    static bool ordered(Box const& box)
    {
        if constexpr (fixed_native_supported<geometry::coordinate_type_t<Box>>::value)
            return geometry::get<0, D>(box) <= geometry::get<1, D>(box);
        else return borrowed_bounds_detail::compare_bounds<D, 0, 1, Root>(box, box) <= 0;
    }
    template <std::size_t Index, std::size_t D, typename Box>
    static int infinite(Box const& box)
    {
        if constexpr (std::is_floating_point<geometry::coordinate_type_t<Box>>::value)
        {
            auto const value = geometry::get<Index, D>(box);
            return std::isinf(value) ? (value < 0 ? -1 : 1) : 0;
        }
        else return 0;
    }
    template <typename Box>
    static bool unbounded(Box const& box)
    { return infinite<0, 0>(box) || infinite<0, 1>(box) || infinite<1, 0>(box) || infinite<1, 1>(box); }

    template <std::size_t Index, std::size_t D, typename Point, typename Box>
    static int point_bound(Point const& point, Box const& box)
    {
        int const inf = infinite<Index, D>(box);
        if (inf) return -inf;
        return borrowed_bounds_detail::with_bound<Index, D, Root>(box,
            [&](auto const& bound) { return borrowed_bounds_detail::compare<D, Root>(point, bound); });
    }
    template <std::size_t D, typename A, typename B, typename Box>
    static bool separated(A const& a, B const& b, Box const& box)
    {
        return (point_bound<0, D>(a, box) < 0 && point_bound<0, D>(b, box) < 0)
            || (point_bound<1, D>(a, box) > 0 && point_bound<1, D>(b, box) > 0);
    }

    template <std::size_t X, std::size_t Y, typename A, typename B, typename Box>
    static int side(A const& a, B const& b, Box const& box)
    {
        return static_predicates_detail::with_homogeneous(a, [&](auto const& first)
        {
            return static_predicates_detail::with_homogeneous(b, [&](auto const& second)
            {
                if constexpr (fixed_native_supported<geometry::coordinate_type_t<Box>>::value)
                {
                    // Metadata keeps its actual native grid, including double
                    // search bounds when the Root coordinate is integral.
                    auto const corner = fixed_input_homogeneous(geometry::get<X, 0>(box), geometry::get<Y, 1>(box));
                    return fixed_orientation(first, second, corner);
                }
                else return borrowed_bounds_detail::with_bound<X, 0, Root>(box, [&](auto const& x)
                {
                    return borrowed_bounds_detail::with_bound<Y, 1, Root>(box, [&](auto const& y)
                    {
                        return static_predicates_detail::with_homogeneous(x, [&](auto const& hx)
                        {
                            return static_predicates_detail::with_homogeneous(y, [&](auto const& hy)
                            { return fixed_orientation(first, second, fixed_splice(hx, 0, hy, 1)); });
                        });
                    });
                });
            });
        });
    }
    template <std::size_t X, std::size_t Y, typename A, typename B, typename Box>
    static int extreme_side(A const& a, B const& b, Box const& box, int infinite_sign)
    { return infinite<X, 0>(box) || infinite<Y, 1>(box) ? infinite_sign : side<X, Y>(a, b, box); }
    template <std::size_t MinX, std::size_t MinY, typename A, typename B, typename Box>
    static bool unbounded_disjoint(A const& a, B const& b, Box const& box)
    { return extreme_side<MinX, MinY>(a, b, box, -1) > 0 || extreme_side<1 - MinX, 1 - MinY>(a, b, box, 1) < 0; }
};

}}}}} // namespace boost::geometry::strategies::detail::exact_cartesian

#endif
