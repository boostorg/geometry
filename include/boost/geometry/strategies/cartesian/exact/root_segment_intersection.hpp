// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_ROOT_SEGMENT_INTERSECTION_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_ROOT_SEGMENT_INTERSECTION_HPP

#include <boost/geometry/views/detail/segment_as_subrange.hpp>
#include <boost/geometry/policies/relate/intersection_points.hpp>
#include <boost/geometry/strategies/cartesian/exact/operation_geometry.hpp>
#include <boost/geometry/strategies/cartesian/exact/root_predicates.hpp>
#include <boost/geometry/strategies/cartesian/exact/filtered_segment.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian
{

// The existing intersection return object owns only its transient numerical
// seeds. Insertion copies them into stable storage; their supports do not change.
template <typename Root>
struct segment_intersection_result
{
    using point_type = point_ref<Root>;
    // An overlap has two existing endpoints; straight segments construct at
    // most one new crossing point.
    std::array<intersection_record<Root>, 1> definitions;
    std::array<point_type, 2> intersections;
    std::size_t count = 0;

    segment_intersection_result()
        : intersections{{point_type(definitions[0]), point_type(definitions[0])}} {}
    segment_intersection_result(segment_intersection_result const& source)
        : definitions(source.definitions), intersections(source.intersections), count(source.count)
    {
        for (std::size_t i = 0; i < count; ++i)
            if (!source.intersections[i].is_native())
                for (std::size_t j = 0; j < definitions.size(); ++j)
                    if (&source.intersections[i].record() == &source.definitions[j])
                        intersections[i] = point_type(definitions[j]);
    }
    segment_intersection_result(segment_intersection_result&& source)
        : segment_intersection_result(static_cast<segment_intersection_result const&>(source)) {}
    segment_intersection_result& operator=(segment_intersection_result const& source)
    {
        if (this != &source)
        {
            definitions = source.definitions;
            intersections = source.intersections;
            count = source.count;
            for (std::size_t i = 0; i < count; ++i)
                if (!source.intersections[i].is_native())
                    for (std::size_t j = 0; j < definitions.size(); ++j)
                        if (&source.intersections[i].record() == &source.definitions[j])
                            intersections[i] = point_type(definitions[j]);
        }
        return *this;
    }
    segment_intersection_result& operator=(segment_intersection_result&& source)
    { return *this = static_cast<segment_intersection_result const&>(source); }
};

template <typename Root>
class root_segment_intersection
{
    geometry_context<Root>& m_context;
    using point = point_ref<Root>;
    using coordinate = typename Root::coordinate_type;

    struct construction_info
    {
        intersection_stage<Root>& stage;
        int endpoint;

        template <typename Segment>
        void copy_native_support(Segment const& segment, std::size_t index,
                                 coordinate* output) const
        {
            auto copy = [](auto const& point, coordinate* to)
            {
                to[0] = static_cast<coordinate>(geometry::get<0>(point));
                to[1] = static_cast<coordinate>(geometry::get<1>(point));
            };
            // Match the direction of the original support, not the view.
            bool const reverse = stage.reverse[stage.segments[index]->source_index];
            copy(segment.first, output + (reverse ? 2 : 0));
            copy(segment.second, output + (reverse ? 0 : 2));
        }

        template <typename Segment1, typename Segment2>
        void calculate(point& output, Segment1 const& a, Segment2 const& b) const
        {
            switch (endpoint)
            {
            case 0: output = stage.vertex(*stage.segments[0], 0); return;
            case 1: output = stage.vertex(*stage.segments[0], 1); return;
            case 2: output = stage.vertex(*stage.segments[1], 0); return;
            case 3: output = stage.vertex(*stage.segments[1], 1); return;
            default:
                auto& seed = output.record();
                seed.context = &stage.context;
                seed.supports = {{stage.support(*stage.segments[0]), stage.support(*stage.segments[1])}};
                seed.is_seed = true;
                if constexpr (root_predicate_detail::native_operand<geometry::point_type_t<Segment1>>::value
                           && root_predicate_detail::native_operand<geometry::point_type_t<Segment2>>::value)
                {
                    std::array<coordinate, 8> coordinates;
                    copy_native_support(a, 0, coordinates.data());
                    copy_native_support(b, 1, coordinates.data() + 4);
                    BOOST_ASSERT(coordinates == output.producers());
                    seed.numeric.init(coordinates);
                }
                else
                {
                    seed.numeric.init(output.producers());
                }
            }
        }
    };

    struct active_stage_scope
    {
        geometry_context<Root>& context;
        intersection_stage<Root>* previous;

        active_stage_scope(geometry_context<Root>& context, intersection_stage<Root>& stage)
            : context(context), previous(context.active_stage)
        {
            context.active_stage = &stage;
        }

        ~active_stage_scope() { context.active_stage = previous; }
        active_stage_scope(active_stage_scope const&) = delete;
        active_stage_scope& operator=(active_stage_scope const&) = delete;
    };

public:
    using cs_tag = cartesian_tag;
    explicit root_segment_intersection(geometry_context<Root>& context) : m_context(context) {}

    template <typename Segment1, typename Segment2, typename OutputIterator, typename Converter>
    OutputIterator intersect_original_segments(Segment1 const& first, Segment2 const& second,
                                               OutputIterator out, Converter&& convert) const
    {
        static_assert(std::is_same<geometry::tag_t<Segment1>, segment_tag>::value
                   && std::is_same<geometry::tag_t<Segment2>, segment_tag>::value,
                      "Primitive intersection requires original native segments");
        static_assert(!std::is_same<geometry::point_type_t<Segment1>, point>::value
                   && !std::is_same<geometry::point_type_t<Segment2>, point>::value,
                      "Constructed segments require their owning turn stage");

        auto& result = m_context.template retain<segment_intersection_result<Root>>();
        {
            intersection_stage<Root> stage(m_context, first, second);
            segment_identifier const first_id(0, -1, -1, 0), second_id(1, -1, -1, 0);
            stage.segments = {{&first_id, &second_id}};
            active_stage_scope scope(m_context, stage);
            geometry::detail::segment_as_subrange<Segment1> range1(first);
            geometry::detail::segment_as_subrange<Segment2> range2(second);
            using policy = policies::relate::segments_intersection_points<segment_intersection_result<Root>>;
            result = apply(range1, range2, policy());
        }
        for (auto& definition : result.definitions)
            definition.is_seed = false;
        for (std::size_t i = 0; i != result.count; ++i)
        {
            *out++ = convert(result.intersections[i]);
        }
        return out;
    }

    template <typename Range1, typename Range2, typename Policy>
    typename Policy::return_type apply(Range1 const& input_p, Range2 const& input_q, Policy const&) const
    {
        auto* const stage = m_context.active_stage;
        BOOST_ASSERT(stage && stage->segments[0] && stage->segments[1]);
        // Predicates borrow the actual range values. Only retained endpoints
        // resolve source IDs: a native subrange may own temporary corner copies.
        auto const& p0 = input_p.at(0);
        auto const& p1 = input_p.at(1);
        auto const& q0 = input_q.at(0);
        auto const& q1 = input_q.at(1);
#ifndef NDEBUG
        BOOST_ASSERT(root_equal<Root>(stage->vertex(*stage->segments[0], 0), p0));
        BOOST_ASSERT(root_equal<Root>(stage->vertex(*stage->segments[0], 1), p1));
        BOOST_ASSERT(root_equal<Root>(stage->vertex(*stage->segments[1], 0), q0));
        BOOST_ASSERT(root_equal<Root>(stage->vertex(*stage->segments[1], 1), q1));
#endif
        using selected = segment_endpoint_policy<Policy, coordinate>;
        return cartesian_segment_events::classify<selected>(p0, p1, q0, q1,
            [](auto const& a, auto const& b, auto const& p) { return root_orientation<Root>(a, b, p); },
            [](auto const& a, auto const& b) { return root_equal<Root>(a, b); },
            [&](auto const&, auto const&)
            {
                auto const a0 = stage->vertex(*stage->segments[0], 0);
                auto const a1 = stage->vertex(*stage->segments[0], 1);
                auto const b0 = stage->vertex(*stage->segments[1], 0);
                auto const b1 = stage->vertex(*stage->segments[1], 1);
                return collinear_segment_event<selected, coordinate>(
                    model::referring_segment<point const>(a0, a1), model::referring_segment<point const>(b0, b1));
            },
            [&](int endpoint) { return construction_info{*stage, endpoint}; },
            [&](auto const&, int index) { return stage->vertex(*stage->segments[0], index); });
    }
};

}}}}}

#endif
