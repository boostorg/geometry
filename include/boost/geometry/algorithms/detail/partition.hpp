// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_PARTITION_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_PARTITION_HPP

#include <vector>
#include <boost/range/algorithm/copy.hpp>
#include <boost/geometry/core/coordinate_type.hpp>

namespace boost { namespace geometry
{


namespace detail
{

template
<
    int Dimension,
    typename Box,
    typename OverlapsPolicy
>
class partition
{
    typedef typename coordinate_type<Box>::type ctype;
    typedef std::vector<std::size_t> index_vector_type;
    typedef boost::range_iterator<index_vector_type const>::type index_iterator_type;
    typedef partition
            <
                1 - Dimension,
                Box,
                OverlapsPolicy
            > sub_divide;


    template <typename InputCollection, typename Policy>
    static inline void handle(InputCollection const& collection,
            index_vector_type const& input,
            Policy& policy)
    {
        // Quadratic behaviour at lowest level (lowest quad, or all exceeding)
        for(index_iterator_type it1 = boost::begin(input); it1 != boost::end(input); ++it1)
        {
            for(index_iterator_type it2 = boost::begin(input); it2 != boost::end(input); ++it2)
            {
                policy.apply(collection[*it1], collection[*it2]);
            }
        }
    }

    template <typename InputCollection, typename Policy>
    static inline void handle_exceeding(InputCollection const& collection,
            index_vector_type const& exceeding,
            index_vector_type const& input,
            Policy& policy)
    {
        if (boost::size(input) > 0)
        {
            for(index_iterator_type it1 = boost::begin(exceeding); it1 != boost::end(exceeding); ++it1)
            {
                for(index_iterator_type it2 = boost::begin(input); it2 != boost::end(input); ++it2)
                {
                    policy.apply(collection[*it1], collection[*it2]);
                }
            }
        }
    }

    template <typename InputCollection, typename Policy>
    static inline void next_level(Box const& box,
            InputCollection const& collection,
            index_vector_type const& input,
            int level, int min_elements,
            Policy& policy)
    {
        if (boost::size(input) > 0)
        {
            if (boost::size(input) > min_elements && level < 100)
            {
                sub_divide::apply(box, collection, input, level + 1, min_elements, policy);
            }
            else
            {
                handle(collection, input, policy);
            }
        }
    }

public :
    template <typename InputCollection, typename Policy>
    static inline void apply(Box const& box,
            InputCollection const& collection,
            index_vector_type const& input,
            int level,
            int min_elements,
            Policy& policy)
    {
        typedef typename boost::range_value<InputCollection>::type item_type;

        // Divide input box into two parts, e.g. left/right
        ctype two = 2;
        ctype mid = (geometry::get<min_corner, Dimension>(box)
                + geometry::get<max_corner, Dimension>(box)) / two;

        Box lower_box = box, upper_box = box;
        geometry::set<max_corner, Dimension>(lower_box, mid);
        geometry::set<min_corner, Dimension>(upper_box, mid);

        // Divide collection into three subsets: lower, upper and oversized (not-fitting)
        // (lower == left or bottom, upper == right or top)
        index_vector_type lower, upper, exceeding;
        for(index_iterator_type it = boost::begin(input);
            it != boost::end(input);
            ++it)
        {
            bool const lower_overlapping = OverlapsPolicy::apply(lower_box, collection[*it]);
            bool const upper_overlapping = OverlapsPolicy::apply(upper_box, collection[*it]);

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
                // Is nowhere! Should not occur!
                BOOST_ASSERT(true);
            }
        }

        if (boost::size(exceeding) > 0)
        {
            // All what is not fitting a partition should be combined
            // with each other, and with all which is fitting.
            handle(collection, exceeding, policy);
            handle_exceeding(collection, exceeding, lower, policy);
            handle_exceeding(collection, exceeding, upper, policy);
        }

        // Recursively call operation both halves
        next_level(lower_box, collection, lower, level, min_elements, policy);
        next_level(upper_box, collection, upper, level, min_elements, policy);
    }
};

}


template
<
    typename Box,
    typename ExpandPolicy,
    typename OverlapsPolicy
>
class partition
{
    typedef std::vector<std::size_t> index_vector_type;

public :
    template <typename InputCollection, typename Visitor>
    static inline void apply(InputCollection const& collection, Visitor& visitor, int min_elements = 16)
    {
        index_vector_type index_vector;

        // Determine total box
        Box total;
        assign_inverse(total);
        std::size_t index = 0;
        for(typename boost::range_iterator<InputCollection const>::type it
            = boost::begin(collection);
            it != boost::end(collection);
            ++it, ++index)
        {
            ExpandPolicy::apply(total, *it);
            index_vector.push_back(index);
        }

        // Start recursion
        detail::partition
            <
                0, Box, OverlapsPolicy
            >::apply(total, collection, index_vector, 0, min_elements, visitor);
    }
};


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_RING_IDENTIFIER_HPP
