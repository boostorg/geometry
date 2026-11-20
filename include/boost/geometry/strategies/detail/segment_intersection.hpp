// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_DETAIL_SEGMENT_INTERSECTION_HPP
#define BOOST_GEOMETRY_STRATEGIES_DETAIL_SEGMENT_INTERSECTION_HPP

#include <cstddef>
#include <utility>

#include <boost/geometry/algorithms/detail/overlay/segment_as_subrange.hpp>
#include <boost/geometry/core/point_type.hpp>
#include <boost/geometry/policies/relate/intersection_points.hpp>
#include <boost/geometry/strategies/detail/point_conversion.hpp>
#include <boost/geometry/strategies/relate/intersection_types.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{

namespace segment_intersection_detail
{

template <typename Strategy, typename Segment1, typename Segment2>
inline auto disjoint(Strategy const& strategy, Segment1 const& first, Segment2 const& second, int)
    -> decltype(strategy.disjoint_segments(first, second))
{
    return strategy.disjoint_segments(first, second);
}

template <typename Strategy, typename Segment1, typename Segment2>
inline bool disjoint(Strategy const& strategy, Segment1 const& first, Segment2 const& second, long)
{
    using point_type = typename strategies::relate::services::intersection_point_type
        <Strategy, point_type_t<Segment1>>::type;
    using result_type = typename strategies::relate::services::intersection_result_type<Strategy, point_type>::type;
    using policy_type = policies::relate::segments_intersection_points<result_type>;
    geometry::detail::segment_as_subrange<Segment1> range1(first);
    geometry::detail::segment_as_subrange<Segment2> range2(second);
    result_type result = strategy.relate().apply(range1, range2, policy_type());
    return result.count == 0;
}

template <typename PointOut, typename Strategy, typename Segment1, typename Segment2,
          typename OutputIterator, typename Converter>
inline auto apply(Strategy const& strategy, Segment1 const& segment1, Segment2 const& segment2,
                  OutputIterator out, Converter&& convert, int)
    -> decltype(strategy.intersect_segments(segment1, segment2, out, std::forward<Converter>(convert)))
{
    return strategy.intersect_segments(segment1, segment2, out, std::forward<Converter>(convert));
}

template <typename PointOut, typename Strategy, typename Segment1, typename Segment2,
          typename OutputIterator, typename Converter>
inline OutputIterator apply(Strategy const& strategy, Segment1 const& segment1, Segment2 const& segment2,
                            OutputIterator out, Converter&&, long)
{
    using point_type = typename strategies::relate::services::intersection_point_type
        <Strategy, point_type_t<PointOut>>::type;
    using intersection_return_type = typename strategies::relate::services::intersection_result_type
        <Strategy, point_type>::type;
    using policy_type = policies::relate::segments_intersection_points<intersection_return_type>;

    geometry::detail::segment_as_subrange<Segment1> sub_range1(segment1);
    geometry::detail::segment_as_subrange<Segment2> sub_range2(segment2);
    intersection_return_type is = strategy.relate().apply(sub_range1, sub_range2, policy_type());

    for (std::size_t i = 0; i < is.count; i++)
    {
        PointOut p;
        strategies::detail::convert_geometry(strategy, is.intersections[i], p);
        *out++ = p;
    }
    return out;
}

} // namespace segment_intersection_detail

template <typename Strategy, typename Segment1, typename Segment2>
inline bool disjoint_segments(Strategy const& strategy, Segment1 const& first, Segment2 const& second)
{
    return segment_intersection_detail::disjoint(strategy, first, second, 0);
}

template <typename PointOut, typename Strategy, typename Segment1, typename Segment2,
          typename OutputIterator, typename Converter>
inline OutputIterator intersect_segments(Strategy const& strategy,
                                        Segment1 const& segment1, Segment2 const& segment2,
                                        OutputIterator out, Converter&& convert)
{
    return segment_intersection_detail::apply<PointOut>
        (strategy, segment1, segment2, out, std::forward<Converter>(convert), 0);
}

}}}} // namespace boost::geometry::strategies::detail

#endif // BOOST_GEOMETRY_STRATEGIES_DETAIL_SEGMENT_INTERSECTION_HPP
