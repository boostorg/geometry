// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_ROOT_BOUNDS_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_ROOT_BOUNDS_HPP

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <iterator>
#include <type_traits>
#include <utility>
#include <boost/assert.hpp>
#include <boost/iterator/reverse_iterator.hpp>
#include <boost/range.hpp>

#include <boost/geometry/util/detail/precise_fp.hpp>
#include <boost/geometry/views/detail/iterator_point_view.hpp>

BOOST_GEOMETRY_DETAIL_PRECISE_FP_BEGIN

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian
{
template <typename Root> struct root_bounds;
template <typename Root> struct root_envelope;
template <typename Root> struct root_expand;
template <typename Root> struct root_segment_box;
enum class box_relation;
template <typename Root, box_relation Relation> struct root_box_predicate;
namespace bounds_detail
{
template <std::size_t Index, std::size_t D, typename Root, typename Box, typename Apply>
decltype(auto) with_bound(Box const&, Apply&&);
}
}}}}}

#include <boost/geometry/strategies/relate/cartesian_exact.hpp>
#include <boost/geometry/strategies/cartesian/exact/operation_geometry.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian
{

// Coordinate-selection metadata, never a geometry corner or edge support.
template <typename Root>
struct root_bounds
{
    using root_type = Root;
    std::array<point_ref<Root>, 4> origins;
    bool initialized = false;
};

namespace bounds_detail
{
template <typename T> struct is_bounds : std::false_type {};
template <typename Root> struct is_bounds<root_bounds<Root>> : std::true_type {};
template <typename T, typename Tag = geometry::tag_t<T>> struct is_double_box : std::false_type {};
template <typename T> struct is_double_box<T, box_tag>
    : std::is_same<geometry::coordinate_type_t<T>, double> {};
template <typename Root, typename T>
using original_indexed = boost::mp11::mp_contains<typename Root::native_access, source_geometry_detail::indexed_vertex<T>>;
template <typename T> struct is_iterator_view : std::false_type {};
template <typename I> struct is_iterator_view<geometry::detail::iterator_point_view<I>> : std::true_type {};
template <typename T> struct is_reverse_iterator : std::false_type {};
template <typename I> struct is_reverse_iterator<std::reverse_iterator<I>> : std::true_type {};
template <typename I> struct is_reverse_iterator<boost::reverse_iterator<I>> : std::true_type {};

template <typename Root, typename Iterator>
point_ref<Root> from_iterator(Iterator const& iterator)
{
    using point = point_ref<Root>;
    using value = typename std::iterator_traits<Iterator>::value_type;
    if constexpr (std::is_same<value, point>::value
        || boost::mp11::mp_contains<typename Root::native_access, geometry::detail::iterator_point_view<Iterator>>::value)
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

template <typename Root, typename Geometry, typename Apply>
void for_each_point(Geometry const& source, Apply&& apply)
{
    using tag = geometry::tag_t<Geometry>;
    using point = point_ref<Root>;
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
        using first_reference = decltype(static_box_detail::corner<0>(source));
        using second_reference = decltype(static_box_detail::corner<1>(source));
        if constexpr (std::is_lvalue_reference<first_reference>::value && std::is_lvalue_reference<second_reference>::value)
        {
            apply(point::borrow(static_box_detail::corner<0>(source)));
            apply(point::borrow(static_box_detail::corner<1>(source)));
        }
        else
        {
            // A custom indexed original segment need not expose point objects.
            apply(point::borrow(source_geometry_detail::indexed_vertex<Geometry>{&source, 0}));
            apply(point::borrow(source_geometry_detail::indexed_vertex<Geometry>{&source, 1}));
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
            apply(point::borrow(source_geometry_detail::indexed_vertex<Geometry>{&source, 0}));
            apply(point::borrow(source_geometry_detail::indexed_vertex<Geometry>{&source,
                2u}));
        }
    }
    else static_assert(std::is_same<tag, void>::value && !std::is_same<tag, void>::value,
                       "This Root envelope does not yet support dynamic geometry collections");
}

using static_box_detail::initialize;
template <typename Root> void initialize(root_bounds<Root>& box) { box.initialized = false; }

template <typename Root>
void expand_point(root_bounds<Root>& box, point_ref<Root> const& point)
{
    if (!box.initialized) { box.origins.fill(point); box.initialized = true; return; }
    if (root_compare_coordinate<0, Root>(point, box.origins[0]) < 0) box.origins[0] = point;
    if (root_compare_coordinate<1, Root>(point, box.origins[1]) < 0) box.origins[1] = point;
    if (root_compare_coordinate<0, Root>(point, box.origins[2]) > 0) box.origins[2] = point;
    if (root_compare_coordinate<1, Root>(point, box.origins[3]) > 0) box.origins[3] = point;
}

template <typename Root, typename Box, std::enable_if_t<is_double_box<Box>::value, int> = 0>
void expand_point(Box& box, point_ref<Root> const& point)
{
    static_box_detail::merge_bounds::apply(box, static_box_detail::point_enclosure(point));
}

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
template <typename Box, typename Input,
          std::enable_if_t<is_double_box<Box>::value && is_double_box<Input>::value, int> = 0>
void merge(Box& box, Input const& input)
{
    static_box_detail::merge_bounds::apply(box, input);
}

template <typename Root, typename Box, std::enable_if_t<is_double_box<Box>::value, int> = 0>
void merge(Box& box, root_bounds<Root> const& input)
{
    if (!input.initialized) return;
    auto const lx = input.origins[0].approximation(0), ly = input.origins[1].approximation(1);
    auto const ux = input.origins[2].approximation(0), uy = input.origins[3].approximation(1);
    static_box_detail::merge_bounds::apply(box, search_box{
        search_point(static_box_detail::lower(lx), static_box_detail::lower(ly)),
        search_point(static_box_detail::upper(ux), static_box_detail::upper(uy))});
}

template <typename Root, typename Geometry, typename Box>
void expand(Geometry const& input, Box& box)
{
    if constexpr (is_bounds<std::decay_t<Geometry>>::value || std::is_same<Geometry, search_box>::value
        || (is_double_box<Geometry>::value && !original_indexed<Root, Geometry>::value))
        merge(box, input);
    else if constexpr (is_double_box<Box>::value && std::is_same<geometry::tag_t<Geometry>, point_tag>::value
        && fixed_native_supported<geometry::coordinate_type_t<Geometry>>::value)
    {
        static_box_detail::merge_bounds::apply(box, static_box_detail::point_enclosure(input));
    }
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
    scalar_approximation approximation(std::size_t) const { return {value, 0}; }
    scalar_approximation refine(std::size_t d) const { return approximation(d); }
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
    else return apply(static_box_detail::corner_operand<Index>(box));
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
    using common = typename static_predicates_detail::common_root<point_ref<Root>, A, B>::type;
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

} // namespace bounds_detail

template <typename Root>
struct root_expand
{
    template <typename Box, typename Geometry>
    static void apply(Box& box, Geometry const& input) { bounds_detail::expand<Root>(input, box); }
};

struct root_box_accumulation
{
    template <typename Box, typename Input>
    static void start(Box& box, Input const& input)
    { initialize(box); merge(box, input); }

    template <typename Box, typename Input>
    static void merge(Box& box, Input const& input) { bounds_detail::merge(box, input); }

    template <typename Box>
    static void initialize(Box& box) { bounds_detail::initialize(box); }
};

template <typename Root>
struct root_envelope : strategy::envelope::box_accumulation_detail::box_accumulator<root_box_accumulation>
{
    using cs_tag = cartesian_tag;
    using strategy::envelope::box_accumulation_detail::box_accumulator<root_box_accumulation>::apply;
    template <typename Geometry, typename Box,
        std::enable_if_t<!std::is_void<geometry::tag_t<Geometry>>::value, int> = 0>
    static void apply(Geometry const& input, Box& box)
    {
        bounds_detail::initialize(box);
        bounds_detail::expand<Root>(input, box);
    }
    template <typename First, typename Second, typename Box>
    static void apply(First const& first, Second const& second, Box& box)
    { apply(first, box); bounds_detail::expand<Root>(second, box); }
};

template <typename Root>
struct root_box_access
{
    static constexpr bool check_both_corners = false;

    template <typename Box>
    static bool empty(Box const& box)
    {
        if constexpr (bounds_detail::is_bounds<Box>::value)
            return !box.initialized;
        else if constexpr (bounds_detail::is_double_box<Box>::value)
            return geometry::get<min_corner, 0>(box) > geometry::get<max_corner, 0>(box)
                || geometry::get<min_corner, 1>(box) > geometry::get<max_corner, 1>(box);
        else
            return false;
    }

    template <std::size_t D, std::size_t I, typename Point, typename Box>
    static int compare_point(Point const& point, Box const& box)
    {
        return bounds_detail::with_bound<I, D, Root>(box, [&](auto const& bound)
        { return bounds_detail::compare<D, Root>(point, bound); });
    }

    template <std::size_t D, std::size_t I, std::size_t J, typename A, typename B>
    static int compare_bounds(A const& a, B const& b)
    {
        return bounds_detail::with_bound<I, D, Root>(a, [&](auto const& first)
        {
            return bounds_detail::with_bound<J, D, Root>(b, [&](auto const& second)
            { return bounds_detail::compare<D, Root>(first, second); });
        });
    }
};

template <typename Root, box_relation Relation>
struct root_box_predicate : box_relation_predicate<root_box_access<Root>, Relation> {};

template <typename Root>
struct root_segment_box
{
    using cs_tag = cartesian_tag;

    template <typename Segment, typename Box>
    static bool apply(Segment const& segment, Box const& box)
    {
        if (root_box_access<Root>::empty(box) || !ordered<0>(box) || !ordered<1>(box)) return true;
        auto const& a = static_box_detail::corner<0>(segment);
        auto const& b = static_box_detail::corner<1>(segment);
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
        else return root_box_access<Root>::template compare_bounds<D, 0, 1>(box, box) <= 0;
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
        return bounds_detail::with_bound<Index, D, Root>(box,
            [&](auto const& bound) { return bounds_detail::compare<D, Root>(point, bound); });
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
        return with_homogeneous(a, [&](auto const& first)
        {
            return with_homogeneous(b, [&](auto const& second)
            {
                if constexpr (fixed_native_supported<geometry::coordinate_type_t<Box>>::value)
                {
                    // Metadata keeps its actual native grid, including double
                    // search bounds when the Root coordinate is integral.
                    auto const corner = fixed_input_homogeneous(geometry::get<X, 0>(box), geometry::get<Y, 1>(box));
                    return fixed_orientation(first, second, corner);
                }
                else return bounds_detail::with_bound<X, 0, Root>(box, [&](auto const& x)
                {
                    return bounds_detail::with_bound<Y, 1, Root>(box, [&](auto const& y)
                    {
                        return with_homogeneous(x, [&](auto const& hx)
                        {
                            return with_homogeneous(y, [&](auto const& hy)
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

BOOST_GEOMETRY_DETAIL_PRECISE_FP_END

#endif
