// Boost.Geometry Index
//
// R-tree initial packing
//
// Copyright (c) 2011-2013 Adam Wulkiewicz, Lodz, Poland.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_INDEX_DETAIL_RTREE_PACK_CREATE_HPP
#define BOOST_GEOMETRY_INDEX_DETAIL_RTREE_PACK_CREATE_HPP

namespace boost { namespace geometry { namespace index { namespace detail { namespace rtree {

// STR leafs number are calculated as rcount/max
// and the number of splitting planes for each dimension as (count/max)^(1/dimension)
// <-> for dimension==2 -> sqrt(count/max)
//
// The main flaw of this algorithm is that the resulting tree will have bad structure for:
// 1. non-uniformly distributed elements
//      Statistic check could be performed, e.g. based on variance of lengths of elements edges for each dimension
// 2. elements distributed mainly along one axis
//      Calculate bounding box of all elements and then number of dividing planes for a dimension
//      from the length of BB edge for this dimension (more or less assuming that elements are uniformly-distributed squares)
//
// Another thing is that the last node may have less elements than Max or even Min.
// The number of splitting planes must be chosen more carefully than count/max
//
// This algorithm is something between STR and TGS
// it is more similar to the top-down recursive kd-tree creation algorithm
// using object median split and split axis of greatest BB edge
// BB is only used as a hint (assuming objects are distributed uniformly)

template <typename Value, typename Options, typename Translator, typename Box, typename Allocators>
class packer
{
    typedef typename rtree::node<Value, typename Options::parameters_type, Box, Allocators, typename Options::node_tag>::type node;
    typedef typename rtree::internal_node<Value, typename Options::parameters_type, Box, Allocators, typename Options::node_tag>::type internal_node;
    typedef typename rtree::leaf<Value, typename Options::parameters_type, Box, Allocators, typename Options::node_tag>::type leaf;

    typedef typename Allocators::node_pointer node_pointer;

    typedef typename traits::point_type<Box>::type point_type;
    typedef typename traits::coordinate_type<point_type>::type coordinate_type;
    typedef typename detail::default_content_result<Box>::type content_type;
    typedef typename Options::parameters_type parameters_type;
    static const std::size_t dimension = traits::dimension<point_type>::value;

    typedef typename rtree::container_from_elements_type<
        typename rtree::elements_type<leaf>::type,
        std::size_t
    >::type values_counts_container;

    typedef typename rtree::elements_type<internal_node>::type internal_elements;
    typedef typename internal_elements::value_type internal_element;

public:
    packer(parameters_type const& p, Translator const& tr, Allocators & al) : m_parameters(p), m_translator(tr), m_allocators(al) {}

