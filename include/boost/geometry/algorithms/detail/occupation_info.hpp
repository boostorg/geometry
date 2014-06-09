// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2012 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OCCUPATION_INFO_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OCCUPATION_INFO_HPP

#include <algorithm>
#include <boost/range.hpp>

#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/core/point_type.hpp>

#include <boost/geometry/policies/compare.hpp>
#include <boost/geometry/iterators/closing_iterator.hpp>

#include <boost/geometry/algorithms/detail/get_left_turns.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail
{

template
<
    typename Iterator,
    typename Vector,
    typename RobustPolicy,
    typename RobustPoint
>
inline Iterator advance_circular(Iterator it, Vector const& vector,
        RobustPolicy const& robust_policy,
        RobustPoint& robust_point,
        segment_identifier& seg_id,
        bool forward = true)
{
    int const increment = forward ? 1 : -1;
    if (it == boost::begin(vector) && increment < 0)
    {
        it = boost::end(vector);
        seg_id.segment_index = boost::size(vector);
    }
    it += increment;
    seg_id.segment_index += increment;
    if (it == boost::end(vector))
    {
        seg_id.segment_index = 0;
        it = boost::begin(vector);
    }
    geometry::recalculate(robust_point, *it, robust_policy);
    return it;
}

template <typename Point, typename T>
struct angle_info
{
    typedef T angle_type;
    typedef Point point_type;

    segment_identifier seg_id;
    int turn_index;
    int operation_index;
    Point intersection_point;
    Point point; // either incoming or outgoing point
    bool incoming;
    bool blocked;
    bool included;

    inline angle_info()
        : blocked(false)
        , included(false)
    {}
};

template <typename AngleInfo>
class occupation_info
{
    typedef std::vector<AngleInfo> collection_type;

public :
    collection_type angles;
    std::vector<detail::left_turns::turn_angle_info<typename AngleInfo::point_type> > turns;
    int m_count;

private :
    bool m_occupied;
    bool m_calculated;

public :
    inline occupation_info()
        : m_count(0)
        , m_occupied(false)
        , m_calculated(false)
    {}

    template <typename RobustPoint>
    inline void add(RobustPoint const& incoming_point,
                    RobustPoint const& outgoing_point,
                    RobustPoint const& intersection_point,
                    int turn_index, int operation_index,
                    segment_identifier const& seg_id)
    {
        geometry::equal_to<RobustPoint> comparator;
        if (comparator(incoming_point, intersection_point))
        {
            return;
        }
        if (comparator(outgoing_point, intersection_point))
        {
            return;
        }

        AngleInfo info;
        info.seg_id = seg_id;
        info.turn_index = turn_index;
        info.operation_index = operation_index;
        info.intersection_point = intersection_point;

        {
            info.point = incoming_point;
            info.incoming = true;
            angles.push_back(info);
        }
        {
            info.point = outgoing_point;
            info.incoming = false;
            angles.push_back(info);
        }
        detail::left_turns::turn_angle_info<typename AngleInfo::point_type> turn(seg_id, incoming_point, outgoing_point);
        turns.push_back(turn);

        m_calculated = false;
    }

    template <typename RobustPoint>
    inline void get_left_turns(RobustPoint const& origin,
                    std::vector<detail::left_turns::left_turn>& turns_to_keep)
    {
        // Sort on angle
        std::sort(angles.begin(), angles.end(),
                detail::left_turns::angle_less<typename AngleInfo::point_type>(origin));

        // Block all turns on the right side of any turn
        detail::left_turns::block_turns_on_right_sides(turns, angles);

        detail::left_turns::get_left_turns(angles, origin, turns_to_keep);
    }
};


template
<
    typename RobustPoint,
    typename RobustPolicy,
    typename Ring,
    typename Info
>
inline void add_incoming_and_outgoing_angles(
                RobustPoint const& intersection_point, // rescaled
                Ring const& ring, // non-rescaled
                RobustPolicy const& robust_policy,
                int turn_index,
                int operation_index,
                segment_identifier seg_id,
                Info& info)
{
    typedef typename boost::range_iterator
        <
            Ring const
        >::type iterator_type;

    int const n = boost::size(ring);
    if (seg_id.segment_index >= n || seg_id.segment_index < 0)
    {
        return;
    }

    segment_identifier real_seg_id = seg_id;
    iterator_type it = boost::begin(ring) + seg_id.segment_index;
    RobustPoint current;
    geometry::recalculate(current, *it, robust_policy);

    // TODO: we can add this points in get_turn_info/assign already
    // TODO: if we use turn-info ("to", "middle"), we know if to advance without resorting to equals
    geometry::equal_to<RobustPoint> comparator;

    if (comparator(intersection_point, current))
    {
        // It should be equal only once. But otherwise we skip it (in "add")
        it = advance_circular(it, ring, robust_policy, current, seg_id, false);
    }

    RobustPoint incoming = current;

    if (comparator(intersection_point, current))
    {
        it = advance_circular(it, ring, robust_policy, current, real_seg_id);
    }
    else
    {
        // Don't upgrade the ID
        it = advance_circular(it, ring, robust_policy, current, seg_id);
    }
    for (int defensive_check = 0;
        comparator(intersection_point, current) && defensive_check < n;
        defensive_check++)
    {
        it = advance_circular(it, ring, robust_policy, current, real_seg_id);
    }

    info.add(incoming, current, intersection_point, turn_index, operation_index, real_seg_id);
}


} // namespace detail
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OCCUPATION_INFO_HPP
