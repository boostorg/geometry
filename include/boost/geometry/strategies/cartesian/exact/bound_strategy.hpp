// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_BOUND_STRATEGY_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_BOUND_STRATEGY_HPP

#include <cmath>
#include <iterator>
#include <type_traits>
#include <tuple>
#include <vector>
#include <boost/geometry/algorithms/for_each.hpp>
#include <boost/geometry/algorithms/num_points.hpp>
#include <boost/geometry/algorithms/detail/overlay/output_conversion.hpp>
#include <boost/geometry/geometries/detail/rebind_geometry.hpp>

#include <boost/geometry/strategies/cartesian/exact/root_bounds.hpp>
#include <boost/geometry/strategies/cartesian/exact/root_box_clipping.hpp>
#include <boost/geometry/strategies/cartesian/exact/geometry_conversion.hpp>
#include <boost/geometry/strategies/cartesian/exact/root_segment_intersection.hpp>
#include <boost/geometry/strategies/cartesian/exact/supported_output.hpp>
#include <boost/geometry/strategies/cartesian/exact/segment.hpp>
#include <boost/geometry/strategies/cartesian/exact/ring_topology_strategy.hpp>
#include <boost/geometry/strategies/cartesian/exact/stable_turn_container.hpp>
#include <boost/geometry/views/detail/segment_as_subrange.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian
{

template <typename Root>
class bound_strategy : public relate::cartesian_exact<typename Root::coordinate_type>
{
    using coordinate = typename Root::coordinate_type;
    using point = point_ref<Root>;
    using context_type = geometry_context<Root>;
    using base = relate::cartesian_exact<coordinate>;
    context_type* m_context = nullptr;
    context_type& context() const { BOOST_ASSERT(m_context); return *m_context; }

    struct topology_type : exact_cartesian::topology<typename Root::coordinate_type>
    {
        template <std::size_t D, typename A, typename B>
        int compare_coordinate(A const& a, B const& b) const
        { return root_compare_coordinate<D, Root>(a, b); }

        template <typename A, typename B>
        auto squared_distance(A const& a, B const& b) const { return root_squared_distance<Root>(a, b); }
        template <typename A, typename B, typename P>
        int side_measure_sign(A const& a, B const& b, P const& p) const
        { return root_orientation<Root>(a, b, p); }
    };

    struct point_distance
    {
        template <typename A, typename B>
        static auto apply(A const& a, B const& b) { return root_squared_distance<Root>(a, b); }
    };

    struct section_type : static_section<typename Root::coordinate_type>
    {
        template <std::size_t D, typename Point, typename Box>
        static bool preceding(int direction, Point const& point, Box const&, Box const& other)
        {
            using coordinate = geometry::coordinate_type_t<Point>;
            if constexpr (std::is_same<Box, search_box>::value
                && fixed_native_supported<coordinate>::value
                && lossless_coordinate_conversion<coordinate, double>::value)
            {
                // A search limit is already a binary64 enclosure endpoint. The
                // native value converts exactly, including next to an infinite limit.
                return (direction == 1 && static_cast<double>(geometry::get<D>(point)) < geometry::get<0, D>(other))
                    || (direction == -1 && static_cast<double>(geometry::get<D>(point)) > geometry::get<1, D>(other));
            }
            else if constexpr (std::is_same<Box, search_box>::value)
            {
                if (direction == 0) return false;
                double const limit = direction == 1 ? geometry::get<0, D>(other) : geometry::get<1, D>(other);
                // An unbounded search enclosure is valid metadata, but infinity
                // is not a homogeneous point and must never enter exact fallback.
                if (std::isinf(limit)) return direction == 1 ? limit > 0 : direction == -1 && limit < 0;
            }
            return (direction == 1 && bounds_detail::with_bound<0, D, Root>(other,
                        [&](auto const& bound) { return root_compare_coordinate<D, Root>(point, bound); }) < 0)
                || (direction == -1 && bounds_detail::with_bound<1, D, Root>(other,
                        [&](auto const& bound) { return root_compare_coordinate<D, Root>(point, bound); }) > 0);
        }
    };

    template <typename Range>
    static original_edge source_support(Range const& range, std::size_t index,
                                        bool reverse, context_type& context)
    {
        auto const& original = working_geometry_detail::conversion_range(range);
        auto const raw = source_geometry_detail::source_edge_index(original, index, reverse);
        if constexpr (working_geometry_detail::is_supported_range<std::decay_t<decltype(original)>>::value)
            return working_geometry_detail::incoming_at(original, (raw + 1) % source_geometry_detail::range_size(original));
        else return {context.register_range(original), raw};
    }

    struct equal_call
    {
        template <typename A, typename B>
        bool operator()(A const& a, B const& b) const { return root_equal<Root>(a, b); }
    };

public:
    template <typename A, typename B>
    bool no_common_boundary_edge(A const& a, B const& b) const
    {
        if constexpr (!(util::is_polygonal<A>::value && util::is_polygonal<B>::value
            && std::is_same<coordinate_type_t<A>, double>::value
            && std::is_same<coordinate_type_t<B>, double>::value)) return false;
        else
        {
            if (geometry::num_points(a, true) > 80 || geometry::num_points(b, true) > 80) return false;
            // Stage C also multiplies the quadratic magnitude by a
            // coefficient >= epsilon^2. The upper budget covers the
            // intermediates of the adaptive sum of the two products.
            constexpr int band = exponent_range::polynomial_band(
                2, 2 * exponent_range::fraction_bits, 8);
            auto safe = [](auto const& p) {
                return exponent_range::contains<band>(get<0>(p))
                    && exponent_range::contains<band>(get<1>(p));
            };
            if (!geometry::all_points_of(a, safe) || !geometry::all_points_of(b, safe)) return false;
            return geometry::all_segments_of(a, [&](auto const& x) {
                return geometry::all_segments_of(b, [&](auto const& y) {
                    auto separated = [](double u, double v, double s, double t) {
                        if (u == v && s == t) return u != s;
                        return (std::max)(u,v) <= (std::min)(s,t) || (std::max)(s,t) <= (std::min)(u,v);
                    };
                    if (separated(get<0>(x.first), get<0>(x.second), get<0>(y.first), get<0>(y.second))
                        || separated(get<1>(x.first), get<1>(x.second), get<1>(y.first), get<1>(y.second)))
                        return true;
                    return native_orientation<double>(x.first, x.second, y.first) != 0
                        || native_orientation<double>(x.first, x.second, y.second) != 0;
                });
            });
        }
    }

    // R-tree's end sentinel default-constructs its stored strategy, but never
    // evaluates it. Actual operations always bind the root context explicitly.
    bound_strategy() = default;
    explicit bound_strategy(context_type& context) : m_context(&context) {}
    // The enclosing geometry_context already owns the precision scope.
    struct operation_scope {};
    static constexpr bool uses_overlay_tolerance = false;
    using supports_direct_crossings = std::true_type;
    using output_policy = typename Root::output_policy;
    using segment_position_type = ratio_free_segment_order;
    template <typename P> using intersection_point_type = point;
    template <typename P> using intersection_result_type = segment_intersection_result<Root>;
    template <typename P> using comparable_distance_type = RootSquaredDistanceValue<Root>;
    template <typename P> using box_type = search_box;
    template <typename P> using geometry_box_type = root_bounds<Root>;
    template <typename G> using ring_measure_type = strategies::detail::topological_ring_tag<point>;
    template <typename G> using overlay_geometry_type = typename geometry::detail::rebind_working<working_geometry_models<Root>, G>::type;
    template <typename G> using overlay_output_type = typename working_output<Root, G>::type;

    template <typename A, typename B, typename Apply>
    decltype(auto) with_operands(A const& a, B const& b, Apply&& apply) const
    { return std::forward<Apply>(apply)(a, b, *this); }

    template <typename Default, bool Local, typename A, typename B>
    auto make_turn_container(A const& a, B const& b) const
    { return stable_turn_container<Root, typename Default::value_type, Local>(context(), a, b); }

    class segment_scope
    {
        intersection_stage<Root>& m_stage;
        std::array<segment_identifier const*, 2> m_previous;
        std::array<bool, 2> m_reverse;
    public:
        segment_scope(intersection_stage<Root>& stage,
                      segment_identifier const& a, segment_identifier const& b,
                      bool reverse_a, bool reverse_b)
            : m_stage(stage), m_previous(stage.segments), m_reverse(stage.reverse)
        {
            stage.segments = {{&a, &b}};
            stage.reverse[a.source_index] = reverse_a;
            stage.reverse[b.source_index] = reverse_b;
        }
        ~segment_scope() { m_stage.segments = m_previous; m_stage.reverse = m_reverse; }
    };
    template <bool Reverse1, bool Reverse2>
    auto segment_pair_scope(segment_identifier const& a, segment_identifier const& b) const
    {
        BOOST_ASSERT(context().active_stage);
        return segment_scope(*context().active_stage, a, b, Reverse1, Reverse2);
    }

    auto relate() const { return root_segment_intersection<Root>(context()); }
    // Do not import the base's static zero-argument overload into this set.
    template <typename A, typename B>
    static auto relate(A const& a, B const& b, void* tag = nullptr)
        -> decltype(base::relate(a, b, tag))
    {
        return base::relate(a, b, tag);
    }
    template <typename Point, typename Box>
    auto clipping(Box const& box) const { return root_box_clipper<Root>(context(), box); }
    template <typename A, typename B>
    static bool disjoint_segments(A const& a, B const& b)
    {
        geometry::detail::segment_as_subrange<A> range1(a);
        geometry::detail::segment_as_subrange<B> range2(b);
        auto const result = segment_intersection<coordinate>::apply
            (range1, range2, policies::relate::segments_direction());
        return result.how == 'd';
    }
    template <typename A, typename B, typename Iterator, typename Converter>
    Iterator intersect_segments(A const& a, B const& b, Iterator output, Converter&& convert) const
    {
        return relate().intersect_original_segments(a, b, output, std::forward<Converter>(convert));
    }
    static auto topology() { return topology_type(); }
    static auto segment_position() { return exact_cartesian::segment_position<coordinate>(); }
    static auto ring_topology() { return exact_cartesian::ring_topology<Root>(); }
    static auto section() { return section_type(); }
    template <typename A, typename B>
    static auto comparable_distance(A const&, B const&) { return point_distance(); }

    template <typename Geometry, typename Box>
    static auto envelope(Geometry const&, Box const&) { return root_envelope<Root>{}; }
    template <typename Box, typename Geometry>
    static auto expand(Box const&, Geometry const&) { return root_expand<Root>{}; }
    template <typename A, typename B>
    static auto within(A const&, B const&, std::enable_if_t<util::is_box<B>::value>* = nullptr)
    { return root_box_predicate<Root, box_relation::within>(); }
    template <typename A, typename B>
    static auto covered_by(A const&, B const&, std::enable_if_t<util::is_box<B>::value>* = nullptr)
    { return root_box_predicate<Root, box_relation::covered_by>(); }
    using base::disjoint;
    template <typename A, typename B>
    static auto disjoint(A const&, B const&,
        std::enable_if_t<util::is_box<A>::value && util::is_box<B>::value>* = nullptr)
    { return root_box_predicate<Root, box_relation::disjoint>(); }
    template <typename A, typename B>
    static auto disjoint(A const&, B const&,
        std::enable_if_t<util::is_segment<A>::value && util::is_box<B>::value>* = nullptr)
    { return root_segment_box<Root>(); }

    template <typename Source, typename Destination>
    void convert_geometry(Source const& source, Destination& destination) const
    {
        if constexpr (std::is_same<Source, Destination>::value) destination = source;
        else geometry_converter<Root>(context()).apply(source, destination);
    }
    template <typename Source, typename Destination>
    void append_output(Source const& source, Destination& destination) const
    {
        geometry::detail::overlay::append_converted_output(source, destination,
            [this](auto const& value, auto& converted, auto const& output)
            {
                if constexpr (!util::is_box<std::decay_t<decltype(output)>>::value)
                    working_geometry_detail::validate_supports<Root>(context(), value);
                convert_geometry(value, converted);
            });
    }

    template <typename Range, typename P>
    static void append_no_duplicates(Range& range, P const& value)
    { append_supported(range, point::borrow(value), exact_cartesian::side<coordinate>(), equal_call(), supported_append_mode::duplicates); }
    template <typename Range, typename P>
    static void append_no_dups_or_spikes(Range& range, P const& value)
    { append_supported(range, point::borrow(value), exact_cartesian::side<coordinate>(), equal_call(), supported_append_mode::spikes); }
    template <typename Range, typename P>
    static void append_no_collinear(Range& range, P const& value)
    { append_supported(range, point::borrow(value), exact_cartesian::side<coordinate>(), equal_call(), supported_append_mode::collinear); }
    template <typename Range>
    static void reverse_range(Range& range)
    { reverse_supported<geometry::closure<Range>::value == closed>(range); }

    template <typename Range>
    static void remove_spikes_at_closure(Range& range)
    {
        while (range.size() >= 4
               && root_orientation<Root>(range[range.size() - 2], range.front(), range[1]) == 0)
        {
            auto const closing = range.incoming.back();
            range.erase(range.begin());
            range.pop_back();
            range.pending = closing;
            range.push_back(range.front());
            range.incoming.front() = closing;
        }
    }
    template <typename Range>
    static void fix_closure(Range& range)
    {
        if constexpr (geometry::closure<Range>::value == open)
            if (!range.empty() && root_equal<Root>(range.front(), range.back()))
            {
                auto const closing = range.incoming.back();
                range.pop_back();
                if (!range.empty()) range.incoming.front() = closing;
            }
    }

    template <bool Reverse1, bool Reverse2, typename Range, typename A, typename B, typename Identifier>
    void prepare_output_edge(Range& output, A const& a, B const& b, Identifier const& id) const
    {
        auto prepare = [&](auto const& geometry, bool reverse)
        {
            source_geometry_detail::selected_range(geometry, id, [&](auto const& range)
            { output.pending = source_support(range, id.segment_index, reverse, context()); });
        };
        if (id.source_index == 0) prepare(a, Reverse1); else prepare(b, Reverse2);
    }
    template <bool Reverse, bool RemoveSpikes, typename G, typename Identifier, typename Range>
    void copy_segments(G const& geometry, Identifier const& id, signed_size_type to, Range& output) const
    {
        source_geometry_detail::selected_range(geometry, id, [&](auto const& range)
        {
            using input = std::decay_t<decltype(range)>;
            constexpr bool circular = std::is_same<geometry::tag_t<input>, ring_tag>::value
                                   || std::is_same<geometry::tag_t<input>, box_tag>::value;
            auto const size = source_geometry_detail::range_size(range) + (circular ? 1 : 0);
            auto const& original = working_geometry_detail::conversion_range(range);
            auto const support_at = [&] {
                if constexpr (working_geometry_detail::is_supported_range<std::decay_t<decltype(original)>>::value)
                    return [&](std::size_t index, bool rev) { return source_support(range, index, rev, context()); };
                else
                    return [&, slot = context().register_range(original)](std::size_t index, bool rev)
                    { return original_edge{slot, source_geometry_detail::source_edge_index(original, index, rev)}; };
            }();
            output.pending = support_at(id.segment_index, Reverse);
            copy_supported<Reverse, RemoveSpikes>(size, circular, id, to, output, exact_cartesian::side<coordinate>(), equal_call(),
                [&](std::size_t index, bool reverse)
                {
                    return context_type::native_vertex(original, source_geometry_detail::source_vertex_index(original, index, reverse));
                },
                support_at);
        });
    }
};

}}}}}

#endif
