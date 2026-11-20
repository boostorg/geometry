// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_BORROWED_INTERSECTION_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_BORROWED_INTERSECTION_HPP

#include <boost/geometry/strategies/cartesian/exact/borrowed_records.hpp>
#include <boost/geometry/strategies/cartesian/exact/borrowed_predicates.hpp>
#include <boost/geometry/strategies/cartesian/exact/filtered_segment.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian
{

// The existing intersection return object owns only its transient numerical
// seeds. Insertion binds them to the actual retained turn and its operation IDs.
template <typename Root>
struct borrowed_intersection_result
{
    using point_type = borrowed_point<Root>;
    // An overlap has two existing endpoints; straight segments construct at
    // most one new crossing point.
    std::array<borrowed_record<Root>, 1> definitions;
    std::array<point_type, 2> intersections;
    std::size_t count = 0;

    borrowed_intersection_result()
        : intersections{{point_type(definitions[0]), point_type(definitions[0])}} {}
    borrowed_intersection_result(borrowed_intersection_result const& source)
        : definitions(source.definitions), intersections(source.intersections), count(source.count)
    {
        for (std::size_t i = 0; i < count; ++i)
            if (!source.intersections[i].is_native())
                for (std::size_t j = 0; j < definitions.size(); ++j)
                    if (&source.intersections[i].record() == &source.definitions[j])
                        intersections[i] = point_type(definitions[j]);
    }
    borrowed_intersection_result(borrowed_intersection_result&& source)
        : borrowed_intersection_result(static_cast<borrowed_intersection_result const&>(source)) {}
    borrowed_intersection_result& operator=(borrowed_intersection_result const& source)
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
    borrowed_intersection_result& operator=(borrowed_intersection_result&& source)
    { return *this = static_cast<borrowed_intersection_result const&>(source); }
};

template <typename Root, int Certified = -1>
class borrowed_segment_intersection
{
    borrowed_context<Root>& m_context;
    using point = borrowed_point<Root>;
    using coordinate = typename Root::coordinate_type;

    struct range
    {
        point first, second;
        point const& at(std::size_t i) const { return i == 0 ? first : second; }
    };

    struct construction_info
    {
        borrowed_stage<Root>& stage;
        int endpoint;

        template <typename Segment1, typename Segment2>
        void calculate(point& output, Segment1 const&, Segment2 const&) const
        {
            switch (endpoint)
            {
            case 0: output = stage.vertex(stage.model[0]->seg_id, 0); return;
            case 1: output = stage.vertex(stage.model[0]->seg_id, 1); return;
            case 2: output = stage.vertex(stage.model[1]->seg_id, 0); return;
            case 3: output = stage.vertex(stage.model[1]->seg_id, 1); return;
            default:
                auto& seed = output.record();
                seed.stage = &stage;
                seed.operations = stage.model;
                seed.is_seed = true;
                seed.numeric.init(output.producers());
            }
        }
    };

public:
    using cs_tag = cartesian_tag;
    explicit borrowed_segment_intersection(borrowed_context<Root>& context) : m_context(context) {}

    template <typename Range1, typename Range2, typename Policy>
    typename Policy::return_type apply(Range1 const& input_p, Range2 const& input_q, Policy const& policy) const
    {
        auto* const stage = m_context.active_stage;
        BOOST_ASSERT(stage && stage->model[0] && stage->model[1]);
        // Predicates borrow the actual range values. Only retained endpoints
        // resolve model IDs: a native subrange may own temporary corner copies.
        auto const& p0 = input_p.at(0);
        auto const& p1 = input_p.at(1);
        auto const& q0 = input_q.at(0);
        auto const& q1 = input_q.at(1);
#ifndef NDEBUG
        BOOST_ASSERT(root_equal<Root>(stage->vertex(stage->model[0]->seg_id, 0), p0));
        BOOST_ASSERT(root_equal<Root>(stage->vertex(stage->model[0]->seg_id, 1), p1));
        BOOST_ASSERT(root_equal<Root>(stage->vertex(stage->model[1]->seg_id, 0), q0));
        BOOST_ASSERT(root_equal<Root>(stage->vertex(stage->model[1]->seg_id, 1), q1));
#endif
        using selected = filtered_segment_detail::endpoint_policy<Policy, coordinate>;
        auto const exact_differences = [&] {
            if constexpr (Certified == -1) return stage->native_differences_exact;
            else return std::bool_constant<Certified == 1>{};
        }();
        int const sa0 = root_orientation<Root>(q0, q1, p0, exact_differences);
        int const sa1 = root_orientation<Root>(q0, q1, p1, exact_differences);
        if (sa0 * sa1 > 0) return selected::disjoint();
        int const sb0 = root_orientation<Root>(p0, p1, q0, exact_differences);
        int const sb1 = root_orientation<Root>(p0, p1, q1, exact_differences);
        if (sb0 * sb1 > 0) return selected::disjoint();
        if (sa0 == 0 && sa1 == 0 && sb0 == 0 && sb1 == 0)
        {
            range const p{stage->vertex(stage->model[0]->seg_id, 0), stage->vertex(stage->model[0]->seg_id, 1)};
            range const q{stage->vertex(stage->model[1]->seg_id, 0), stage->vertex(stage->model[1]->seg_id, 1)};
            return filtered_collinear_intersection<coordinate>::apply(p, q, policy);
        }
        int const endpoint = sa0 == 0 ? 0 : sa1 == 0 ? 1 : sb0 == 0 ? 2 : sb1 == 0 ? 3 : -1;
        construction_info const info{*stage, endpoint};
        side_info sides;
        sides.template set<0>(sa0, sa1);
        sides.template set<1>(sb0, sb1);
        if (sa0 == 0 && (root_equal<Root>(p0, q0) || root_equal<Root>(p0, q1)))
            return selected::segments_share_common_point(sides, info, stage->vertex(stage->model[0]->seg_id, 0));
        if (sa1 == 0 && (root_equal<Root>(p1, q0) || root_equal<Root>(p1, q1)))
            return selected::segments_share_common_point(sides, info, stage->vertex(stage->model[0]->seg_id, 1));
        using p_type = std::remove_cv_t<std::remove_reference_t<decltype(p0)>>;
        using q_type = std::remove_cv_t<std::remove_reference_t<decltype(q0)>>;
        model::referring_segment<p_type const> const first(p0, p1);
        model::referring_segment<q_type const> const second(q0, q1);
        return selected::segments_crosses(sides, info, first, second);
    }
};

}}}}}

#endif
