// Boost.Geometry Index
//
// R-tree linear split algorithm implementation
//
// Copyright (c) 2008 Federico J. Fernandez.
// Copyright (c) 2011-2013 Adam Wulkiewicz, Lodz, Poland.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_INDEX_DETAIL_RTREE_LINEAR_REDISTRIBUTE_ELEMENTS_HPP
#define BOOST_GEOMETRY_INDEX_DETAIL_RTREE_LINEAR_REDISTRIBUTE_ELEMENTS_HPP

#include <boost/type_traits/is_unsigned.hpp>

#include <boost/geometry/index/detail/algorithms/content.hpp>

#include <boost/geometry/index/detail/rtree/node/node.hpp>
#include <boost/geometry/index/detail/rtree/visitors/insert.hpp>
#include <boost/geometry/index/detail/rtree/visitors/is_leaf.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree {

namespace linear {

// TODO: awulkiew - there are loops inside find_greatest_normalized_separation::apply()
// iteration is done for each DimensionIndex.
// Separations and seeds for all DimensionIndex(es) could be calculated at once, stored, then the greatest would be choosen.

// TODO: Implement separate version for Points

// What if width calculated inside find_greatest_normalized_separation::apply() is near 0?
// What epsilon should be taken to calculation and what would be the value of resulting separation?

// from void find_normalized_separations(std::vector<Box> const& boxes, T& separation, unsigned int& first, unsigned int& second) const

template <typename Elements, typename Parameters, typename Translator, size_t DimensionIndex>
struct find_greatest_normalized_separation
{
    typedef typename Elements::value_type element_type;
    typedef typename rtree::element_indexable_type<element_type, Translator>::type indexable_type;
    typedef typename index::detail::traits::coordinate_type<indexable_type>::type coordinate_type;

    static inline void apply(Elements const& elements,
                             Parameters const& parameters,
                             Translator const& translator,
                             coordinate_type & separation,
                             size_t & seed1,
                             size_t & seed2)
    {
        const size_t elements_count = parameters.get_max_elements() + 1;
        BOOST_GEOMETRY_INDEX_ASSERT(elements.size() == elements_count, "unexpected number of elements");
        BOOST_GEOMETRY_INDEX_ASSERT(2 <= elements_count, "unexpected number of elements");

        // find the lowest low, highest high
        coordinate_type lowest_low = index::detail::get<min_corner, DimensionIndex>(rtree::element_indexable(elements[0], translator));
        coordinate_type highest_high = index::detail::get<max_corner, DimensionIndex>(rtree::element_indexable(elements[0], translator));
        // and the lowest high
        coordinate_type lowest_high = highest_high;
        size_t lowest_high_index = 0;
        for ( size_t i = 1 ; i < elements_count ; ++i )
        {
            coordinate_type min_coord = index::detail::get<min_corner, DimensionIndex>(rtree::element_indexable(elements[i], translator));
            coordinate_type max_coord = index::detail::get<max_corner, DimensionIndex>(rtree::element_indexable(elements[i], translator));

            if ( max_coord < lowest_high )
            {
                lowest_high = max_coord;
                lowest_high_index = i;
            }

            if ( min_coord < lowest_low )
                lowest_low = min_coord;

            if ( highest_high < max_coord )
                highest_high = max_coord;
        }

        // find the highest low
        size_t highest_low_index = lowest_high_index == 0 ? 1 : 0;
        coordinate_type highest_low = index::detail::get<min_corner, DimensionIndex>(rtree::element_indexable(elements[highest_low_index], translator));
        for ( size_t i = highest_low_index ; i < elements_count ; ++i )
        {
            coordinate_type min_coord = index::detail::get<min_corner, DimensionIndex>(rtree::element_indexable(elements[i], translator));
            if ( highest_low < min_coord &&
                 i != lowest_high_index )
            {
                highest_low = min_coord;
                highest_low_index = i;
            }
        }

        coordinate_type const width = highest_high - lowest_low;

        // TODO: awulkiew - following separation calculation has two flaws:
        // 1. for floating point numbers width should be compared witn some EPS
        // 2. separation calculation won't work for unsigned numbers
        //    but there should be possible to calculate negative value (cast to some floating point type?)

        // Temporary workaround
        BOOST_STATIC_ASSERT(!boost::is_unsigned<coordinate_type>::value);

        if ( width == 0 )
            separation = 0;
            // (highest_low - lowest_high) == 0
        else
            separation = (highest_low - lowest_high) / width;

        seed1 = highest_low_index;
        seed2 = lowest_high_index;

        BOOST_GEOMETRY_INDEX_DETAIL_USE_PARAM(parameters)
    }
};

template <typename Elements, typename Parameters, typename Translator, size_t Dimension>
struct pick_seeds_impl
{
    BOOST_STATIC_ASSERT(0 < Dimension);

