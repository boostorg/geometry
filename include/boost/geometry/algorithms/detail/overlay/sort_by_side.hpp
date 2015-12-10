// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2015 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_SORT_BY_SIDE_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_SORT_BY_SIDE_HPP

#include <algorithm>

#include <boost/array.hpp>

#include <boost/geometry/algorithms/detail/direction_code.hpp>
#include <boost/geometry/algorithms/detail/overlay/turn_info.hpp>
#include <boost/geometry/strategies/side.hpp>

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace overlay { namespace sort_by_side
{

enum index_type { index_unknown = -1, index_from = 0, index_to = 1 };

enum role_type { role_unknown = -1, role_subject = 1, role_a, role_b };

// Point-wrapper, adding some properties
template <typename Point>
struct point_with_role
{
    point_with_role()
        : rank(0)
        , role(role_unknown)
        , index(index_unknown)
        , operation(operation_none)
    {}

    point_with_role(const Point& p, role_type r, index_type i,
                    operation_type op)
        : point(p)
        , rank(0)
        , role(r)
        , index(i)
        , operation(op)
    {}

    Point point;
    std::size_t rank;
    role_type role;
    index_type index;
    operation_type operation;
};

struct less_by_role
{
    template <typename T>
    inline bool operator()(const T& first, const T& second) const
    {
        return first.role == second.role
            ? first.index < second.index
            : first.role < second.role
            ;
    }
};

template <typename Point>
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
        less_by_role by_role;

        // Order from left to right so state that left = -1, right = 1
        int const minus_side_first = -side::apply(m_p1, m_p2, first.point);
        int const minus_side_second = -side::apply(m_p1, m_p2, second.point);

        if (minus_side_first == 0 && minus_side_second == 0)
        {
            // Both collinear. They might point into different directions: <------*------>
            // If so, order the one going backwards as the very first.

            int const first_code = direction_code(m_p1, m_p2, first.point);
            int const second_code = direction_code(m_p1, m_p2, second.point);

            // Order by code, backwards first, then forward. For same direction, use role (for now)
            return first_code != second_code
                ? first_code < second_code
                : by_role(first, second)
                ;
        }
        else if (minus_side_first == 0
                && direction_code(m_p1, m_p2, first.point) == -1)
        {
            // First collinear and going backwards.
            // Order as the very first, so return always true
            return true;
        }
        else if (minus_side_second == 0
            && direction_code(m_p1, m_p2, second.point) == -1)
        {
            // Second is collinear and going backwards
            // Order as very last, so return always false
            return false;
        }

        // They are not both collinear

        if (minus_side_first != minus_side_second)
        {
            return minus_side_first < minus_side_second;
        }

        // They are both left, both right, and/or both collinear (with each other and/or with p1,p2)
        // Check mutual side
        int const side_second_wrt_first = side::apply(m_p2, first.point, second.point);

        // If collinear order by role,
        // otherwise, both are on same side, return true if second is right w.r.t. left
        return side_second_wrt_first == 0
            ? by_role(first, second)
            : side_second_wrt_first == -1
            ;
    }

private :
    Point m_p1, m_p2;
};

