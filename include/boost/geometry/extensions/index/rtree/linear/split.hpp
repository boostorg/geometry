// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.Index - R*-tree split algorithm implementation
//
// Copyright 2008 Federico J. Fernandez.
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_LINEAR_SPLIT_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_LINEAR_SPLIT_HPP

#include <algorithm>

#include <boost/tuple/tuple.hpp>

#include <boost/geometry/extensions/index/algorithms/area.hpp>
#include <boost/geometry/extensions/index/algorithms/union_area.hpp>

#include <boost/geometry/extensions/index/rtree/node.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/insert.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/is_leaf.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree { namespace linear {

// from void find_normalized_separations(std::vector<Box> const& boxes, T& separation, unsigned int& first, unsigned int& second) const

template <typename Elements, typename Translator, size_t DimensionIndex>
struct find_greatest_normalized_separation
{
    typedef typename Elements::value_type element_type;
    typedef typename rtree::element_indexable_type<element_type, Translator>::type indexable_type;
    typedef typename index::traits::coordinate_type<indexable_type>::type coordinate_type;

    static inline void apply(Elements const& elements,
                             Translator const& tr,
                             coordinate_type & separation,
                             size_t & seed1,
                             size_t & seed2)
    {
        size_t elements_count = elements.size();

        assert(2 <= elements_count);

        // find the lowest low, highest high
        coordinate_type lowest_low = index::get<min_corner, DimensionIndex>(rtree::element_indexable(elements[0], tr));
        coordinate_type highest_high = index::get<max_corner, DimensionIndex>(rtree::element_indexable(elements[0], tr));
        // and the lowest high
        coordinate_type lowest_high = highest_high;
        size_t lowest_high_index = 0;
        for ( size_t i = 1 ; i < elements_count ; ++i )
        {
            coordinate_type min_coord = index::get<min_corner, DimensionIndex>(rtree::element_indexable(elements[i], tr));
            coordinate_type max_coord = index::get<max_corner, DimensionIndex>(rtree::element_indexable(elements[i], tr));

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
        coordinate_type highest_low = index::get<min_corner, DimensionIndex>(rtree::element_indexable(elements[highest_low_index], tr));
        for ( size_t i = highest_low_index ; i < elements_count ; ++i )
        {
            coordinate_type min_coord = index::get<min_corner, DimensionIndex>(rtree::element_indexable(elements[i], tr));
            if ( highest_low < min_coord &&
                 i != lowest_high_index )
            {
                highest_low = min_coord;
                highest_low_index = i;
            }
        }

        coordinate_type const width = highest_high - lowest_low;

        separation = (highest_low - lowest_high) / width;
        seed1 = highest_low_index;
        seed2 = lowest_high_index;
    }
};

namespace dispatch {

template <typename Elements, typename Translator, size_t DimensionIndex>
struct choose_axis_impl
{
    BOOST_STATIC_ASSERT(0 < DimensionIndex);

    typedef typename Elements::value_type element_type;
    typedef typename rtree::element_indexable_type<element_type, Translator>::type indexable_type;
    typedef typename index::traits::coordinate_type<indexable_type>::type coordinate_type;

    static inline void apply(Elements const& elements,
                             Translator const& tr,
                             size_t & axis,
                             coordinate_type & separation,
                             size_t & seed1,
                             size_t & seed2)
    {
        choose_axis_impl<Elements, Translator, DimensionIndex - 1>::apply(elements, tr, axis, separation, seed1, seed2);

        coordinate_type current_separation;
        size_t s1, s2;
        find_greatest_normalized_separation<Elements, Translator, DimensionIndex - 1>::apply(elements, tr, current_separation, s1, s2);

        // TODO: operator test!, change <= to < later
        if ( separation <= current_separation )
        {
            separation = current_separation;
            seed1 = s1;
            seed2 = s2;
            axis = DimensionIndex - 1;
        }
    }
};

template <typename Elements, typename Translator>
struct choose_axis_impl<Elements, Translator, 1>
{
    typedef typename Elements::value_type element_type;
    typedef typename rtree::element_indexable_type<element_type, Translator>::type indexable_type;
    typedef typename index::traits::coordinate_type<indexable_type>::type coordinate_type;

    static inline void apply(Elements const& elements,
                             Translator const& tr,
                             size_t & axis,
                             coordinate_type & separation,
                             size_t & seed1,
                             size_t & seed2)
    {
        find_greatest_normalized_separation<Elements, Translator, 0>::apply(elements, tr, separation, seed1, seed2);
        axis = 0;
    }
};

} // namespace dispatch

// from void linear_pick_seeds(node_pointer const& n, unsigned int &seed1, unsigned int &seed2) const

template <typename Elements, typename Translator>
struct choose_axis
{
    typedef typename Elements::value_type element_type;
    typedef typename rtree::element_indexable_type<element_type, Translator>::type indexable_type;
    typedef typename index::traits::coordinate_type<indexable_type>::type coordinate_type;
    
    static const size_t dimension = index::traits::dimension<indexable_type>::value;

    static inline size_t apply(Elements const& elements,
                               Translator const& tr,
                               size_t & seed1,
                               size_t & seed2)
    {
        size_t axis = 0;
        coordinate_type separation = 0;
        dispatch::choose_axis_impl<Elements, Translator, dimension>::apply(elements, tr, axis, separation, seed1, seed2);
        return axis;
    }
};

// from void split_node(node_pointer const& n, node_pointer& n1, node_pointer& n2) const

template <typename Value, typename Translator, typename Box>
struct redistribute_elements
{
    typedef typename rtree::node<Value, Box, rstar_tag>::type node;
    typedef typename rtree::internal_node<Value, Box, rstar_tag>::type internal_node;
    typedef typename rtree::leaf<Value, Box, rstar_tag>::type leaf;

