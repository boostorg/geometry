// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_BORROWED_STRATEGY_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_BORROWED_STRATEGY_HPP

#include <iterator>
#include <cmath>
#include <tuple>
#include <vector>
#include <boost/geometry/algorithms/for_each.hpp>
#include <boost/geometry/algorithms/num_points.hpp>

#include <boost/geometry/strategies/cartesian/exact/borrowed_bounds.hpp>
#include <boost/geometry/strategies/cartesian/exact/borrowed_clipping.hpp>
#include <boost/geometry/strategies/cartesian/exact/borrowed_geometry.hpp>
#include <boost/geometry/strategies/cartesian/exact/borrowed_intersection.hpp>
#include <boost/geometry/strategies/cartesian/exact/borrowed_output.hpp>
#include <boost/geometry/strategies/cartesian/exact/borrowed_output_alias.hpp>
#include <boost/geometry/strategies/cartesian/exact/borrowed_primitive.hpp>
#include <boost/geometry/strategies/cartesian/exact/borrowed_turns.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian
{

template <typename Root>
struct borrowed_topology : topology<typename Root::coordinate_type>
{
    template <typename A, typename B>
    auto squared_distance(A const& a, B const& b) const { return root_squared_distance<Root>(a, b); }
    template <typename A, typename B, typename P>
    auto distance_measure(A const& a, B const& b, P const& p) const
    { return root_distance_measure<Root>(a, b, p); }
};

template <typename Root>
struct borrowed_comparable_distance
{
    template <typename A, typename B>
    static auto apply(A const& a, B const& b) { return root_squared_distance<Root>(a, b); }
};

template <typename Root>
struct borrowed_section : static_section<typename Root::coordinate_type>
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
        return (direction == 1 && borrowed_bounds_detail::with_bound<0, D, Root>(other,
                    [&](auto const& bound) { return root_compare_coordinate<D, Root>(point, bound); }) < 0)
            || (direction == -1 && borrowed_bounds_detail::with_bound<1, D, Root>(other,
                    [&](auto const& bound) { return root_compare_coordinate<D, Root>(point, bound); }) > 0);
    }
};

template <typename Root, int Certified = -1>
class borrowed_strategy : public relate::cartesian_exact<typename Root::coordinate_type>
{
    using coordinate = typename Root::coordinate_type;
    using point = borrowed_point<Root>;
    using context_type = borrowed_context<Root>;
    using base = relate::cartesian_exact<coordinate>;
    context_type* m_context = nullptr;
    context_type& context() const { BOOST_ASSERT(m_context); return *m_context; }

    template <typename Source, typename Destination>
    auto stage_output(Source const& source, Destination const& destination) const
    {
        if constexpr (tuples::is_tuple<Destination>::value)
            return stage_tuple(source, destination, std::make_index_sequence<tuples::size<Destination>::value>());
        else if constexpr (util::is_box<Destination>::value)
        {
            Destination converted;
            convert_geometry(source, converted);
            return converted;
        }
        else
        {
            std::vector<typename boost::range_value<Destination>::type> converted;
            converted.reserve(boost::size(source));
            for (auto const& value : source)
            {
                borrowed_detail::validate_supports<Root>(context(), value);
                convert_geometry(value, converted.emplace_back());
            }
            return converted;
        }
    }
    template <typename Source, typename Destination, std::size_t... I>
    auto stage_tuple(Source const& source, Destination const& destination, std::index_sequence<I...>) const
    { return std::make_tuple(stage_output(tuples::get<I>(source), tuples::get<I>(destination))...); }

    template <typename Staged, typename Destination>
    void publish_output(Staged& staged, Destination& destination) const
    {
        if constexpr (tuples::is_tuple<Destination>::value)
            publish_tuple(staged, destination, std::make_index_sequence<tuples::size<Destination>::value>());
        else if constexpr (util::is_box<Destination>::value)
            geometry::convert(staged, destination);
        else
            for (auto& value : staged) traits::push_back<Destination>::apply(destination, std::move(value));
    }
    template <typename Staged, typename Destination, std::size_t... I>
    void publish_tuple(Staged& staged, Destination& destination, std::index_sequence<I...>) const
    { (publish_output(std::get<I>(staged), tuples::get<I>(destination)), ...); }