    typedef typename Elements::value_type element_type;
    typedef typename rtree::element_indexable_type<element_type, Translator>::type indexable_type;
    typedef typename index::detail::traits::coordinate_type<indexable_type>::type coordinate_type;

    static inline void apply(Elements const& elements,
                             Parameters const& parameters,
                             Translator const& tr,
                             coordinate_type & separation,
                             size_t & seed1,
                             size_t & seed2)
    {
        pick_seeds_impl<Elements, Parameters, Translator, Dimension - 1>::apply(elements, parameters, tr, separation, seed1, seed2);

        coordinate_type current_separation;
        size_t s1, s2;
        find_greatest_normalized_separation<Elements, Parameters, Translator, Dimension - 1>::apply(elements, parameters, tr, current_separation, s1, s2);

        // in the old implementation different operator was used: <= (y axis prefered)
        if ( separation < current_separation )
        {
            separation = current_separation;
            seed1 = s1;
            seed2 = s2;
        }
    }
};

template <typename Elements, typename Parameters, typename Translator>
struct pick_seeds_impl<Elements, Parameters, Translator, 1>
{
    typedef typename Elements::value_type element_type;
    typedef typename rtree::element_indexable_type<element_type, Translator>::type indexable_type;
    typedef typename index::detail::traits::coordinate_type<indexable_type>::type coordinate_type;

    static inline void apply(Elements const& elements,
                             Parameters const& parameters,
                             Translator const& tr,
                             coordinate_type & separation,
                             size_t & seed1,
                             size_t & seed2)
    {
        find_greatest_normalized_separation<Elements, Parameters, Translator, 0>::apply(elements, parameters, tr, separation, seed1, seed2);
    }
};

// from void linear_pick_seeds(node_pointer const& n, unsigned int &seed1, unsigned int &seed2) const

template <typename Elements, typename Parameters, typename Translator>
struct pick_seeds
{
    typedef typename Elements::value_type element_type;
    typedef typename rtree::element_indexable_type<element_type, Translator>::type indexable_type;
    typedef typename index::detail::traits::coordinate_type<indexable_type>::type coordinate_type;

    static const size_t dimension = index::detail::traits::dimension<indexable_type>::value;

    static inline void apply(Elements const& elements,
                             Parameters const& parameters,
                             Translator const& tr,
                             size_t & seed1,
                             size_t & seed2)
    {
        coordinate_type separation = 0;
        pick_seeds_impl<Elements, Parameters, Translator, dimension>::apply(elements, parameters, tr, separation, seed1, seed2);
    }
};

} // namespace linear

// from void split_node(node_pointer const& n, node_pointer& n1, node_pointer& n2) const

template <typename Value, typename Options, typename Translator, typename Box, typename Allocators>
struct redistribute_elements<Value, Options, Translator, Box, Allocators, linear_tag>
{
    typedef typename Options::parameters_type parameters_type;

    typedef typename rtree::node<Value, parameters_type, Box, Allocators, typename Options::node_tag>::type node;
    typedef typename rtree::internal_node<Value, parameters_type, Box, Allocators, typename Options::node_tag>::type internal_node;
    typedef typename rtree::leaf<Value, parameters_type, Box, Allocators, typename Options::node_tag>::type leaf;

