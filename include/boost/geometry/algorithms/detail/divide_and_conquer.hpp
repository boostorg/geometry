// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_DIVIDE_AND_CONQUER_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_DIVIDE_AND_CONQUER_HPP

#include <vector>
#include <boost/range/algorithm/copy.hpp>
#include <boost/geometry/core/coordinate_type.hpp>

namespace boost { namespace geometry
{


namespace detail
{

// Helper structure
struct dac_index
{
    int index; // in original vector
    bool is_quad_exceeding;

    dac_index(int i = 0)
        : index(i)
        , is_quad_exceeding(false)
    {}
};


class avoid_duplicates_helper
{
    std::set<std::pair<int,int> > processed;

public :
    template <typename Item>
    inline bool apply(Item const& item1, Item const& item2)
    {
        bool done = false;
        if (item1.is_quad_exceeding && item2.is_quad_exceeding)
        {
            std::pair<int,int> p;
            if (item1.index < item2.index)
            {
                p.first = item1.index;
                p.second = item2.index;
            }
            else
            {
                p.first = item2.index;
                p.second = item1.index;
            }

            BOOST_AUTO(sit, processed.find(p));
            if (sit == boost::end(processed))
            {
                processed.insert(p);
            }
            else
            {
                done = true;
            }
        }
        return !done;
    }
};

template <typename Visitor>
class dac_static_visitor_avoiding_duplicates
{
    typedef std::vector<dac_index> dac_vector_type;
    typedef boost::range_iterator<dac_vector_type const>::type dac_iterator_type;

    avoid_duplicates_helper avoid_duplicates;
    Visitor& m_visitor;

public :
    dac_static_visitor_avoiding_duplicates(Visitor& visitor)
        : m_visitor(visitor)
    {}

    template <typename InputCollection>
    inline void apply(InputCollection const& collection, 
        dac_vector_type const& indexes) 
    {
        // Quadratic loop over the lowest quad
        for(dac_iterator_type it1 = boost::begin(indexes); it1 != boost::end(indexes); ++it1)
        {
            for(dac_iterator_type it2 = boost::begin(indexes); it2 != boost::end(indexes); ++it2)
            {
                // This policy always calls in order of original colection
                if (it1->index < it2->index)
                {
                    // This policy avoids handlig duplicates
                    if (avoid_duplicates.apply(*it1, *it2))
                    {
                        m_visitor.apply(collection[it1->index], collection[it2->index], 
                                it1->is_quad_exceeding, it2->is_quad_exceeding);
                    }
                }
            }
        }
    }
};


template
<
    int Dimension,
    typename Box,
    typename OverlapsPolicy
>
struct divide_and_conquer
{
    typedef typename coordinate_type<Box>::type ctype;

    template <typename InputCollection, typename Policy>
    static inline void next_level(Box const& box, 
            InputCollection const& collection,
            std::vector<dac_index> const& fitting_in_quad,
            std::vector<dac_index> exceeding_quad, // effectively const
            int level, int min_elements,
            Policy& policy)
    {
        typedef divide_and_conquer
                <
                    1 - Dimension,
                    Box,
                    OverlapsPolicy
                > sub_divide;

        if (level < 50
            && boost::size(fitting_in_quad) > min_elements
            && boost::size(fitting_in_quad) > boost::size(exceeding_quad)
            )
        {
            sub_divide::apply(box, collection, fitting_in_quad, exceeding_quad, level + 1, min_elements, policy);
        }
        else
        {
            boost::copy(fitting_in_quad, std::back_inserter(exceeding_quad));
            policy.apply(collection, exceeding_quad);
        }
    }

    template <typename InputCollection, typename Policy>
    static inline void apply(Box const& box,
            InputCollection const& collection,
            std::vector<dac_index> const& fitting_in_quad,
            std::vector<dac_index>& exceeding_quad, // by value is intentional
            int level,
            int min_elements,
            Policy& policy,
            int size = -1)
    {

        typedef typename boost::range_value<InputCollection>::type item_type;

        if (size == -1)
        {
            size = boost::size(collection);
        }

        // Divide input box into two parts, e.g. left/right
        ctype two = 2;
        ctype mid = (geometry::get<min_corner, Dimension>(box)
                + geometry::get<max_corner, Dimension>(box)) / two;

        Box lower = box, upper = box;
        geometry::set<max_corner, Dimension>(lower, mid);
        geometry::set<min_corner, Dimension>(upper, mid);

        // Divide collection into three subsets: lower, upper and oversized (not-fitting)
        // (lower == left or bottom, upper == right or top)
        std::vector<dac_index> lower_index, upper_index;
        bool has_exceeding = false;
        for(std::vector<dac_index>::const_iterator it = boost::begin(fitting_in_quad); it != boost::end(fitting_in_quad); ++it)
        {
            bool const overlaps_lower = OverlapsPolicy::apply(lower, collection[it->index]);
            bool const overlaps_upper = OverlapsPolicy::apply(upper, collection[it->index]);

            if (overlaps_lower && overlaps_upper)
            {
                dac_index oversized = *it;
                oversized.is_quad_exceeding = true;
                has_exceeding = true;
                exceeding_quad.push_back(oversized);
            }
            else if (overlaps_lower)
            {
                lower_index.push_back(*it);
            }
            else if (overlaps_upper)
            {
                upper_index.push_back(*it);
            }
            else
            {
                // Is nowhere! Should not occur!
                BOOST_ASSERT(true);
            }
        }

        // Recursively call operation both halves
        if (boost::size(lower_index) > 0 || has_exceeding)
        {
            next_level(lower, collection, lower_index, exceeding_quad, level, min_elements, policy);
        }
        if (boost::size(upper_index) > 0)
        {
            next_level(upper, collection, upper_index, exceeding_quad, level, min_elements, policy);
        }
    }
};
}


template
<
    typename Box,
    typename ExpandPolicy,
    typename OverlapsPolicy
>
struct divide_and_conquer
{

    template <typename InputCollection, typename Visitor>
    static inline void apply(InputCollection const& collection, Visitor& visitor, int min_elements = 16)
    {
        std::vector<detail::dac_index> index_vector, empty;


        // Determine total box
        Box total;
        assign_inverse(total);
        int index = 0;
        for(typename boost::range_iterator<InputCollection const>::type it
            = boost::begin(collection);
            it != boost::end(collection);
            ++it, ++index)
        {
            ExpandPolicy::apply(total, *it);
            index_vector.push_back(detail::dac_index(index));
        }

        // First call to recursive function
        detail::dac_static_visitor_avoiding_duplicates<Visitor> policy(visitor);
        detail::divide_and_conquer
            <
                0, Box, OverlapsPolicy
            >::apply(total, collection, index_vector, empty, 0, min_elements, policy);
    }
};


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_RING_IDENTIFIER_HPP
