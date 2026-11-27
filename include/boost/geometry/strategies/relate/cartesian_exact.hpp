// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Licensed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_GEOMETRY_STRATEGIES_RELATE_CARTESIAN_EXACT_HPP
#define BOOST_GEOMETRY_STRATEGIES_RELATE_CARTESIAN_EXACT_HPP

#include <boost/geometry/strategies/distance/cartesian.hpp>

#include <iterator>
#include <type_traits>
#include <utility>
#include <boost/geometry/strategies/cartesian/exact/areal_equality.hpp>
#include <boost/geometry/strategies/cartesian/exact/box_clipping.hpp>
#include <boost/geometry/strategies/cartesian/detail/segment_extents.hpp>
#include <boost/geometry/strategies/cartesian/exact/filtered_rational.hpp>
#include <boost/geometry/strategies/cartesian/exact/relate_primitives.hpp>
#include <boost/geometry/strategies/cartesian/exact/segment_position.hpp>
#include <boost/geometry/strategies/relate/cartesian.hpp>
#include <boost/geometry/views/detail/iterator_point_view.hpp>

namespace boost { namespace geometry { namespace strategies { namespace relate
{

/*!
\brief Exact Cartesian predicates and implicit straight-segment intersections.
\tparam Coordinate Native input coordinate type: integral, float or double.
\tparam OutputPolicy approximate_output (default), nearest_output, or bounded_ulp_output<N>.
\details Finite binary floating-point inputs denote their exact binary values.
         Input promotion must preserve the complete source coordinate type.
         Constructed points use forward error filters
         with exact fallback. Overlay keeps exact
         working geometries until the complete result is converted to output.
\note Requires two-dimensional Cartesian input, IEEE binary arithmetic and
      round-to-nearest. Floating overflow and underflow are out of scope.
      Supported compiler pragmas disable unsafe expression transformations
      and implicit contraction inside the kernel. On other compilers these
      optimizations must be disabled explicitly. Clang's
      -ffp-contract=fast overrides the pragma; use on, off, or
      fast-honor-pragmas instead. Even for finite input coordinates, filters
      use infinity as a sentinel. Whole-translation-unit fast-math or
      assumptions that values are finite are not supported.
      Long double input/output is not supported. approximate_output emits the
      currently stored approximation without output-only refinement.
      nearest_output rounds root-bound
      intersections to nearest binary64, ties to even, at construction and
      omits stored coordinate error fields. Float output rounds the exact
      coordinate directly to float. approximate_output retains forward error bounds.
      bounded_ulp_output<N> certifies binary64 intersections within N
      representable steps of nearest, ties to even. Root-bound constructions
      omit both error fields and use predicate filters specialized on N.
      Uncertified axes, non-double Coordinate types, and float output use
      nearest rounding for floating constructions. Rational inputs use the
      filtered rational kernel and retain exact coordinates until export.
      Integral output uses BG numeric_cast conversion, which truncates floating
      values toward zero, not the displacement rounding of native integer segment
      construction. No policy guarantees validity after output conversion;
      no topology decision is made on the converted result.
*/
template <typename Coordinate, typename OutputPolicy = approximate_output>
class cartesian_exact : public cartesian<double>
{
    static_assert(std::is_same<OutputPolicy, approximate_output>::value
        || std::is_same<OutputPolicy, nearest_output>::value
        || strategies::detail::exact_cartesian::is_bounded_ulp_output<OutputPolicy>::value,
        "Unknown exact Cartesian output policy");
    using primitives = strategies::detail::exact_cartesian::point_equal<Coordinate>;

public:
    using base_type = cartesian<double>;
    using output_policy = OutputPolicy;
    using base_type::area;
    using base_type::disjoint;

    template <typename A, typename B, typename Apply>
    decltype(auto) with_operands(A const& a, B const& b, Apply&& apply) const;

