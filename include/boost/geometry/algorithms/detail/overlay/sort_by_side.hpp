// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2015 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_SORT_BY_SIDE_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_SORT_BY_SIDE_HPP

#include <algorithm>
#include <vector>

#include <boost/geometry/algorithms/detail/direction_code.hpp>
#include <boost/geometry/algorithms/detail/overlay/turn_info.hpp>
#include <boost/geometry/strategies/side.hpp>

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace overlay { namespace sort_by_side
{

enum index_type { index_unknown = -1, index_from = 0, index_to = 1 };

// Point-wrapper, adding some properties
template <typename Point>
struct ranked_point
{
    ranked_point()
        : rank(0)
        , main_rank(0)
        , turn_index(-1)
        , op_index(-1)
        , index(index_unknown)
        , operation(operation_none)
    {}

    ranked_point(const Point& p, signed_size_type ti, signed_size_type oi,
                 index_type i, operation_type op, segment_identifier sid)
        : point(p)
        , rank(0)
        , main_rank(0)
        , turn_index(ti)
        , op_index(oi)
        , index(i)
        , operation(op)
        , seg_id(sid)
    {}

    Point point;
    std::size_t rank;
    std::size_t main_rank;
    signed_size_type turn_index;
    signed_size_type op_index;
    index_type index;
    operation_type operation;
    segment_identifier seg_id;
};

struct less_by_turn_index
{
    template <typename T>
    inline bool operator()(const T& first, const T& second) const
    {
        return first.turn_index == second.turn_index
            ? first.index < second.index
            : first.turn_index < second.turn_index
            ;
    }
};

struct less_by_index
{
    template <typename T>
    inline bool operator()(const T& first, const T& second) const
    {
        // First order by from/to
        if (first.index != second.index)
        {
            return first.index < second.index;
        }
        // All the same, order by turn index (we might consider length too)
        return first.turn_index < second.turn_index;
    }
};

struct less_false
{
    template <typename T>
    inline bool operator()(const T&, const T& ) const
    {
        return false;
    }
};

template <typename Point, typename LessOnSame, typename Compare>
struct less_by_side
{
    typedef typename strategy::side::services::default_strategy
        <
            typename cs_tag<Point>::type
        >::type side;

    less_by_side(const Point& p1, const Point& p2)
        : m_p1(p1)
        , m_p2(p2)
    {}

    template <typename T>
    inline bool operator()(const T& first, const T& second) const
    {
        LessOnSame on_same;
        Compare compare;

        int const side_first = side::apply(m_p1, m_p2, first.point);
        int const side_second = side::apply(m_p1, m_p2, second.point);

        if (side_first == 0 && side_second == 0)
        {
            // Both collinear. They might point into different directions: <------*------>
            // If so, order the one going backwards as the very first.

            int const first_code = direction_code(m_p1, m_p2, first.point);
            int const second_code = direction_code(m_p1, m_p2, second.point);

            // Order by code, backwards first, then forward.
            return first_code != second_code
                ? first_code < second_code
                : on_same(first, second)
                ;
        }
        else if (side_first == 0
                && direction_code(m_p1, m_p2, first.point) == -1)
        {
            // First collinear and going backwards.
            // Order as the very first, so return always true
            return true;
        }
        else if (side_second == 0
            && direction_code(m_p1, m_p2, second.point) == -1)
        {
            // Second is collinear and going backwards
            // Order as very last, so return always false
            return false;
        }

        // They are not both collinear

        if (side_first != side_second)
        {
            return compare(side_first, side_second);
        }

        // They are both left, both right, and/or both collinear (with each other and/or with p1,p2)
        // Check mutual side
        int const side_second_wrt_first = side::apply(m_p2, first.point, second.point);

        if (side_second_wrt_first == 0)
        {
            return on_same(first, second);
        }

        int const side_first_wrt_second = -side_second_wrt_first;

        // Both are on same side, and not collinear
        // Union: return true if second is right w.r.t. first, so -1,
        // so other is 1. union has greater as compare functor
        // Intersection: v.v.
        return compare(side_first_wrt_second, side_second_wrt_first);
    }

private :
    Point m_p1, m_p2;
};

template <bool Reverse1, bool Reverse2, typename Point, typename Compare>
struct side_sorter
{
    typedef ranked_point<Point> rp;


    template <typename Operation, typename Geometry1, typename Geometry2>
    void add(Operation const& op, signed_size_type turn_index, signed_size_type op_index,
            Geometry1 const& geometry1,
            Geometry2 const& geometry2,
            bool is_subject = false)
    {
        Point point1, point2, point3;
        geometry::copy_segment_points<Reverse1, Reverse2>(geometry1, geometry2,
                op.seg_id, point1, point2, point3);
        Point const& point_to = op.fraction.is_one() ? point3 : point2;

        m_ranked_points.push_back(rp(point1, turn_index, op_index, index_from, op.operation, op.seg_id));
        m_ranked_points.push_back(rp(point_to, turn_index, op_index, index_to, op.operation, op.seg_id));

        if (is_subject)
        {
            m_from = point1;
        }
    }

    void apply(Point const& turn_point)
    {
        // We need three compare functors:
        // 1) to order clockwise (union) or counter clockwise (intersection)
        // 2) to order by side, resulting in unique ranks for all points
        // 3) to order by side, resulting in non-unique ranks
        //    to give colinear points

        // Sort by side and assign rank
        less_by_side<Point, less_by_index, Compare> less_unique(m_from, turn_point);
        less_by_side<Point, less_false, Compare> less_non_unique(m_from, turn_point);

        std::sort(m_ranked_points.begin(), m_ranked_points.end(), less_unique);

        std::size_t colinear_rank = 0;
        for (std::size_t i = 0; i < m_ranked_points.size(); i++)
        {
            if (i > 0
                && less_non_unique(m_ranked_points[i - 1], m_ranked_points[i]))
            {
                // It is not collinear
                colinear_rank++;
            }

            m_ranked_points[i].rank = i;
            m_ranked_points[i].main_rank = colinear_rank;
        }
    }

    // Returns first index which is outgoing, and all incoming arcs are balanced
    // by outgoing arcs
    std::size_t first_open_index()
    {
        // map source index to state
        typedef std::map<int, int> map_type;
        map_type state;
        bool free = false;
        std::size_t result = m_ranked_points.size();
        std::size_t last_main_rank = 0;

        for (std::size_t i = 0; i < m_ranked_points.size(); i++)
        {
            const rp& ranked = m_ranked_points[i];

            if (free && ranked.main_rank > last_main_rank)
            {
                return result;
            }
            free = false;

            if (ranked.index == index_from)
            {
                state[ranked.seg_id.source_index]++;
            }
            else
            {
                state[ranked.seg_id.source_index]--;
            }

            if (ranked.index == index_to)
            {
                int sum = 0;
                for (map_type::const_iterator it = state.begin();
                     it != state.end(); ++it)
                {
                    sum += it->second;
                }

                if (sum == 0)
                {
                    // It is open. If next is a new main rank,
                    // then this is the result
                    free = true;
                    result = i;
                    last_main_rank = ranked.main_rank;
                }
            }
        }

        if (free)
        {
            return result;
        }

        return m_ranked_points.size();
    }

    void reverse()
    {
        if (m_ranked_points.empty())
        {
            return;
        }

        int const last = 1 + m_ranked_points.back().main_rank;

        // Move iterator after main_rank==0
        bool has_first = false;
        typename container_type::iterator it = m_ranked_points.begin() + 1;
        for (; it != m_ranked_points.end() && it->main_rank == 0; ++it)
        {
            has_first = true;
        }

        int index = 0;
        if (has_first)
        {
            // Reverse first part (having main_rank == 0), if any,
            // but skip the very first row
            std::reverse(m_ranked_points.begin() + 1, it);
            for (typename container_type::iterator fit = m_ranked_points.begin();
                 fit != it; ++fit, ++index)
            {
                BOOST_ASSERT(fit->main_rank == 0);
                fit->rank = index;
            }
        }

        // Reverse the rest (main rank > 0)
        std::reverse(it, m_ranked_points.end());
        for (; it != m_ranked_points.end(); ++it, ++index)
        {
            BOOST_ASSERT(it->main_rank > 0);
            it->rank = index;
            it->main_rank = last - it->main_rank;
        }
    }

//protected :

    typedef std::vector<rp> container_type;
    container_type m_ranked_points;
    Point m_from;
};


}}} // namespace detail::overlay::sort_by_side
#endif //DOXYGEN_NO_DETAIL


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_SORT_BY_SIDE_HPP
