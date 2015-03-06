// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2011-2015 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2015.
// Modifications copyright (c) 2015 Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_PARTITION_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_PARTITION_HPP

#include <cstddef>
#include <vector>
#include <boost/range.hpp>
#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/algorithms/assign.hpp>


namespace boost { namespace geometry
{

namespace detail { namespace partition
{

template <int Dimension, typename Box>
inline void divide_box(Box const& box, Box& lower_box, Box& upper_box)
{
    typedef typename coordinate_type<Box>::type ctype;

    // Divide input box into two parts, e.g. left/right
    ctype two = 2;
    ctype mid = (geometry::get<min_corner, Dimension>(box)
            + geometry::get<max_corner, Dimension>(box)) / two;

    lower_box = box;
    upper_box = box;
    geometry::set<max_corner, Dimension>(lower_box, mid);
    geometry::set<min_corner, Dimension>(upper_box, mid);
}

// Divide forward_range into three subsets: lower, upper and oversized
// (not-fitting)
// (lower == left or bottom, upper == right or top)
template
<
    typename OverlapsPolicy,
    typename ForwardRange,
    typename Box,
    typename IteratorVector
>
inline void divide_into_subsets(Box const& lower_box,
        Box const& upper_box,
        ForwardRange const& /*forward_range*/,
        IteratorVector const& input,
        IteratorVector& lower,
        IteratorVector& upper,
        IteratorVector& exceeding)
{
    typedef typename boost::range_iterator
        <
            IteratorVector const
        >::type it_type;

    for(it_type it = boost::begin(input); it != boost::end(input); ++it)
    {
        bool const lower_overlapping = OverlapsPolicy::apply(lower_box, **it);
        bool const upper_overlapping = OverlapsPolicy::apply(upper_box, **it);

        if (lower_overlapping && upper_overlapping)
        {
            exceeding.push_back(*it);
        }
        else if (lower_overlapping)
        {
            lower.push_back(*it);
        }
        else if (upper_overlapping)
        {
            upper.push_back(*it);
        }
        else
        {
            // Is nowhere. That is (since 1.58) possible, it might be
            // skipped by the OverlapsPolicy to enhance performance
        }
    }
}

template
<
    typename ExpandPolicy,
    typename Box,
    typename ForwardRange,
    typename IteratorVector
>
inline void expand_with_elements(Box& total,
                ForwardRange const& /*forward_range*/,
                IteratorVector const& input)
{
    typedef typename boost::range_iterator<IteratorVector const>::type it_type;
    for(it_type it = boost::begin(input); it != boost::end(input); ++it)
    {
        ExpandPolicy::apply(total, **it);
    }
}


// Match forward_range with itself
template
<
    typename ForwardRange,
    typename Policy,
    typename IteratorVector
>
inline void handle_one(ForwardRange const& /*forward_range*/,
        IteratorVector const& input,
        Policy& policy)
{
    if (boost::size(input) == 0)
    {
        return;
    }

    typedef typename boost::range_iterator<IteratorVector const>::type it_type;

    // Quadratic behaviour at lowest level (lowest quad, or all exceeding)
    for (it_type it1 = boost::begin(input); it1 != boost::end(input); ++it1)
    {
        it_type it2 = it1;
        for (++it2; it2 != boost::end(input); ++it2)
        {
            policy.apply(**it1, **it2);
        }
    }
}

// Match forward range 1 with forward range 2
template
<
    typename ForwardRange1,
    typename ForwardRange2,
    typename Policy,
    typename IteratorVector1,
    typename IteratorVector2
>
inline void handle_two(
        ForwardRange1 const& /*forward_range1*/, IteratorVector1 const& input1,
        ForwardRange2 const& /*forward_range2*/, IteratorVector2 const& input2,
        Policy& policy)
{
    typedef typename boost::range_iterator
        <
            IteratorVector1 const
        >::type iterator_type1;

    typedef typename boost::range_iterator
        <
            IteratorVector2 const
        >::type iterator_type2;

    if (boost::size(input1) == 0 || boost::size(input2) == 0)
    {
        return;
    }

    for(iterator_type1 it1 = boost::begin(input1);
        it1 != boost::end(input1);
        ++it1)
    {
        for(iterator_type2 it2 = boost::begin(input2);
            it2 != boost::end(input2);
            ++it2)
        {
            policy.apply(**it1, **it2);
        }
    }
}

template <typename IteratorVector>
inline bool recurse_ok(IteratorVector const& input,
                std::size_t min_elements, std::size_t level)
{
    return boost::size(input) >= min_elements
        && level < 100;
}

template <typename IteratorVector1, typename IteratorVector2>
inline bool recurse_ok(IteratorVector1 const& input1,
                IteratorVector2 const& input2,
                std::size_t min_elements, std::size_t level)
{
    return boost::size(input1) >= min_elements
        && recurse_ok(input2, min_elements, level);
}

template
<
    typename IteratorVector1,
    typename IteratorVector2,
    typename IteratorVector3
>
inline bool recurse_ok(IteratorVector1 const& input1,
                IteratorVector2 const& input2,
                IteratorVector3 const& input3,
                std::size_t min_elements, std::size_t level)
{
    return boost::size(input1) >= min_elements
        && recurse_ok(input2, input3, min_elements, level);
}

template
<
    int Dimension,
    typename Box,
    typename OverlapsPolicy1,
    typename OverlapsPolicy2,
    typename ExpandPolicy1,
    typename ExpandPolicy2,
    typename VisitBoxPolicy
>
class partition_two_ranges;


template
<
    int Dimension,
    typename Box,
    typename OverlapsPolicy,
    typename ExpandPolicy,
    typename VisitBoxPolicy
>
class partition_one_range
{
    template <typename ForwardRange, typename IteratorVector>
    static inline Box get_new_box(ForwardRange const& forward_range,
                    IteratorVector const& input)
    {
        Box box;
        geometry::assign_inverse(box);
        expand_with_elements<ExpandPolicy>(box, forward_range, input);
        return box;
    }

