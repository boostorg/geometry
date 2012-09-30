// Boost.Geometry Index
//
// R-tree linear split algorithm implementation
//
// Copyright (c) 2008 Federico J. Fernandez.
// Copyright (c) 2011-2012 Adam Wulkiewicz, Lodz, Poland.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_LINEAR_REDISTRIBUTE_ELEMENTS_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_LINEAR_REDISTRIBUTE_ELEMENTS_HPP

#include <boost/type_traits/is_unsigned.hpp>

#include <boost/geometry/extensions/index/algorithms/content.hpp>
#include <boost/geometry/extensions/index/algorithms/union_content.hpp>

#include <boost/geometry/extensions/index/rtree/node/node.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/insert.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/is_leaf.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree { namespace visitors {

namespace detail {

namespace linear {

// TODO: awulkiew - there are loops inside find_greatest_normalized_separation::apply()
// iteration is done for each DimensionIndex.
// Separations and seeds for all DimensionIndex(es) could be calculated at once, stored, then the greatest would be choosen.

// TODO: Implement separate version for Points

// What if width calculated inside find_greatest_normalized_separation::apply() is near 0?
// What epsilon should be taken to calculation and what would be the value of resulting separation?

// from void find_normalized_separations(std::vector<Box> const& boxes, T& separation, unsigned int& first, unsigned int& second) const

template <typename Elements, typename NodeProxy, size_t DimensionIndex>
struct find_greatest_normalized_separation
{
    typedef typename Elements::value_type element_type;
    typedef typename rtree::element_indexable_type<element_type, typename NodeProxy::translator_type>::type indexable_type;
    typedef typename index::traits::coordinate_type<indexable_type>::type coordinate_type;

    static inline void apply(Elements const& elements,
                             NodeProxy const& node_proxy,
                             coordinate_type & separation,
                             size_t & seed1,
                             size_t & seed2)
    {
		const size_t elements_count = node_proxy.parameters().get_max_elements() + 1;
		BOOST_GEOMETRY_INDEX_ASSERT(elements.size() == elements_count, "unexpected number of elements");
		BOOST_GEOMETRY_INDEX_ASSERT(2 <= elements_count, "unexpected number of elements");

        // find the lowest low, highest high
        coordinate_type lowest_low = index::get<min_corner, DimensionIndex>(node_proxy.indexable(elements[0]));
        coordinate_type highest_high = index::get<max_corner, DimensionIndex>(node_proxy.indexable(elements[0]));
        // and the lowest high
        coordinate_type lowest_high = highest_high;
        size_t lowest_high_index = 0;
        for ( size_t i = 1 ; i < elements_count ; ++i )
        {
            coordinate_type min_coord = index::get<min_corner, DimensionIndex>(node_proxy.indexable(elements[i]));
            coordinate_type max_coord = index::get<max_corner, DimensionIndex>(node_proxy.indexable(elements[i]));

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
        coordinate_type highest_low = index::get<min_corner, DimensionIndex>(node_proxy.indexable(elements[highest_low_index]));
        for ( size_t i = highest_low_index ; i < elements_count ; ++i )
        {
            coordinate_type min_coord = index::get<min_corner, DimensionIndex>(node_proxy.indexable(elements[i]));
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
    }
};

template <typename Elements, typename NodeProxy, size_t Dimension>
struct pick_seeds_impl
{
    BOOST_STATIC_ASSERT(0 < Dimension);

    typedef typename Elements::value_type element_type;
    typedef typename rtree::element_indexable_type<element_type, typename NodeProxy::translator_type>::type indexable_type;
    typedef typename index::traits::coordinate_type<indexable_type>::type coordinate_type;

    static inline void apply(Elements const& elements,
                             NodeProxy const& node_proxy,
                             coordinate_type & separation,
                             size_t & seed1,
                             size_t & seed2)
    {
        pick_seeds_impl<Elements, NodeProxy, Dimension - 1>::apply(elements, node_proxy, separation, seed1, seed2);

        coordinate_type current_separation;
        size_t s1, s2;
        find_greatest_normalized_separation<Elements, NodeProxy, Dimension - 1>::apply(elements, node_proxy, current_separation, s1, s2);

        // in the old implementation different operator was used: <= (y axis prefered)
        if ( separation < current_separation )
        {
            separation = current_separation;
            seed1 = s1;
            seed2 = s2;
        }
    }
};

template <typename Elements, typename NodeProxy>
struct pick_seeds_impl<Elements, NodeProxy, 1>
{
    typedef typename Elements::value_type element_type;
    typedef typename rtree::element_indexable_type<element_type, typename NodeProxy::translator_type>::type indexable_type;
    typedef typename index::traits::coordinate_type<indexable_type>::type coordinate_type;

    static inline void apply(Elements const& elements,
                             NodeProxy const& node_proxy,
                             coordinate_type & separation,
                             size_t & seed1,
                             size_t & seed2)
    {
        find_greatest_normalized_separation<Elements, NodeProxy, 0>::apply(elements, node_proxy, separation, seed1, seed2);
    }
};

// from void linear_pick_seeds(node_pointer const& n, unsigned int &seed1, unsigned int &seed2) const

template <typename Elements, typename NodeProxy>
struct pick_seeds
{
    typedef typename Elements::value_type element_type;
    typedef typename rtree::element_indexable_type<element_type, typename NodeProxy::translator_type>::type indexable_type;
    typedef typename index::traits::coordinate_type<indexable_type>::type coordinate_type;

    static const size_t dimension = index::traits::dimension<indexable_type>::value;

    static inline void apply(Elements const& elements,
                             NodeProxy const& node_proxy,
							 size_t & seed1,
							 size_t & seed2)
    {
        coordinate_type separation = 0;
        pick_seeds_impl<Elements, NodeProxy, dimension>::apply(elements, node_proxy, separation, seed1, seed2);
    }
};

} // namespace linear

// from void split_node(node_pointer const& n, node_pointer& n1, node_pointer& n2) const

template <typename Value, typename NodeProxy>
struct redistribute_elements<Value, NodeProxy, linear_tag>
{
    typedef typename NodeProxy::node node;
    typedef typename NodeProxy::internal_node internal_node;
    typedef typename NodeProxy::leaf leaf;
    typedef typename NodeProxy::box_type box_type;