    template <typename Range>
    static original_edge source_support(Range const& range, std::size_t index,
                                        bool reverse, context_type& context)
    {
        auto const& original = borrowed_detail::conversion_range(range);
        auto const raw = borrowed_detail::source_edge_index(original, index, reverse);
        if constexpr (borrowed_detail::is_supported_range<std::decay_t<decltype(original)>>::value)
            return borrowed_detail::incoming_at(original, (raw + 1) % borrowed_detail::range_size(original));
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
            // In this band every nonzero difference is at least 2^-180
            // and all orient2d expansion products fit in normal binary64.
            auto safe = [](auto const& p) {
                auto admit = [](double x) { return x == 0 || (std::abs(x) >= 0x1p-128 && std::abs(x) <= 0x1p128); };
                return admit(get<0>(p)) && admit(get<1>(p));
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
    borrowed_strategy() = default;
    explicit borrowed_strategy(context_type& context) : m_context(&context) {}
    // The enclosing borrowed_context already owns the precision scope.
    struct operation_scope {};
    using output_policy = typename Root::output_policy;
    using segment_position_type = borrowed_segment_position_tag;
    template <typename P> using intersection_point_type = point;
    template <typename P> using intersection_result_type = borrowed_intersection_result<Root>;
    template <typename P> using comparable_distance_type = RootSquaredDistanceValue<Root>;
    template <typename P> using box_type = search_box;
    template <typename P> using geometry_box_type = root_bounds<Root>;
    template <typename G> using ring_measure_type = strategies::detail::topological_ring_tag<point>;
    template <typename G> using overlay_geometry_type = typename borrowed_working<Root, G>::type;
    template <typename G> using overlay_output_type = typename borrowed_output_working<Root, G>::type;

    template <typename A, typename B, typename Apply>
    decltype(auto) with_sources(A const&, B const&, Apply&& apply) const
    { return std::forward<Apply>(apply)(*this); }

    template <typename Default, typename A, typename B>
    auto make_turn_container(A const& a, B const& b) const
    { return borrowed_turn_container<Root, typename Default::value_type>(context(), a, b); }

    class model_scope
    {
        borrowed_stage<Root>& m_stage;
        std::array<typename Root::operation_type const*, 2> m_previous;
    public:
        template <bool Reverse1, bool Reverse2, typename Turn>
        model_scope(borrowed_stage<Root>& stage, Turn const& turn,
                    std::integral_constant<bool, Reverse1>, std::integral_constant<bool, Reverse2>)
            : m_stage(stage), m_previous(stage.model)
        {
            stage.model = {{&turn.operations[0], &turn.operations[1]}};
            stage.reverse[stage.model[0]->seg_id.source_index] = Reverse1;
            stage.reverse[stage.model[1]->seg_id.source_index] = Reverse2;
        }
        ~model_scope() { m_stage.model = m_previous; }
    };
    template <bool Reverse1, bool Reverse2, typename Turn>
    auto model_turn_scope(Turn const& turn) const
    {
        BOOST_ASSERT(context().active_stage);
        return model_scope(*context().active_stage, turn, std::integral_constant<bool, Reverse1>{},
                           std::integral_constant<bool, Reverse2>{});
    }

    template <typename Apply>
    void with_turn_strategy(Apply const& apply) const
    {
        if constexpr (std::is_same<coordinate, double>::value)
        {
            if (context().active_stage && context().active_stage->native_differences_exact)
            {
                apply(borrowed_strategy<Root, 1>(context()));
                return;
            }
        }
        apply(borrowed_strategy<Root, 0>(context()));
    }

    auto relate() const { return borrowed_segment_intersection<Root, Certified>(context()); }
    // Do not import the base's static zero-argument overload into this set.
    template <typename A, typename B>
    static auto relate(A const& a, B const& b, void* tag = nullptr)
        -> decltype(base::relate(a, b, tag))
    {
        return base::relate(a, b, tag);
    }
    template <typename Point, typename Box>
    auto clipping(Box const& box) const { return borrowed_clipping<Root>(context(), box); }
    template <typename A, typename B>
    static bool disjoint_segments(A const& a, B const& b)
    { return borrowed_segments_disjoint<Root>(a, b); }
    template <typename A, typename B, typename Iterator, typename Converter>
    Iterator intersect_segments(A const& a, B const& b, Iterator output, Converter&& convert) const
    {
        return borrowed_intersect_segments<Root>(context(), a, b, output, std::forward<Converter>(convert));
    }
    static auto topology() { return borrowed_topology<Root>(); }
    static auto section() { return borrowed_section<Root>(); }
    template <typename A, typename B>
    static auto comparable_distance(A const&, B const&) { return borrowed_comparable_distance<Root>(); }

    template <typename Geometry, typename Box>
    auto envelope(Geometry const&, Box const&) const { return root_envelope<Root>{context()}; }
    template <typename Box, typename Geometry>
    auto expand(Box const&, Geometry const&) const { return root_expand<Root>{context()}; }
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
        else borrowed_geometry_converter<Root>(context()).apply(source, destination);
    }
    template <typename Source, typename Destination>
    void append_output(Source const& source, Destination& destination) const
    {
        if constexpr (!tuples::is_tuple<Destination>::value && !util::is_box<Destination>::value)
        {
            auto const first = boost::begin(source), last = boost::end(source);
            if (first == last) return;
            if (std::next(first) == last)
            {
                borrowed_detail::validate_supports<Root>(context(), *first);
                typename boost::range_value<Destination>::type converted;
                convert_geometry(*first, converted);
                traits::push_back<Destination>::apply(destination, std::move(converted));
                return;
            }
        }
        // The destination can own a borrowed input range, even when it is not
        // the same object as either operand. Finish reading every result (and
        // every tuple member) before a push can reallocate that storage.
        auto staged = stage_output(source, destination);
        publish_output(staged, destination);
    }

    template <typename Source, typename Destination, typename A, typename B>
    void append_output(Source const& source, Destination& destination, A const& a, B const& b) const
    {
        namespace alias = borrowed_output_alias;
        if constexpr (alias::known_root<Destination, A> && alias::known_root<Destination, B>)
        {
            if (alias::disjoint(destination, a) && alias::disjoint(destination, b))
            {
                // These roots own their storage and the destination neither
                // is nor owns either root. No publication can kill a borrower.
                for (auto const& value : source)
                {
                    borrowed_detail::validate_supports<Root>(context(), value);
                    typename boost::range_value<Destination>::type converted;
                    convert_geometry(value, converted);
                    traits::push_back<Destination>::apply(destination, std::move(converted));
                }
                return;
            }
        }
        append_output(source, destination);
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
            borrowed_detail::selected_range(geometry, id, [&](auto const& range)
            { output.pending = source_support(range, id.segment_index, reverse, context()); });
        };
        if (id.source_index == 0) prepare(a, Reverse1); else prepare(b, Reverse2);
    }
    template <bool Reverse, bool RemoveSpikes, typename G, typename Identifier, typename Range>
    void copy_segments(G const& geometry, Identifier const& id, signed_size_type to, Range& output) const
    {
        borrowed_detail::selected_range(geometry, id, [&](auto const& range)
        {
            using input = std::decay_t<decltype(range)>;
            constexpr bool circular = std::is_same<geometry::tag_t<input>, ring_tag>::value
                                   || std::is_same<geometry::tag_t<input>, box_tag>::value;
            auto const size = borrowed_detail::range_size(range) + (circular ? 1 : 0);
            auto const& original = borrowed_detail::conversion_range(range);
            auto const support_at = [&] {
                if constexpr (borrowed_detail::is_supported_range<std::decay_t<decltype(original)>>::value)
                    return [&](std::size_t index, bool rev) { return source_support(range, index, rev, context()); };
                else
                    return [&, slot = context().register_range(original)](std::size_t index, bool rev)
                    { return original_edge{slot, borrowed_detail::source_edge_index(original, index, rev)}; };
            }();
            output.pending = support_at(id.segment_index, Reverse);
            copy_supported<Reverse, RemoveSpikes>(size, circular, id, to, output, exact_cartesian::side<coordinate>(), equal_call(),
                [&](std::size_t index, bool reverse)
                {
                    return context_type::native_vertex(original, borrowed_detail::source_vertex_index(original, index, reverse));
                },
                support_at);
        });
    }
};

}}}}}

#endif