    template
    <
        typename ForwardRange,
        typename Policy,
        typename IteratorVector
    >
    static inline void next_level(Box const& box,
            ForwardRange const& forward_range,
            IteratorVector const& input,
            std::size_t level, std::size_t min_elements,
            Policy& policy, VisitBoxPolicy& box_policy)
    {
        if (recurse_ok(input, min_elements, level))
        {
            partition_one_range
            <
                1 - Dimension,
                Box,
                OverlapsPolicy,
                ExpandPolicy,
                VisitBoxPolicy
            >::apply(box, forward_range, input,
                level + 1, min_elements, policy, box_policy);
        }
        else
        {
            handle_one(forward_range, input, policy);
        }
    }

    // Function to switch to two forward ranges if there are
    // geometries exceeding the separation line
    template
    <
        typename ForwardRange,
        typename Policy,
        typename IteratorVector
    >
    static inline void next_level2(Box const& box,
            ForwardRange const& forward_range,
            IteratorVector const& input1,
            IteratorVector const& input2,
            std::size_t level, std::size_t min_elements,
            Policy& policy, VisitBoxPolicy& box_policy)
    {
        if (recurse_ok(input1, input2, min_elements, level))
        {
            partition_two_ranges
            <
                1 - Dimension,
                Box,
                OverlapsPolicy, OverlapsPolicy,
                ExpandPolicy, ExpandPolicy,
                VisitBoxPolicy
            >::apply(box, forward_range, input1, forward_range, input2,
                level + 1, min_elements, policy, box_policy);
        }
        else
        {
            handle_two(forward_range, input1, forward_range, input2, policy);
        }
    }

public :
    template
    <
        typename ForwardRange,
        typename Policy,
        typename IteratorVector
    >
    static inline void apply(Box const& box,
            ForwardRange const& forward_range,
            IteratorVector const& input,
            std::size_t level,
            std::size_t min_elements,
            Policy& policy, VisitBoxPolicy& box_policy)
    {
        box_policy.apply(box, level);

        Box lower_box, upper_box;
        divide_box<Dimension>(box, lower_box, upper_box);

        IteratorVector lower, upper, exceeding;
        divide_into_subsets<OverlapsPolicy>(lower_box, upper_box, forward_range,
                    input, lower, upper, exceeding);

        if (boost::size(exceeding) > 0)
        {
            // Get the box of exceeding-only
            Box exceeding_box = get_new_box(forward_range, exceeding);

            // Recursively do exceeding elements only, in next dimension they
            // will probably be less exceeding within the new box
            next_level(exceeding_box, forward_range, exceeding, level,
                min_elements, policy, box_policy);

            // Switch to two forward ranges, combine exceeding with
            // lower resp upper, but not lower/lower, upper/upper
            next_level2(exceeding_box, forward_range, exceeding, lower, level,
                min_elements, policy, box_policy);
            next_level2(exceeding_box, forward_range, exceeding, upper, level,
                min_elements, policy, box_policy);
        }

        // Recursively call operation both parts
        next_level(lower_box, forward_range, lower, level, min_elements,
                        policy, box_policy);
        next_level(upper_box, forward_range, upper, level, min_elements,
                        policy, box_policy);
    }
};

template
<
    int Dimension,
    typename Box,
    typename OverlapsPolicy1,
    typename OverlapsPolicy2,
    typename ExpandPolicy1,
    typename ExpandPolicy2,
    typename VisitBoxPolicy
>
class partition_two_ranges
{
    template
    <
        typename ForwardRange1,
        typename ForwardRange2,
        typename Policy,
        typename IteratorVector1,
        typename IteratorVector2
    >
    static inline void next_level(Box const& box,
            ForwardRange1 const& forward_range1,
            IteratorVector1 const& input1,
            ForwardRange2 const& forward_range2,
            IteratorVector2 const& input2,
            std::size_t level, std::size_t min_elements,
            Policy& policy, VisitBoxPolicy& box_policy)
    {
        partition_two_ranges
        <
            1 - Dimension,
            Box,
            OverlapsPolicy1,
            OverlapsPolicy2,
            ExpandPolicy1,
            ExpandPolicy2,
            VisitBoxPolicy
        >::apply(box, forward_range1, input1, forward_range2, input2,
                level + 1, min_elements,
                policy, box_policy);
    }