    // Arbitrary iterators
    template <typename InIt>
    node_pointer pack(InIt first, InIt last)
    {
        typedef std::pair<point_type, InIt> entry_type;
        std::vector<entry_type> entries;

        // TODO if RandomAccess
        // count = distance(first, last);
        // reserve(count);

        Box hint_box;
        geometry::assign_inverse(hint_box);

        std::size_t count = 0;
        for ( ; first != last ; ++first, ++count )
        {
            geometry::expand(hint_box, m_translator(*first));

            point_type pt;
            geometry::centroid(m_translator(*first), pt);
            entries.push_back(std::make_pair(pt, first));
        }

        std::pair<std::size_t, std::size_t> subtree_counts = subtree_elements_counts(count);
        internal_element el = per_level(entries.begin(), entries.end(), hint_box, count, subtree_counts);
        return el.second;
    }

private:
    template <typename EIt>
    internal_element per_level(EIt first, EIt last, Box const& hint_box, std::size_t count, std::pair<std::size_t, std::size_t> const& subtree_counts)
    {
        // remove it later
        BOOST_ASSERT(first <= last); BOOST_ASSERT(last - first == count);

        if ( subtree_counts.first <= 1 )
        {
            // ROOT or LEAF
            BOOST_ASSERT(count <= m_parameters.get_max_elements());
            // if !root check m_parameters.get_min_elements() <= count

            // create new leaf node
            node_pointer n = rtree::create_node<Allocators, leaf>::apply(m_allocators);                     // MAY THROW (A)
            leaf & l = rtree::get<leaf>(*n);
            // reserve space for values
            rtree::elements(l).reserve(count);                                                              // MAY THROW (A)
            // calculate values box and copy values
            Box elements_box;
            geometry::assign_inverse(elements_box);
            for ( ; first != last ; ++first )
            {
                rtree::elements(l).push_back(*(first->second));                                             // MAY THROW (C)
                geometry::expand(elements_box, m_translator(*(first->second)));
            }
            return internal_element(elements_box, n);
        }

        // calculate numbers of values in subtrees
        values_counts_container values_counts;
        calculate_nodes_counts(values_counts, count, subtree_counts);                                       // MAY THROW (A)
        BOOST_ASSERT(are_nodes_counts_ok(values_counts, count, subtree_counts));
        
        // calculate next max and min subtree counts
        std::pair<std::size_t, std::size_t> next_subtree_counts = subtree_counts;
        next_subtree_counts.first /= m_parameters.get_max_elements();
        next_subtree_counts.second /= m_parameters.get_max_elements();

        // create new internal node
        node_pointer n = rtree::create_node<Allocators, internal_node>::apply(m_allocators);                // MAY THROW (A)
        internal_node & in = rtree::get<internal_node>(*n);
        // reserve space for values
        rtree::elements(in).reserve(values_counts.size());                                                  // MAY THROW (A)
        // calculate values box and copy values
        Box elements_box;
        geometry::assign_inverse(elements_box);

        per_level_packets(first, last, values_counts.begin(), values_counts.end(),
                          hint_box, next_subtree_counts,
                          rtree::elements(in), elements_box);

        return internal_element(elements_box, n);
    }

    template <typename EIt, typename CIt>
    void per_level_packets(EIt first, EIt last, CIt counts_first, CIt counts_last,
                           Box const& hint_box,
                           std::pair<std::size_t, std::size_t> const& next_subtree_counts,
                           internal_elements & elements, Box & elements_box)
    {
        std::size_t packets_count = counts_last - counts_first;

        BOOST_ASSERT( 0 < packets_count );

        if ( 1 == packets_count )
        {
            // the end, move to the next level
            internal_element el = per_level(first, last, hint_box, *counts_first, next_subtree_counts);
            elements.push_back(el);                                                                         // SHOULDN'T THROW (C)
            geometry::expand(elements_box, el.first);
            return;
        }
        
        CIt counts_median = counts_first + packets_count / 2;
        std::size_t left_sum = 0;
        for ( CIt it = counts_first ; it != counts_median ; ++it )
            left_sum += *it;

        EIt median = first + left_sum;

        coordinate_type greatest_length;
        std::size_t greatest_dim_index = 0;
        biggest_edge<dimension>::apply(hint_box, greatest_length, greatest_dim_index);
        partial_sort_for_dimension<0, dimension>::apply(first, median, last, greatest_dim_index);
        Box left, right;
        half_boxes_for_dimension<0, dimension>::apply(hint_box, left, right, greatest_dim_index);
        
        per_level_packets(first, median, counts_first, counts_median, left, next_subtree_counts, elements, elements_box);
        per_level_packets(median, last, counts_median, counts_last, right, next_subtree_counts, elements, elements_box);
    }

    std::pair<std::size_t, std::size_t> subtree_elements_counts(std::size_t elements_count)
    {
        std::pair<std::size_t, std::size_t> res = std::make_pair(1, 1);

        std::size_t smax = m_parameters.get_max_elements();
        std::size_t smin = m_parameters.get_min_elements();

        for ( ; smax < elements_count ; smax *= m_parameters.get_max_elements(), smin *= m_parameters.get_max_elements() )
        {
            res.first = smax;
            res.second = smin;
        }

        return res;
    }