    template <typename Node>
    static inline void apply(Node & n,
                             Node & second_node,
                             box_type & box1,
                             box_type & box2,
                             NodeProxy const& node_proxy)
    {
        typedef typename rtree::elements_type<Node>::type elements_type;
        typedef typename elements_type::value_type element_type;
        typedef typename rtree::element_indexable_type<element_type, typename NodeProxy::translator_type>::type indexable_type;
        typedef typename index::traits::coordinate_type<indexable_type>::type coordinate_type;
        typedef typename index::default_content_result<box_type>::type content_type;

		elements_type & elements1 = rtree::elements(n);
		elements_type & elements2 = rtree::elements(second_node);
		const size_t elements1_count = node_proxy.parameters().get_max_elements() + 1;

		BOOST_GEOMETRY_INDEX_ASSERT(elements1.size() == elements1_count, "unexpected number of elements");

		// copy original elements
		elements_type elements_copy(elements1);

        // calculate initial seeds
        size_t seed1 = 0;
        size_t seed2 = 0;
        linear::pick_seeds<elements_type, NodeProxy>::apply(elements_copy, node_proxy, seed1, seed2);

        // prepare nodes' elements containers
        elements1.clear();
        BOOST_GEOMETRY_INDEX_ASSERT(elements2.empty(), "unexpected container state");

        // add seeds
        elements1.push_back(elements_copy[seed1]);
        elements2.push_back(elements_copy[seed2]);

        // calculate boxes
        geometry::convert(node_proxy.indexable(elements_copy[seed1]), box1);
        geometry::convert(node_proxy.indexable(elements_copy[seed2]), box2);

        // initialize areas
        content_type content1 = index::content(box1);
        content_type content2 = index::content(box2);

        BOOST_GEOMETRY_INDEX_ASSERT(2 <= elements1_count, "unexpected elements number");
        size_t remaining = elements1_count - 2;

        // redistribute the rest of the elements
        for ( size_t i = 0 ; i < elements1_count ; ++i )
        {
            if (i != seed1 && i != seed2)
            {
                element_type const& elem = elements_copy[i];
                // TODO - here indexable by value may be returned
                indexable_type const& indexable = node_proxy.indexable(elem);

                // if there is small number of elements left and the number of elements in node is lesser than min_elems
                // just insert them to this node
                if ( elements1.size() + remaining <= node_proxy.parameters().get_min_elements() )
                {
                    elements1.push_back(elem);
                    geometry::expand(box1, indexable);
                    content1 = index::content(box1);
                }
                else if ( elements2.size() + remaining <= node_proxy.parameters().get_min_elements() )
                {
                    elements2.push_back(elem);
                    geometry::expand(box2, indexable);
                    content2 = index::content(box2);
                }
                // choose better node and insert element
                else
                {
                    // calculate enlarged boxes and areas
                    box_type enlarged_box1(box1);
                    box_type enlarged_box2(box2);
                    geometry::expand(enlarged_box1, indexable);
                    geometry::expand(enlarged_box2, indexable);
                    content_type enlarged_content1 = index::content(enlarged_box1);
                    content_type enlarged_content2 = index::content(enlarged_box2);

                    content_type content_increase1 = enlarged_content1 - content1;
                    content_type content_increase2 = enlarged_content2 - content2;

                    // choose group which box content have to be enlarged least or has smaller content or has fewer elements
                    if ( content_increase1 < content_increase2 ||
                         ( content_increase1 == content_increase2 && content1 < content2 ) ||
                         ( content1 == content2 && elements1.size() <= elements2.size() ) )
                    {
                        elements1.push_back(elem);
                        box1 = enlarged_box1;
                        content1 = enlarged_content1;
                    }
                    else
                    {
                        elements2.push_back(elem);
                        box2 = enlarged_box2;
                        content2 = enlarged_content2;
                    }
                }
                
                assert(0 < remaining);
                --remaining;
            }
        }
    }
};

} // namespace detail

}}} // namespace detail::rtree::visitors

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_LINEAR_REDISTRIBUTE_ELEMENTS_HPP