    template
    <
        typename ExpandPolicy,
        typename ForwardRange,
        typename IteratorVector
    >
    static inline Box get_new_box(ForwardRange const& forward_range,
                    IteratorVector const& input)
    {
        Box box;
        geometry::assign_inverse(box);
        expand_with_elements<ExpandPolicy>(box, forward_range, input);
        return box;
    }

    template
    <
        typename ForwardRange1,
        typename ForwardRange2,
        typename IteratorVector1,
        typename IteratorVector2
    >
    static inline Box get_new_box(ForwardRange1 const& forward_range1,
                    IteratorVector1 const& input1,
                    ForwardRange2 const& forward_range2,
                    IteratorVector2 const& input2)
    {
        Box box = get_new_box<ExpandPolicy1>(forward_range1, input1);
        expand_with_elements<ExpandPolicy2>(box, forward_range2, input2);
        return box;
    }

public :
    template
    <
        typename ForwardRange1,
        typename ForwardRange2,
        typename Policy,
        typename IteratorVector1,
        typename IteratorVector2
    >
    static inline void apply(Box const& box,
            ForwardRange1 const& forward_range1,
            IteratorVector1 const& input1,
            ForwardRange2 const& forward_range2,
            IteratorVector2 const& input2,
            std::size_t level,
            std::size_t min_elements,
            Policy& policy, VisitBoxPolicy& box_policy)
    {
        box_policy.apply(box, level);

        Box lower_box, upper_box;
        divide_box<Dimension>(box, lower_box, upper_box);

        IteratorVector1 lower1, upper1, exceeding1;
        IteratorVector2 lower2, upper2, exceeding2;
        divide_into_subsets<OverlapsPolicy1>(lower_box, upper_box,
                    forward_range1, input1, lower1, upper1, exceeding1);
        divide_into_subsets<OverlapsPolicy2>(lower_box, upper_box,
                    forward_range2, input2, lower2, upper2, exceeding2);

        if (boost::size(exceeding1) > 0)
        {
            // All exceeding from 1 with 2:

            if (recurse_ok(exceeding1, exceeding2, min_elements, level))
            {
                Box exceeding_box = get_new_box(forward_range1, exceeding1,
                            forward_range2, exceeding2);
                next_level(exceeding_box, forward_range1, exceeding1,
                                forward_range2, exceeding2, level,
                                min_elements, policy, box_policy);
            }
            else
            {
                handle_two(forward_range1, exceeding1, forward_range2,
                           exceeding2, policy);
            }

            // All exceeding from 1 with lower and upper of 2:

            // (Check sizes of all three forward ranges to avoid recurse into
            // the same combinations again and again)
            if (recurse_ok(lower2, upper2, exceeding1, min_elements, level))
            {
                Box exceeding_box
                    = get_new_box<ExpandPolicy1>(forward_range1, exceeding1);
                next_level(exceeding_box, forward_range1, exceeding1,
                    forward_range2, lower2, level, min_elements,
                    policy, box_policy);
                next_level(exceeding_box, forward_range1, exceeding1,
                    forward_range2, upper2, level, min_elements,
                    policy, box_policy);
            }
            else
            {
                handle_two(forward_range1, exceeding1, forward_range2, lower2,
                    policy);
                handle_two(forward_range1, exceeding1, forward_range2, upper2,
                    policy);
            }
        }

        if (boost::size(exceeding2) > 0)
        {
            // All exceeding from 2 with lower and upper of 1:
            if (recurse_ok(lower1, upper1, exceeding2, min_elements, level))
            {
                Box exceeding_box
                    = get_new_box<ExpandPolicy2>(forward_range2, exceeding2);
                next_level(exceeding_box, forward_range1, lower1,
                    forward_range2, exceeding2, level, min_elements,
                    policy, box_policy);
                next_level(exceeding_box, forward_range1, upper1,
                    forward_range2, exceeding2, level, min_elements,
                    policy, box_policy);
            }
            else
            {
                handle_two(forward_range1, lower1, forward_range2, exceeding2,
                    policy);
                handle_two(forward_range1, upper1, forward_range2, exceeding2,
                    policy);
            }
        }

        if (recurse_ok(lower1, lower2, min_elements, level))
        {
            next_level(lower_box, forward_range1, lower1,
                            forward_range2, lower2, level,
                            min_elements, policy, box_policy);
        }
        else
        {
            handle_two(forward_range1, lower1, forward_range2, lower2, policy);
        }
        if (recurse_ok(upper1, upper2, min_elements, level))
        {
            next_level(upper_box, forward_range1, upper1,
                            forward_range2, upper2, level,
                            min_elements, policy, box_policy);
        }
        else
        {
            handle_two(forward_range1, upper1, forward_range2, upper2, policy);
        }
    }
};

struct visit_no_policy
{
    template <typename Box>
    static inline void apply(Box const&, std::size_t )
    {}
};

struct include_all_policy
{
    template <typename Item>
    static inline bool apply(Item const&)
    {
        return true;
    }
};


}} // namespace detail::partition

template
<
    typename Box,
    typename ExpandPolicy1,
    typename OverlapsPolicy1,
    typename ExpandPolicy2 = ExpandPolicy1,
    typename OverlapsPolicy2 = OverlapsPolicy1,
    typename IncludePolicy1 = detail::partition::include_all_policy,
    typename IncludePolicy2 = detail::partition::include_all_policy,
    typename VisitBoxPolicy = detail::partition::visit_no_policy
>
class partition
{
    template
    <
        typename ExpandPolicy,
        typename IncludePolicy,
        typename ForwardRange,
        typename IteratorVector
    >
    static inline void expand_to_range(ForwardRange const& forward_range,
                Box& total, IteratorVector& iterator_vector)
    {
        for(typename boost::range_iterator<ForwardRange const>::type it
            = boost::begin(forward_range);
            it != boost::end(forward_range);
            ++it)
        {
            if (IncludePolicy::apply(*it))
            {
                ExpandPolicy::apply(total, *it);
                iterator_vector.push_back(it);
            }
        }
    }

public :
    template <typename ForwardRange, typename VisitPolicy>
    static inline void apply(ForwardRange const& forward_range,
            VisitPolicy& visitor,
            std::size_t min_elements = 16,
            VisitBoxPolicy box_visitor = detail::partition::visit_no_policy()
            )
    {
        typedef typename boost::range_iterator
            <
                ForwardRange const
            >::type iterator_type;

        if (std::size_t(boost::size(forward_range)) > min_elements)
        {
            std::vector<iterator_type> iterator_vector;
            Box total;
            assign_inverse(total);
            expand_to_range<ExpandPolicy1, IncludePolicy1>(forward_range,
                    total, iterator_vector);

            detail::partition::partition_one_range
                <
                    0, Box,
                    OverlapsPolicy1,
                    ExpandPolicy1,
                    VisitBoxPolicy
                >::apply(total, forward_range, iterator_vector, 0, min_elements,
                                visitor, box_visitor);
        }
        else
        {
            for(iterator_type it1 = boost::begin(forward_range);
                it1 != boost::end(forward_range);
                ++it1)
            {
                iterator_type it2 = it1;
                for(++it2; it2 != boost::end(forward_range); ++it2)
                {
                    visitor.apply(*it1, *it2);
                }
            }
        }
    }