    template <typename Geometry, typename Box>
    static auto envelope(Geometry const&, Box const&)
    {
        using namespace strategies::detail::exact_cartesian;
        using tag = geometry::tag_t<Geometry>;
        using selected = std::conditional_t<std::is_same<tag, point_tag>::value, static_envelope_point<Coordinate>,
            std::conditional_t<std::is_same<tag, box_tag>::value, static_envelope_box<Coordinate>,
            std::conditional_t<std::is_same<tag, segment_tag>::value, static_envelope_segment<Coordinate>,
            std::conditional_t<std::is_same<tag, multi_point_tag>::value
                || std::is_same<tag, linestring_tag>::value || std::is_same<tag, ring_tag>::value
                || std::is_same<tag, polygon_tag>::value, static_envelope_range<Coordinate>, static_envelope_boxes<Coordinate>>>>>;
        return selected();
    }

    template <typename Box, typename Geometry>
    static auto expand(Box const&, Geometry const&)
    {
        using namespace strategies::detail::exact_cartesian;
        using tag = geometry::tag_t<Geometry>;
        using selected = std::conditional_t<std::is_same<tag, point_tag>::value, static_expand_point<Coordinate>,
            std::conditional_t<std::is_same<tag, segment_tag>::value, static_expand_segment<Coordinate>, static_expand_box<Coordinate>>>;
        return selected();
    }

    template <typename Point, typename Box>
    static auto within(Point const&, Box const&,
        std::enable_if_t<util::is_pointlike<Point>::value && util::is_box<Box>::value>* = nullptr)
    {
        return strategies::detail::exact_cartesian::box_predicate<Coordinate,
            strategies::detail::exact_cartesian::box_relation::within>();
    }

    template <typename Box1, typename Box2>
    static auto within(Box1 const&, Box2 const&,
        std::enable_if_t<util::is_box<Box1>::value && util::is_box<Box2>::value>* = nullptr)
    {
        return strategies::detail::exact_cartesian::box_predicate<Coordinate,
            strategies::detail::exact_cartesian::box_relation::within>();
    }

    template <typename Point, typename Box>
    static auto covered_by(Point const&, Box const&,
        std::enable_if_t<util::is_pointlike<Point>::value && util::is_box<Box>::value>* = nullptr)
    {
        return strategies::detail::exact_cartesian::box_predicate<Coordinate,
            strategies::detail::exact_cartesian::box_relation::covered_by>();
    }

    template <typename Box1, typename Box2>
    static auto covered_by(Box1 const&, Box2 const&,
        std::enable_if_t<util::is_box<Box1>::value && util::is_box<Box2>::value>* = nullptr)
    {
        return strategies::detail::exact_cartesian::box_predicate<Coordinate,
            strategies::detail::exact_cartesian::box_relation::covered_by>();
    }

    template <typename Box1, typename Box2>
    static auto disjoint(Box1 const&, Box2 const&,
        std::enable_if_t<util::is_box<Box1>::value && util::is_box<Box2>::value>* = nullptr)
    {
        return strategies::detail::exact_cartesian::box_predicate<Coordinate,
            strategies::detail::exact_cartesian::box_relation::disjoint>();
    }

    template <typename Segment, typename Box>
    static auto disjoint(Segment const&, Box const&,
        std::enable_if_t<util::is_segment<Segment>::value && util::is_box<Box>::value>* = nullptr)
    {
        return strategies::detail::exact_cartesian::segment_box<Coordinate>();
    }
    template <typename Point>
    using box_type = strategies::detail::exact_cartesian::bounding_box;

    template <typename Point>
    using section_point_type = Point;

    template <typename Iterator>
    using section_iterator_point_type = std::conditional_t<
        strategies::detail::exact_cartesian::fixed_native_supported<
            geometry::coordinate_type_t<typename std::iterator_traits<Iterator>::value_type>>::value,
        geometry::detail::iterator_point_view<Iterator>,
        typename std::iterator_traits<Iterator>::value_type>;

    static constexpr bool exact_predicates = true;
    using exact_cluster_compare = strategies::detail::exact_cartesian::point_compare
        <Coordinate, strategy::compare::less>;

