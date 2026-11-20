// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Licensed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_GEOMETRY_STRATEGIES_DETAIL_OUTPUT_SUPPORT_HPP
#define BOOST_GEOMETRY_STRATEGIES_DETAIL_OUTPUT_SUPPORT_HPP

#include <boost/range/algorithm/reverse.hpp>
#include <boost/geometry/algorithms/detail/signed_size_type.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{

namespace output_support_detail
{
template <typename Strategy, typename Range>
inline auto remove_spikes_at_closure(Strategy const& strategy, Range& range, int)
    -> decltype(strategy.remove_spikes_at_closure(range), bool())
{ strategy.remove_spikes_at_closure(range); return true; }
template <typename Strategy, typename Range>
inline bool remove_spikes_at_closure(Strategy const&, Range&, long) { return false; }
template <typename Strategy, typename Range>
inline auto fix_closure(Strategy const& strategy, Range& range, int)
    -> decltype(strategy.fix_closure(range), bool())
{ strategy.fix_closure(range); return true; }
template <typename Strategy, typename Range>
inline bool fix_closure(Strategy const&, Range&, long) { return false; }

// A detected callback owns the operation; absent callbacks leave it to the caller.
template <typename Strategy, typename Range, typename Point>
inline auto append_no_duplicates(Strategy const& strategy, Range& range,
                                 Point const& point, int)
    -> decltype(strategy.append_no_duplicates(range, point), bool())
{
    strategy.append_no_duplicates(range, point);
    return true;
}

template <typename Strategy, typename Range, typename Point>
inline bool append_no_duplicates(Strategy const&, Range&, Point const&, long)
{
    return false;
}

template <typename Strategy, typename Range, typename Point>
inline auto append_no_dups_or_spikes(Strategy const& strategy, Range& range,
                                    Point const& point, int)
    -> decltype(strategy.append_no_dups_or_spikes(range, point), bool())
{
    strategy.append_no_dups_or_spikes(range, point);
    return true;
}

template <typename Strategy, typename Range, typename Point>
inline bool append_no_dups_or_spikes(Strategy const&, Range&, Point const&, long)
{
    return false;
}

template <typename Strategy, typename Range, typename Point>
inline auto append_no_collinear(Strategy const& strategy, Range& range,
                                Point const& point, int)
    -> decltype(strategy.append_no_collinear(range, point), bool())
{
    strategy.append_no_collinear(range, point);
    return true;
}

template <typename Strategy, typename Range, typename Point>
inline bool append_no_collinear(Strategy const&, Range&, Point const&, long)
{
    return false;
}

template <bool Reverse, bool RemoveSpikes, typename Strategy, typename Geometry,
          typename SegmentIdentifier, typename Range>
inline auto copy_segments(Strategy const& strategy, Geometry const& geometry,
                          SegmentIdentifier const& seg_id, signed_size_type to_index,
                          Range& range, int)
    -> decltype(strategy.template copy_segments<Reverse, RemoveSpikes>(
                    geometry, seg_id, to_index, range), bool())
{
    strategy.template copy_segments<Reverse, RemoveSpikes>(geometry, seg_id, to_index, range);
    return true;
}

template <bool Reverse, bool RemoveSpikes, typename Strategy, typename Geometry,
          typename SegmentIdentifier, typename Range>
inline bool copy_segments(Strategy const&, Geometry const&, SegmentIdentifier const&,
                          signed_size_type, Range&, long)
{
    return false;
}

template <bool Reverse1, bool Reverse2, typename Strategy, typename Range,
          typename Geometry1, typename Geometry2, typename SegmentIdentifier>
inline auto prepare_output_edge(Strategy const& strategy, Range& range,
                                Geometry1 const& geometry1, Geometry2 const& geometry2,
                                SegmentIdentifier const& seg_id, int)
    -> decltype(strategy.template prepare_output_edge<Reverse1, Reverse2>(
                    range, geometry1, geometry2, seg_id), void())
{
    strategy.template prepare_output_edge<Reverse1, Reverse2>(range, geometry1, geometry2, seg_id);
}

template <bool Reverse1, bool Reverse2, typename Strategy, typename Range,
          typename Geometry1, typename Geometry2, typename SegmentIdentifier>
inline void prepare_output_edge(Strategy const&, Range&, Geometry1 const&,
                                Geometry2 const&, SegmentIdentifier const&, long)
{}

template <typename Strategy, typename Range>
inline auto prepare_clipping_segment(Strategy const& strategy, Range const& range,
                                     signed_size_type index, int)
    -> decltype(strategy.prepare_segment(range, index), void())
{
    strategy.prepare_segment(range, index);
}

template <typename Strategy, typename Range>
inline void prepare_clipping_segment(Strategy const&, Range const&, signed_size_type, long)
{}

template <typename Strategy, typename Range>
inline auto reverse_range(Strategy const& strategy, Range& range, int)
    -> decltype(strategy.reverse_range(range), void())
{
    strategy.reverse_range(range);
}

template <typename Strategy, typename Range>
inline void reverse_range(Strategy const&, Range& range, long)
{
    boost::reverse(range);
}

template <bool Reverse, typename Geometry, typename SegmentIdentifier, typename PointOut>
inline auto copy_source_vertex(Geometry const& geometry, SegmentIdentifier const& seg_id,
                               signed_size_type offset, PointOut& point, int)
    -> decltype(PointOut::template copy_source_vertex<Reverse>(geometry, seg_id, offset, point), bool())
{
    PointOut::template copy_source_vertex<Reverse>(geometry, seg_id, offset, point);
    return true;
}

template <bool Reverse, typename Geometry, typename SegmentIdentifier, typename PointOut>
inline bool copy_source_vertex(Geometry const&, SegmentIdentifier const&,
                               signed_size_type, PointOut&, long)
{
    return false;
}

} // namespace output_support_detail

