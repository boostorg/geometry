// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_BORROWED_PRIMITIVE_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_BORROWED_PRIMITIVE_HPP

#include <array>
#include <cstddef>
#include <memory>
#include <type_traits>
#include <variant>

#include <boost/geometry/algorithms/detail/overlay/segment_as_subrange.hpp>
#include <boost/geometry/strategies/cartesian/exact/borrowed_geometry.hpp>
#include <boost/geometry/strategies/cartesian/exact/borrowed_intersection.hpp>
#include <boost/geometry/strategies/cartesian/exact/segment.hpp>
#include <boost/geometry/strategies/detail/segment_as_subrange.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian
{

namespace borrowed_primitive_detail
{

template <typename Root>
struct work
{
    borrowed_stage<Root> stage;
    std::array<typename Root::operation_type, 2> model;
    borrowed_intersection_result<Root> result;

    template <typename Segment1, typename Segment2>
    work(borrowed_context<Root>& context, Segment1 const& first, Segment2 const& second)
        : stage(context, first, second)
    {
        model[0].seg_id = segment_identifier(0, -1, -1, 0);
        model[1].seg_id = segment_identifier(1, -1, -1, 0);
        stage.model = {{std::addressof(model[0]), std::addressof(model[1])}};
    }

    void finish()
    {
        auto const first = stage.support(model[0].seg_id);
        auto const second = stage.support(model[1].seg_id);
        BOOST_ASSERT(first.valid() && second.valid());
        model[0].seg_id = segment_identifier(static_cast<signed_size_type>(first.range), -1, -1,
                                             static_cast<signed_size_type>(first.segment));
        model[1].seg_id = segment_identifier(static_cast<signed_size_type>(second.range), -1, -1,
                                             static_cast<signed_size_type>(second.segment));
        for (auto& definition : result.definitions)
        {
            definition.is_seed = false;
        }
        stage.finalized = true;
        for (auto& source : stage.sources)
        {
            source = std::monostate{};
        }
    }
};

template <typename Root>
struct active_stage_scope
{
    borrowed_context<Root>& context;
    borrowed_stage<Root>* previous;

    active_stage_scope(borrowed_context<Root>& context, borrowed_stage<Root>& stage)
        : context(context), previous(context.active_stage)
    {
        context.active_stage = &stage;
    }

    ~active_stage_scope() { context.active_stage = previous; }
    active_stage_scope(active_stage_scope const&) = delete;
    active_stage_scope& operator=(active_stage_scope const&) = delete;
};

} // namespace borrowed_primitive_detail

template <typename Root, typename Segment1, typename Segment2>
inline bool borrowed_segments_disjoint(Segment1 const& first, Segment2 const& second)
{
    strategies::detail::segment_as_subrange<Segment1> range1(first);
    strategies::detail::segment_as_subrange<Segment2> range2(second);
    auto const result = segment_intersection<typename Root::coordinate_type>::apply
        (range1, range2, policies::relate::segments_direction());
    return result.how == 'd';
}

template <typename Root, typename Segment1, typename Segment2,
          typename OutputIterator, typename Converter>
inline OutputIterator borrowed_intersect_segments(borrowed_context<Root>& context,
                                                 Segment1 const& first, Segment2 const& second,
                                                 OutputIterator out, Converter&& convert)
{
    static_assert(std::is_same<geometry::tag_t<Segment1>, segment_tag>::value
               && std::is_same<geometry::tag_t<Segment2>, segment_tag>::value,
                  "Primitive intersection requires original native segments");
    using point = borrowed_point<Root>;
    static_assert(!std::is_same<geometry::point_type_t<Segment1>, point>::value
               && !std::is_same<geometry::point_type_t<Segment2>, point>::value,
                  "Borrowed intermediate segments require their owning turn stage");

    auto& owned = context.template retain<borrowed_primitive_detail::work<Root>>(context, first, second);
    {
        borrowed_primitive_detail::active_stage_scope<Root> scope(context, owned.stage);
        geometry::detail::segment_as_subrange<Segment1> range1(first);
        geometry::detail::segment_as_subrange<Segment2> range2(second);
        using policy = policies::relate::segments_intersection_points<borrowed_intersection_result<Root>>;
        owned.result = borrowed_segment_intersection<Root>(context).apply(range1, range2, policy());
    }
    owned.finish();
    for (std::size_t i = 0; i != owned.result.count; ++i)
    {
        *out++ = convert(owned.result.intersections[i]);
    }
    return out;
}

}}}}} // namespace boost::geometry::strategies::detail::exact_cartesian

#endif // BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_BORROWED_PRIMITIVE_HPP