    template <typename Node>
    static inline void apply(Node & n,
                             Node & second_node,
                             Box & box1,
                             Box & box2,
                             parameters_type const& parameters,
                             Translator const& translator,
                             Allocators & allocators)
    {
        typedef typename rtree::elements_type<Node>::type elements_type;
        typedef typename elements_type::value_type element_type;
        typedef typename rtree::element_indexable_type<element_type, Translator>::type indexable_type;
        typedef typename index::detail::traits::coordinate_type<indexable_type>::type coordinate_type;
        typedef typename index::detail::default_content_result<Box>::type content_type;

        elements_type & elements1 = rtree::elements(n);
        elements_type & elements2 = rtree::elements(second_node);
        const size_t elements1_count = parameters.get_max_elements() + 1;

        BOOST_GEOMETRY_INDEX_ASSERT(elements1.size() == elements1_count, "unexpected number of elements");

        // copy original elements
        elements_type elements_copy(elements1);                                                             // MAY THROW, STRONG (alloc, copy)

        // calculate initial seeds
        size_t seed1 = 0;
        size_t seed2 = 0;
        linear::pick_seeds<
            elements_type,
            parameters_type,
            Translator
        >::apply(elements_copy, parameters, translator, seed1, seed2);

        // prepare nodes' elements containers
        elements1.clear();
        BOOST_GEOMETRY_INDEX_ASSERT(elements2.empty(), "unexpected container state");

        BOOST_TRY
        {
            // add seeds
            elements1.push_back(elements_copy[seed1]);                                                      // MAY THROW, STRONG (copy)
            elements2.push_back(elements_copy[seed2]);                                                      // MAY THROW, STRONG (alloc, copy)

            // calculate boxes
            geometry::convert(rtree::element_indexable(elements_copy[seed1], translator), box1);
            geometry::convert(rtree::element_indexable(elements_copy[seed2], translator), box2);

            // initialize areas
            content_type content1 = index::detail::content(box1);
            content_type content2 = index::detail::content(box2);

            BOOST_GEOMETRY_INDEX_ASSERT(2 <= elements1_count, "unexpected elements number");
            size_t remaining = elements1_count - 2;

            // redistribute the rest of the elements
            for ( size_t i = 0 ; i < elements1_count ; ++i )
            {
                if (i != seed1 && i != seed2)
                {
                    element_type const& elem = elements_copy[i];
                    indexable_type const& indexable = rtree::element_indexable(elem, translator);

                    // if there is small number of elements left and the number of elements in node is lesser than min_elems
                    // just insert them to this node
                    if ( elements1.size() + remaining <= parameters.get_min_elements() )
                    {
                        elements1.push_back(elem);                                                          // MAY THROW, STRONG (copy)
                        geometry::expand(box1, indexable);
                        content1 = index::detail::content(box1);
                    }
                    else if ( elements2.size() + remaining <= parameters.get_min_elements() )
                    {
                        elements2.push_back(elem);                                                          // MAY THROW, STRONG (alloc, copy)
                        geometry::expand(box2, indexable);
                        content2 = index::detail::content(box2);
                    }
                    // choose better node and insert element
                    else
                    {
                        // calculate enlarged boxes and areas
                        Box enlarged_box1(box1);
                        Box enlarged_box2(box2);
                        geometry::expand(enlarged_box1, indexable);
                        geometry::expand(enlarged_box2, indexable);
                        content_type enlarged_content1 = index::detail::content(enlarged_box1);
                        content_type enlarged_content2 = index::detail::content(enlarged_box2);

                        content_type content_increase1 = enlarged_content1 - content1;
                        content_type content_increase2 = enlarged_content2 - content2;

                        // choose group which box content have to be enlarged least or has smaller content or has fewer elements
                        if ( content_increase1 < content_increase2 ||
                                ( content_increase1 == content_increase2 &&
                                    ( content1 < content2 ||
                                        ( content1 == content2 && elements1.size() <= elements2.size() ) ) ) )
                        {
                            elements1.push_back(elem);                                                      // MAY THROW, STRONG (copy)
                            box1 = enlarged_box1;
                            content1 = enlarged_content1;
                        }
                        else
                        {
                            elements2.push_back(elem);                                                      // MAY THROW, STRONG (alloc, copy)
                            box2 = enlarged_box2;
                            content2 = enlarged_content2;
                        }
                    }
                
                    BOOST_GEOMETRY_INDEX_ASSERT(0 < remaining, "unexpected value");
                    --remaining;
                }
            }
        }
        BOOST_CATCH(...)
        {
            elements1.clear();
            elements2.clear();

            rtree::destroy_elements<Value, Options, Translator, Box, Allocators>::apply(elements_copy, allocators);
            //elements_copy.clear();

            BOOST_RETHROW                                                                                     // RETHROW, BASIC
        }
        BOOST_CATCH_END
    }
};

}} // namespace detail::rtree::visitors

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_INDEX_DETAIL_RTREE_LINEAR_REDISTRIBUTE_ELEMENTS_HPP
