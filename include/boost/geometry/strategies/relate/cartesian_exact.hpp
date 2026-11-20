// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Licensed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_GEOMETRY_STRATEGIES_RELATE_CARTESIAN_EXACT_HPP
#define BOOST_GEOMETRY_STRATEGIES_RELATE_CARTESIAN_EXACT_HPP

#include <iterator>
#include <type_traits>
#include <utility>
#include <boost/geometry/strategies/cartesian/exact/box_clipping.hpp>
#include <boost/geometry/strategies/cartesian/exact/geometry_conversion.hpp>
#include <boost/geometry/strategies/cartesian/exact/relate_primitives.hpp>
#include <boost/geometry/strategies/cartesian/exact/segment_position.hpp>
#include <boost/geometry/strategies/cartesian/exact/static_ring_topology.hpp>
#include <boost/geometry/strategies/relate/cartesian.hpp>

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
\note Requires two-dimensional Cartesian input, IEEE binary arithmetic, and no
      fast-math/reassociation. Floating overflow and underflow are out of scope.
      Round-to-nearest and controlled contraction are required.
      Long double input/output is not supported. approximate_output emits the
      currently stored approximation without output-only refinement.
      nearest_output rounds root-bound
      intersections to nearest binary64, ties to even, at construction and
      omits stored coordinate error fields. Float output rounds the exact
      coordinate directly to float. Owning exact points retain their bounds
      and round at floating emission. approximate_output retains forward error bounds.
      bounded_ulp_output<N> certifies binary64 intersections within N
      representable steps of nearest, ties to even. Root-bound constructions
      omit both error fields and use predicate filters specialized on N.
      Uncertified axes, non-double Coordinate types, float output and owning
      exact point conversion use nearest rounding for floating constructions.
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
    using operation_scope = strategies::detail::exact_cartesian::point_cache_scope<Coordinate>;
    using base_type::area;
    using base_type::disjoint;

    template <typename A, typename B, typename Apply>
    decltype(auto) with_sources(A const& a, B const& b, Apply&& apply) const;

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
    using intersection_point_type = strategies::detail::exact_cartesian::exact_point<Coordinate>;

    using segment_position_type = strategies::detail::exact_cartesian::segment_position_tag;

    template <typename Point>
    using comparable_distance_type = strategies::detail::exact_cartesian::squared_distance_value<Coordinate>;

    template <typename Point>
    using box_type = strategies::detail::exact_cartesian::bounding_box;

    template <typename Point>
    using geometry_box_type = model::box<strategies::detail::exact_cartesian::exact_point<Coordinate>>;

    template <typename Point>
    using section_point_type = Point;

    template <typename Iterator>
    using section_iterator_point_type = std::conditional_t<
        strategies::detail::exact_cartesian::fixed_native_supported<
            geometry::coordinate_type_t<typename std::iterator_traits<Iterator>::value_type>>::value,
        strategies::detail::exact_cartesian::iterator_point_view<Iterator>,
        typename std::iterator_traits<Iterator>::value_type>;

    static constexpr bool exact_predicates = true;
    using exact_cluster_compare = strategies::detail::exact_cartesian::point_compare
        <Coordinate, strategy::compare::less>;

    template <typename Point>
    using intersection_result_type = strategies::detail::exact_cartesian::segment_intersection_result
        <intersection_point_type<Point>>;

    template <typename Geometry>
    using overlay_geometry_type = typename strategies::detail::exact_cartesian::working_geometry
        <Coordinate, Geometry>::type;

    // Box failure preserves untouched coordinates; its primitive writes in place.
    template <typename Collection>
    using overlay_output_type = std::conditional_t<util::is_box<Collection>::value,
        Collection, typename strategies::detail::exact_cartesian::working_output
            <Coordinate, Collection>::type>;

    static auto side() { return strategies::detail::exact_cartesian::side<Coordinate>(); }
    static auto topology() { return strategies::detail::exact_cartesian::topology<Coordinate>(); }
    template <typename A, typename B>
    static auto comparable_distance(A const&, B const&)
    { return strategies::detail::exact_cartesian::comparable_squared_distance<Coordinate>(); }
    static auto segment_position() { return strategies::detail::exact_cartesian::segment_position<Coordinate>(); }
    static auto ring_topology() { return strategies::detail::exact_cartesian::ring_topology<Coordinate>(); }
    static auto section() { return strategies::detail::exact_cartesian::static_section<Coordinate>(); }
    static auto box_intersection() { return strategies::detail::exact_cartesian::box_intersection<Coordinate, OutputPolicy>(); }
    static auto box_topology() { return strategies::detail::exact_cartesian::box_topology<Coordinate>(); }
    template <typename Point, typename Box>
    static auto clipping(Box const&) { return strategies::detail::exact_cartesian::clipping<Coordinate, OutputPolicy>(); }
    static auto relate()
    {
        return strategies::detail::exact_cartesian::output_segment_intersection<Coordinate, OutputPolicy>();
    }

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

    template <typename Geometry>
    using ring_measure_type = strategies::detail::exact_cartesian::ring_metadata<Coordinate>;

    template <typename Source, typename Destination>
    static void convert_geometry(Source const& source, Destination& destination,
        std::enable_if_t<!std::is_same<Source, Destination>::value || std::is_array<Source>::value>* = nullptr)
    {
        strategies::detail::exact_cartesian::geometry_converter<Coordinate, geometry::tag_t<Source>, OutputPolicy>
            ::apply(source, destination);
    }

    template <typename Geometry>
    static void convert_geometry(Geometry const& source, Geometry& destination,
        std::enable_if_t<!std::is_array<Geometry>::value>* = nullptr)
    {
        destination = source;
    }

    template <typename Source, typename Destination>
    static void append_output(Source const& source, Destination& destination)
    {
        strategies::detail::exact_cartesian::append_output<Coordinate, OutputPolicy>(source, destination);
    }
};

}}}} // namespace boost::geometry::strategies::relate

#include <boost/geometry/strategies/cartesian/exact/borrowed_strategy.hpp>

namespace boost { namespace geometry { namespace strategies { namespace relate
{
template <typename Coordinate, typename OutputPolicy>
template <typename A, typename B, typename Apply>
decltype(auto) cartesian_exact<Coordinate, OutputPolicy>::with_sources(
    A const& a, B const& b, Apply&& apply) const
{
    namespace exact = strategies::detail::exact_cartesian;
    if constexpr (exact::fixed_native_supported<geometry::coordinate_type_t<A>>::value
                  && exact::fixed_native_supported<geometry::coordinate_type_t<B>>::value)
    {
        exact::check_coordinate_type<Coordinate, A>();
        exact::check_coordinate_type<Coordinate, B>();
        using root = exact::borrowed_root<Coordinate, OutputPolicy, A, B>;
        exact::borrowed_context<root> context(a, b);
        exact::borrowed_strategy<root> bound(context);
        return std::forward<Apply>(apply)(bound);
    }
    else return std::forward<Apply>(apply)(*this);
}
}}}}

#endif