    template <typename Node>
    static inline void apply(Node & n,
                             Node & second_node,
                             Box & box1,
                             Box & box2,
                             size_t min_elems,
                             size_t max_elems,
                             Translator const& tr)
    {
        typedef typename rtree::elements_type<Node>::type elements_type;
        typedef typename elements_type::value_type element_type;
        typedef typename rtree::element_indexable_type<element_type, Translator>::type indexable_type;
        typedef typename index::traits::coordinate_type<indexable_type>::type coordinate_type;
        typedef typename index::default_area_result<Box>::type area_type;
        
        static const size_t dimension = index::traits::dimension<indexable_type>::value;

        // copy original elements
        elements_type elements_copy = rtree::elements_get(n);
        size_t elements_count = elements_copy.size();

        // calculate initial seeds
        size_t seed1 = 0;
        size_t seed2 = 0;
        choose_axis<elements_type, Translator>::apply(elements_copy, tr, seed1, seed2);

        // prepare nodes' elements containers
        elements_type & elements1 = rtree::elements_get(n);
        elements_type & elements2 = rtree::elements_get(second_node);
        elements1.clear();
        assert(elements2.empty());

        // add seeds
        elements1.push_back(elements_copy[seed1]);
        elements2.push_back(elements_copy[seed2]);

        // calculate boxes
        geometry::convert(rtree::element_indexable(elements_copy[seed1], tr), box1);
        geometry::convert(rtree::element_indexable(elements_copy[seed2], tr), box2);

        // initialize areas
        area_type area1 = index::area(box1);
        area_type area2 = index::area(box2);

        assert(2 <= elements_count);
        size_t remaining = elements_count - 2;

        // redistribute the rest of the elements
        for ( size_t i = 0 ; i < elements_count ; ++i )
        {
            if (i != seed1 && i != seed2)
            {
                element_type const& elem = elements_copy[i];
                indexable_type const& indexable = rtree::element_indexable(elem, tr);

                // TODO: awulkiew - is this needed?
                if ( elements1.size() + remaining == min_elems )
                {
                    elements1.push_back(elem);
                    geometry::expand(box1, indexable);
                    area1 = index::area(box1);
                    continue;
                }
                if ( elements2.size() + remaining == min_elems )
                {
                    elements2.push_back(elem);
                    geometry::expand(box2, indexable);
                    area2 = index::area(box2);
                    continue;
                }

                assert(0 < remaining);
                remaining--;

                // calculate enlarged boxes and areas
                Box enlarged_box1(box1);
                Box enlarged_box2(box2);
                geometry::expand(enlarged_box1, indexable);
                geometry::expand(enlarged_box2, indexable);
                area_type enlarged_area1 = index::area(enlarged_box1);
                area_type enlarged_area2 = index::area(enlarged_box2);

                area_type areas_diff1 = enlarged_area1 - area1;
                area_type areas_diff2 = enlarged_area2 - area2;

                // choose group which box area have to be enlarged least
                if ( areas_diff1 < areas_diff2 )
                {
                    elements1.push_back(elem);
                    box1 = enlarged_box1;
                    area1 = enlarged_area1;
                }
                else if ( areas_diff2 < areas_diff1 )
                {
                    elements2.push_back(elem);
                    box2 = enlarged_box2;
                    area2 = enlarged_area2;
                }
                else
                {
                    // choose group which box has smaller area
                    if ( area1 < area2 )
                    {
                        elements1.push_back(elem);
                        box1 = enlarged_box1;
                        area1 = enlarged_area1;
                    }
                    else if ( area2 < area1 )
                    {
                        elements2.push_back(elem);
                        box2 = enlarged_box2;
                        area2 = enlarged_area2;
                    }
                    else
                    {
                        // choose group with fewer elements
                        if ( elements1.size() <= elements2.size() )
                        {
                            elements1.push_back(elem);
                            box1 = enlarged_box1;
                            area1 = enlarged_area1;
                        }
                        else
                        {
                            elements2.push_back(elem);
                            box2 = enlarged_box2;
                            area2 = enlarged_area2;
                        }
                    }
                }
            }
        }
    }
};

// split

template <typename Value, typename Translator, typename Box>
class split
{
    typedef typename rtree::node<Value, Box, linear_tag>::type node;
    typedef typename rtree::internal_node<Value, Box, linear_tag>::type internal_node;
    typedef typename rtree::leaf<Value, Box, linear_tag>::type leaf;

    static const size_t dimension = index::traits::dimension<Box>::value;

public:
    template <typename Node>
    static inline void apply(
        Node & n,
        internal_node *parent,
        size_t current_child_index,
        node *& root,
        size_t min_elems,
        size_t max_elems,
        Translator const& tr)
    {
        node * second_node = rtree::create_node(Node());
        
        // redistribute elements
        Box box1, box2;
        linear::redistribute_elements<Value, Translator, Box>::
            apply(n, boost::get<Node>(*second_node), box1, box2, min_elems, max_elems, tr);

        // node is not the root
        if ( parent != 0 )
        {
            // update old node's box
            parent->children[current_child_index].first = box1;
            // add new node to the parent's children
            parent->children.push_back(std::make_pair(box2, second_node));
        }
        // node is the root
        else
        {
            assert(&n == boost::get<Node>(root));

            // create new root and add nodes
            node * new_root = rtree::create_node(internal_node());

            boost::get<internal_node>(*new_root).children.push_back(std::make_pair(box1, root));
            boost::get<internal_node>(*new_root).children.push_back(std::make_pair(box2, second_node));

            root = new_root;
        }
    }
};

}}} // namespace detail::rtree:linear

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_LINEAR_SPLIT_HPP
