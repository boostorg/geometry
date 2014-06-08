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

template <typename T, typename P1, typename P2>
inline T calculate_angle(P1 const& from_point, P2 const& to_point)
{
    typedef P1 vector_type;
    vector_type v = from_point;
    geometry::subtract_point(v, to_point);
    return atan2(geometry::get<1>(v), geometry::get<0>(v));
}

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
    Point direction_point;
    T angle;
    bool incoming;
};

template <typename AngleInfo>
class occupation_info
{
    typedef std::vector<AngleInfo> collection_type;

    struct angle_sort
    {
        inline bool operator()(AngleInfo const& left, AngleInfo const& right) const
        {
            // In this case we can compare even double using equals
            // return geometry::math::equals(left.angle, right.angle)
            return left.angle == right.angle
                ? int(left.incoming) < int(right.incoming)
                : left.angle < right.angle
                ;
        }
    };

public :
    collection_type angles;
    int m_count;

private :
    bool m_occupied;
    bool m_calculated;

    inline bool is_occupied()
    {
        if (boost::size(angles) <= 1)
        {
            return false;
        }

        std::sort(angles.begin(), angles.end(), angle_sort());

        typedef geometry::closing_iterator<collection_type const> closing_iterator;
        closing_iterator vit(angles);
        closing_iterator end(angles, true);

        closing_iterator prev = vit++;
        for( ; vit != end; prev = vit++)
        {
            if (! geometry::math::equals(prev->angle, vit->angle)
                && ! prev->incoming
                && vit->incoming)
            {
                return false;
            }
        }
        return true;
    }
#endif


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
            info.angle = calculate_angle<typename AngleInfo::angle_type>(incoming_point, intersection_point);
            info.direction_point = incoming_point;
            info.incoming = true;
            angles.push_back(info);
        }
        {
            info.angle = calculate_angle<typename AngleInfo::angle_type>(outgoing_point, intersection_point);
            info.direction_point = outgoing_point;
            info.incoming = false;
            angles.push_back(info);
        }

        m_calculated = false;
    }

#ifdef BOOST_GEOMETRY_OLD_GET_LEFT_TURNS
    inline bool occupied()
    {
        if (! m_calculated)
        {
            m_occupied = is_occupied();
            m_calculated = true;
        }
        return m_occupied;
    }
#endif

    template <typename Turns, typename TurnSegmentIndices>
    inline void get_left_turns(
                    Turns& turns, TurnSegmentIndices const& turn_segment_indices,
                    std::set<int>& keep_indices)
    {
#ifdef BOOST_GEOMETRY_OLD_GET_LEFT_TURNS
        std::sort(angles.begin(), angles.end(), angle_sort());
        calculate_left_turns<AngleInfo>(angles, turns, turn_segment_indices, keep_indices);
#else
        // Sort on angle
        std::sort(sorted.begin(), sorted.end(), detail::left_turns::angle_less<typename angle_info::point_type>(p));

        // Block all turns on the right side of any turn
        detail::left_turns::block_turns_on_right_sides(angles, sorted);
#endif
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