    template
    <
        typename ForwardRange1,
        typename ForwardRange2,
        typename VisitPolicy
    >
    static inline void apply(ForwardRange1 const& forward_range1,
                ForwardRange2 const& forward_range2,
                VisitPolicy& visitor,
                std::size_t min_elements = 16,
                VisitBoxPolicy box_visitor
                    = detail::partition::visit_no_policy()
                )
    {
        typedef typename boost::range_iterator
            <
                ForwardRange1 const
            >::type iterator_type1;

        typedef typename boost::range_iterator
            <
                ForwardRange2 const
            >::type iterator_type2;

        if (std::size_t(boost::size(forward_range1)) > min_elements
            && std::size_t(boost::size(forward_range2)) > min_elements)
        {
            std::vector<iterator_type1> iterator_vector1;
            std::vector<iterator_type2> iterator_vector2;
            Box total;
            assign_inverse(total);
            expand_to_range<ExpandPolicy1, IncludePolicy1>(forward_range1,
                    total, iterator_vector1);
            expand_to_range<ExpandPolicy2, IncludePolicy2>(forward_range2,
                    total, iterator_vector2);

            detail::partition::partition_two_ranges
                <
                    0, Box, OverlapsPolicy1, OverlapsPolicy2,
                    ExpandPolicy1, ExpandPolicy2, VisitBoxPolicy
                >::apply(total,
                    forward_range1, iterator_vector1,
                    forward_range2, iterator_vector2,
                    0, min_elements, visitor, box_visitor);
        }
        else
        {
            for(iterator_type1 it1 = boost::begin(forward_range1);
                it1 != boost::end(forward_range1);
                ++it1)
            {
                for(iterator_type2 it2 = boost::begin(forward_range2);
                    it2 != boost::end(forward_range2);
                    ++it2)
                {
                    visitor.apply(*it1, *it2);
                }
            }
        }
    }
};


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_PARTITION_HPP