template <bool Reverse1, bool Reverse2, typename Point>
struct side_sorter
{
    template <typename Operation,  typename Geometry1, typename Geometry2>
    void apply(Point const& intersection_point,
            Operation const& op_a, Operation const& op_a_other,
            Operation const& op_b, Operation const& op_b_other,
            Geometry1 const& geometry1,
            Geometry2 const& geometry2)
    {
        // All this has to be called in the context of sorting two operations,
        // located on the same segment. Therefore this assertion should be true
        BOOST_ASSERT(op_a.seg_id == op_b.seg_id);

        Point both1, both2, both3;
        Point a_other1, a_other2, a_other3;
        Point b_other1, b_other2, b_other3;
        geometry::copy_segment_points<Reverse1, Reverse2>(geometry1, geometry2,
                op_a.seg_id, both1, both2, both3);
        geometry::copy_segment_points<Reverse1, Reverse2>(geometry1, geometry2,
                op_a_other.seg_id, a_other1, a_other2, a_other3);
        geometry::copy_segment_points<Reverse1, Reverse2>(geometry1, geometry2,
                op_b_other.seg_id, b_other1, b_other2, b_other3);

        Point const& both_to = op_a.fraction.is_one() ? both3 : both2;
        Point const& a_other_to = op_a_other.fraction.is_one() ? a_other3 : a_other2;
        Point const& b_other_to = op_b_other.fraction.is_one() ? b_other3 : b_other2;

        typedef point_with_role<Point> pwr;
        m_array[0] = pwr(both_to, role_subject, index_to, op_a.operation);
        m_array[1] = pwr(a_other1, role_a, index_from, op_a_other.operation);
        m_array[2] = pwr(a_other_to, role_a, index_to, op_a_other.operation);
        m_array[3] = pwr(b_other1, role_b, index_from, op_b_other.operation);
        m_array[4] = pwr(b_other_to, role_b, index_to, op_b_other.operation);

        // Sort by side and assign rank
        std::sort(m_array.begin(), m_array.end(),
                less_by_side<Point>(both1, intersection_point));
        for (std::size_t i = 0; i < m_array.size(); i++)
        {
            m_array[i].rank = i;
        }

        // Sort back in original order
        std::sort(m_array.begin(), m_array.end(), less_by_role());
    }

    std::size_t rank(role_type role, index_type index) const
    {
        BOOST_ASSERT(index >= index_from && index <= index_to
                && role >= role_subject && role <= role_b);

        return role == role_a ? m_array[1 + index].rank
            : role == role_b ? m_array[3 + index].rank
            : m_array.front().rank
            ;
    }

    // Returns true if b is independent without segments in between
    bool is_b_independent() const
    {
        //
        //     [1] A    B (to) [2]
        //   (from) \   |
        //           \  |
        //            \ |
        //             \|
        // S <----------+----------- B (from) [3]
        // (to) [0]     |\           .
        //              | \          .
        //              |  \         .
        //              |   \        .
        //              |    A [4] (to)
        //              S (from)

        // Segments are sorted CCW w.r.t. S -> IP (+)
        // B (polygon is on the right side) goes from right IP to top, there is
        // nothing in between. Here B is completely independent on Subject/A

        return rank(role_b, index_from) - 1 == rank(role_b, index_to);
    }

    bool is_intersection_switching_a_to_b() const
    {
        //
        //                   A (from) [2]
        //                  /
        //                 /
        //                /
        //               /
        // S <----------+----------- A (to) [3]
        // (to) [1]    /|\           .
        //            / | \          .
        //           /  |  \         .
        //          /   |   \        .
        //    (to) B[0] |    B (from) [4]
        //              S (from)

        // Here A is supposed to be the ii turn, B the non ii turn (this is
        // visible in the picture but might take some time to see it.
        // Image is derived from #case_107_multi, inverse version, but rotated.
        // All polygons are on the right side (of from->to)
        // A makes a ii turn with S
        // B makes a iu turn with S

        // B goes from lowerright to IP, and
        // then can switches to A's to at the right side.
        // The polygon thus made on both right sides has no other segments
        // in between.

        // In these cases iu is sorted first, ii as last

        return rank(role_b, index_from) - 1 == rank(role_a, index_to);
    }

    bool is_tight_a_to_b() const
    {
        // Similarly, but now for e.g. ordering colocated iu/ix turns.
        // If A goes to IP (polygon on right side) and then, immediately CCW,
        // B leaves IP, then order A first, before B, such that the outgoing
        // segment of B is traversed.
        return rank(role_a, index_from) - 1 == rank(role_b, index_to);
    }

    bool is_tight_b_to_a() const
    {
        // Symmetricly as is_tight_a_to_b. And here the condition is identical
        // as is_intersection_switching_a_to_b but here we switch to a!
        // Here it is not ii/iu but iu/ix (or similar)
        return rank(role_b, index_from) - 1 == rank(role_a, index_to);
    }


protected :

    boost::array<point_with_role<Point>, 5> m_array;
};


}}} // namespace detail::overlay::sort_by_side
#endif //DOXYGEN_NO_DETAIL


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_SORT_BY_SIDE_HPP