template <typename Strategy, typename Range>
inline bool try_remove_spikes_at_closure(Strategy const& strategy, Range& range)
{ return output_support_detail::remove_spikes_at_closure(strategy, range, 0); }
template <typename Strategy, typename Range>
inline bool try_fix_closure(Strategy const& strategy, Range& range)
{ return output_support_detail::fix_closure(strategy, range, 0); }

template <typename Strategy, typename Range, typename Point>
inline bool try_append_no_duplicates(Strategy const& strategy, Range& range, Point const& point)
{
    return output_support_detail::append_no_duplicates(strategy, range, point, 0);
}

template <typename Strategy, typename Range, typename Point>
inline bool try_append_no_dups_or_spikes(Strategy const& strategy, Range& range, Point const& point)
{
    return output_support_detail::append_no_dups_or_spikes(strategy, range, point, 0);
}

template <typename Strategy, typename Range, typename Point>
inline bool try_append_no_collinear(Strategy const& strategy, Range& range, Point const& point)
{
    return output_support_detail::append_no_collinear(strategy, range, point, 0);
}

template <bool Reverse, bool RemoveSpikes, typename Strategy, typename Geometry,
          typename SegmentIdentifier, typename Range>
inline bool try_copy_segments(Strategy const& strategy, Geometry const& geometry,
                              SegmentIdentifier const& seg_id, signed_size_type to_index,
                              Range& range)
{
    return output_support_detail::copy_segments<Reverse, RemoveSpikes>(
        strategy, geometry, seg_id, to_index, range, 0);
}

template <bool Reverse1, bool Reverse2, typename Strategy, typename Range,
          typename Geometry1, typename Geometry2, typename SegmentIdentifier>
inline void prepare_output_edge(Strategy const& strategy, Range& range,
                                Geometry1 const& geometry1, Geometry2 const& geometry2,
                                SegmentIdentifier const& seg_id)
{
    output_support_detail::prepare_output_edge<Reverse1, Reverse2>(
        strategy, range, geometry1, geometry2, seg_id, 0);
}

template <typename Strategy, typename Range>
inline void reverse_output_range(Strategy const& strategy, Range& range)
{
    output_support_detail::reverse_range(strategy, range, 0);
}

template <typename Strategy, typename Range>
inline void prepare_clipping_segment(Strategy const& strategy, Range const& range,
                                     signed_size_type index)
{
    output_support_detail::prepare_clipping_segment(strategy, range, index, 0);
}

template <bool Reverse, typename Geometry, typename SegmentIdentifier, typename PointOut>
inline bool try_copy_source_vertex(Geometry const& geometry, SegmentIdentifier const& seg_id,
                                   signed_size_type offset, PointOut& point)
{
    return output_support_detail::copy_source_vertex<Reverse>(geometry, seg_id, offset, point, 0);
}

}}}} // namespace boost::geometry::strategies::detail

#endif // BOOST_GEOMETRY_STRATEGIES_DETAIL_OUTPUT_SUPPORT_HPP