    static auto side() { return strategies::detail::exact_cartesian::side<Coordinate>(); }
    template <typename P, typename Q>
    static auto segment_y_disjoint(P const& a, P const& b, Q const& c, Q const& d)
        -> std::enable_if_t<strategies::detail::native_double_pair<P, Q>::value, bool>
    {
        return strategies::detail::segment_y_disjoint(a, b, c, d);
    }

    template <typename P, typename Q>
    static auto segment_overlap_has_length(P const& a, P const& b, Q const& c, Q const& d)
        -> std::enable_if_t<strategies::detail::native_double_pair<P, Q>::value, bool>
    {
        using strategies::detail::segment_extent_overlap;
        auto const x = segment_extent_overlap<0>(a, b, c, d);
        auto const y = segment_extent_overlap<1>(a, b, c, d);
        return x >= 0 && y >= 0 && (x > 0 || y > 0)
            && !primitives::apply(a, b) && !primitives::apply(c, d)
            && side().apply(a, b, c) == 0 && side().apply(a, b, d) == 0;
    }

    static auto section() { return strategies::detail::exact_cartesian::static_section<Coordinate>(); }
    static auto box_intersection() { return strategies::detail::exact_cartesian::box_intersection<Coordinate, OutputPolicy>(); }
    static auto box_topology() { return strategies::detail::exact_cartesian::box_topology<Coordinate>(); }
    static auto areal_equality() { return strategies::detail::exact_cartesian::areal_equality<Coordinate>(); }
    template <typename A, typename B>
    static auto relate(A const&, B const&,
        std::enable_if_t<util::is_pointlike<A>::value && util::is_pointlike<B>::value>* = nullptr)
    {
        return primitives();
    }

    template <typename A, typename B>
    static auto relate(A const&, B const&,
        std::enable_if_t<util::is_pointlike<A>::value
            && (util::is_linear<B>::value || util::is_polygonal<B>::value)>* = nullptr)
    {
        return strategies::detail::exact_cartesian::winding<Coordinate>();
    }

    template <typename ComparePolicy, typename EqualsPolicy>
    using compare_type = strategies::detail::exact_cartesian::point_compare<Coordinate, ComparePolicy>;

};

namespace services
{
template <typename Coordinate, typename OutputPolicy>
struct predicate_strategy<cartesian_exact<Coordinate, OutputPolicy>>
{
    template <typename A, typename B>
    static decltype(auto) get(cartesian_exact<Coordinate, OutputPolicy> const& strategy,
                              A const&, B const&)
    {
        // Collection relate may pass intermediate overlay output to predicates.
        if constexpr (std::is_same<tag_t<A>, geometry_collection_tag>::value
                   || std::is_same<tag_t<B>, geometry_collection_tag>::value)
            return (strategy);
        else
            return cartesian_exact<Coordinate, approximate_output>{};
    }
};
}

}}}} // namespace boost::geometry::strategies::relate

#include <boost/geometry/strategies/cartesian/exact/bound_strategy.hpp>

namespace boost { namespace geometry { namespace strategies { namespace relate
{
template <typename Coordinate, typename OutputPolicy>
template <typename A, typename B, typename Apply>
decltype(auto) cartesian_exact<Coordinate, OutputPolicy>::with_operands(
    A const& a, B const& b, Apply&& apply) const
{
    namespace exact = strategies::detail::exact_cartesian;
    if constexpr (exact::source_geometry_detail::native_sources<A, B>::value)
    {
        exact::check_coordinate_type<Coordinate, A>();
        exact::check_coordinate_type<Coordinate, B>();
        using root = exact::operation_types<Coordinate, OutputPolicy, A, B>;
        exact::geometry_context<root> context;
        exact::bound_strategy<root> bound(context);
        return std::forward<Apply>(apply)(a, b, bound);
    }
    else return exact::lazy_decimal_detail::apply<std::is_same<OutputPolicy, approximate_output>::value>(
        a, b, std::forward<Apply>(apply));
}
}}}}

#endif