    void calculate_nodes_counts(values_counts_container & values_counts,
                                std::size_t count,
                                std::pair<std::size_t, std::size_t> const& subtree_counts)
    {
        // e.g. for max = 5, min = 2, count = 52, subtree_max = 25, subtree_min = 10

        std::size_t n = count / subtree_counts.first; // e.g. 52 / 25 = 2
        std::size_t r = count % subtree_counts.first; // e.g. 52 % 25 = 2
        bool is_min_node = false;

        if ( 0 < r && r < subtree_counts.second ) // 2 < 10
        {
            is_min_node = true;
            std::size_t count_minus_min = count - subtree_counts.second; // e.g. 52 - 10 = 42
            n = count_minus_min / subtree_counts.first; // e.g. 42 / 25 = 1
            r = count_minus_min % subtree_counts.first; // e.g. 42 % 25 = 17
        }

        values_counts.resize(n, subtree_counts.first);                                                          // MAY THROW (A)
        if ( 0 < r )
            values_counts.push_back(r);                                                                         // MAY THROW (A)
        if ( is_min_node )
            values_counts.push_back(subtree_counts.second);                                                     // MAY THROW (A)
    }

    bool are_nodes_counts_ok(values_counts_container const& values_counts,
                             std::size_t count,
                             std::pair<std::size_t, std::size_t> const& subtree_counts)
    {
        std::size_t sum = 0;

        for(std::size_t i = 0 ; i < values_counts.size() ; ++i )
        {
            std::size_t c = values_counts[i];
            sum += c;
            if ( c < subtree_counts.second || subtree_counts.first < c )
                return false;
        }
        if ( sum != count )
            return false;
        return true;
    }

    template <std::size_t Dimension>
    struct biggest_edge
    {
        BOOST_STATIC_ASSERT(0 < Dimension);
        static inline void apply(Box const& box, coordinate_type & length, std::size_t & dim_index)
        {
            biggest_edge<Dimension-1>::apply(box, length, dim_index);
            coordinate_type curr = geometry::get<max_corner, Dimension-1>(box) - geometry::get<min_corner, Dimension-1>(box);
            if ( length < curr )
            {
                dim_index = Dimension - 1;
                length = curr;
            }
        }
    };

    template <>
    struct biggest_edge<1>
    {
        static inline void apply(Box const& box, coordinate_type & length, std::size_t & dim_index)
        {
            dim_index = 0;
            length = geometry::get<max_corner, 0>(box) - geometry::get<min_corner, 0>(box);
        }
    };

    template <std::size_t I, std::size_t Dimension>
    struct partial_sort_for_dimension
    {
        template <typename EIt>
        static inline void apply(EIt first, EIt median, EIt last, std::size_t dim_index)
        {
            if ( I == dim_index )
                std::partial_sort(first, median, last, point_entries_comparer<I>());
            else
                partial_sort_for_dimension<I+1, dimension>::apply(first, median, last, dim_index);
        }
    };

    template <std::size_t Dimension>
    struct partial_sort_for_dimension<Dimension, Dimension>
    {
        template <typename EIt>
        static inline void apply(EIt , EIt , EIt , std::size_t ) {}
    };

    template <std::size_t I>
    struct point_entries_comparer
    {
        template <typename typename PointEntry>
        bool operator()(PointEntry const& e1, PointEntry const& e2) const
        {
            return geometry::get<I>(e1.first) < geometry::get<I>(e2.first);
        }
    };

    template <std::size_t I, std::size_t Dimension>
    struct half_boxes_for_dimension
    {
        static inline void apply(Box const& box, Box & left, Box & right, std::size_t dim_index)
        {
            if ( I == dim_index )
            {
                geometry::convert(box, left);
                geometry::convert(box, right);
                coordinate_type median = geometry::get<min_corner, I>(box) + (geometry::get<max_corner, I>(box) - geometry::get<min_corner, I>(box)) / 2;
                geometry::set<max_corner, I>(left, median);
                geometry::set<min_corner, I>(right, median);
            }
            else
                half_boxes_for_dimension<I+1, dimension>::apply(box, left, right, dim_index);
        }
    };

    template <std::size_t Dimension>
    struct half_boxes_for_dimension<Dimension, Dimension>
    {
        static inline void apply(Box const& , Box & , Box & , std::size_t ) {}
    };

    parameters_type const& m_parameters;
    Translator const& m_translator;
    Allocators & m_allocators;
};

}}}}} // namespace boost::geometry::index::detail::rtree

#endif // BOOST_GEOMETRY_INDEX_DETAIL_RTREE_PACK_CREATE_HPP
