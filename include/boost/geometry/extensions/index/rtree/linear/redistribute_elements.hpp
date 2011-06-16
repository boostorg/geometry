// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.Index - R-tree linear split algorithm implementation
//
// Copyright 2008 Federico J. Fernandez.
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_LINEAR_REDISTRIBUTE_ELEMENTS_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_LINEAR_REDISTRIBUTE_ELEMENTS_HPP

#include <algorithm>

#include <boost/tuple/tuple.hpp>

#include <boost/geometry/extensions/index/algorithms/area.hpp>
#include <boost/geometry/extensions/index/algorithms/union_area.hpp>

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

// from void find_normalized_separations(std::vector<Box> const& boxes, T& separation, unsigned int& first, unsigned int& second) const

template <typename Elements, typename Parameters, typename Translator, size_t DimensionIndex>
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
		const size_t elements_count = Parameters::max_elements + 1;
		BOOST_GEOMETRY_INDEX_ASSERT(elements.size() == elements_count, "unexpected number of elements");
		BOOST_STATIC_ASSERT(2 <= elements_count);

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

template <typename Elements, typename Parameters, typename Translator, size_t DimensionIndex>
struct pick_seeds_impl
{
    BOOST_STATIC_ASSERT(0 < DimensionIndex);

    typedef typename Elements::value_type element_type;
    typedef typename rtree::element_indexable_type<element_type, Translator>::type indexable_type;
    typedef typename index::traits::coordinate_type<indexable_type>::type coordinate_type;

    static inline void apply(Elements const& elements,
                             Translator const& tr,
                             coordinate_type & separation,
                             size_t & seed1,
                             size_t & seed2)
    {
        pick_seeds_impl<Elements, Parameters, Translator, DimensionIndex - 1>::apply(elements, tr, separation, seed1, seed2);

        coordinate_type current_separation;
        size_t s1, s2;
        find_greatest_normalized_separation<Elements, Parameters, Translator, DimensionIndex - 1>::apply(elements, tr, current_separation, s1, s2);

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
    typedef typename index::traits::coordinate_type<indexable_type>::type coordinate_type;

    static inline void apply(Elements const& elements,
                             Translator const& tr,
                             coordinate_type & separation,
                             size_t & seed1,
                             size_t & seed2)
    {
        find_greatest_normalized_separation<Elements, Parameters, Translator, 0>::apply(elements, tr, separation, seed1, seed2);
    }
};

// from void linear_pick_seeds(node_pointer const& n, unsigned int &seed1, unsigned int &seed2) const

template <typename Elements, typename Parameters, typename Translator>
struct pick_seeds
{
    typedef typename Elements::value_type element_type;
    typedef typename rtree::element_indexable_type<element_type, Translator>::type indexable_type;
    typedef typename index::traits::coordinate_type<indexable_type>::type coordinate_type;

    static const size_t dimension = index::traits::dimension<indexable_type>::value;

    static inline void apply(Elements const& elements,
							 Translator const& tr,
							 size_t & seed1,
							 size_t & seed2)
    {
        coordinate_type separation = 0;
        pick_seeds_impl<Elements, Parameters, Translator, dimension>::apply(elements, tr, separation, seed1, seed2);
    }
};

} // namespace linear

// from void split_node(node_pointer const& n, node_pointer& n1, node_pointer& n2) const

template <typename Value, typename Options, typename Translator, typename Box>
struct redistribute_elements<Value, Options, Translator, Box, linear_tag>
{
    typedef typename rtree::node<Value, typename Options::parameters_type, Box, typename Options::node_tag>::type node;
    typedef typename rtree::internal_node<Value, typename Options::parameters_type, Box, typename Options::node_tag>::type internal_node;
    typedef typename rtree::leaf<Value, typename Options::parameters_type, Box, typename Options::node_tag>::type leaf;

	typedef typename Options::parameters_type parameters_type;

    template <typename Node>
    static inline void apply(Node & n,
                             Node & second_node,
                             Box & box1,
                             Box & box2,
                             Translator const& tr)
    {
        typedef typename rtree::elements_type<Node>::type elements_type;
        typedef typename elements_type::value_type element_type;
        typedef typename rtree::element_indexable_type<element_type, Translator>::type indexable_type;
        typedef typename index::traits::coordinate_type<indexable_type>::type coordinate_type;
        typedef typename index::default_area_result<Box>::type area_type;

		elements_type & elements1 = rtree::elements(n);
		elements_type & elements2 = rtree::elements(second_node);
		const size_t elements1_count = parameters_type::max_elements + 1;

		BOOST_GEOMETRY_INDEX_ASSERT(elements1.size() == elements1_count, "unexpected number of elements");

		typedef boost::array<element_type, elements1_count> elements_copy_type;

		// copy original elements
		elements_copy_type elements_copy;
		std::copy(elements1.begin(), elements1.end(), elements_copy.begin());

        // calculate initial seeds
        size_t seed1 = 0;
        size_t seed2 = 0;
        linear::pick_seeds<elements_copy_type, parameters_type, Translator>::apply(elements_copy, tr, seed1, seed2);

        // prepare nodes' elements containers
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

        BOOST_STATIC_ASSERT(2 <= elements1_count);
        size_t remaining = elements1_count - 2;

        // redistribute the rest of the elements
        for ( size_t i = 0 ; i < elements1_count ; ++i )
        {
            if (i != seed1 && i != seed2)
            {
                element_type const& elem = elements_copy[i];
                indexable_type const& indexable = rtree::element_indexable(elem, tr);

                // if there is small number of elements left and the number of elements in node is lesser than min_elems
                // just insert them to this node
                if ( elements1.size() + remaining <= parameters_type::min_elements )
                {
                    elements1.push_back(elem);
                    geometry::expand(box1, indexable);
                    area1 = index::area(box1);
                }
                else if ( elements2.size() + remaining <= parameters_type::min_elements )
                {
                    elements2.push_back(elem);
                    geometry::expand(box2, indexable);
                    area2 = index::area(box2);
                }
                // choose better node and insert element
                else
                {
                    // calculate enlarged boxes and areas
                    Box enlarged_box1(box1);
                    Box enlarged_box2(box2);
                    geometry::expand(enlarged_box1, indexable);
                    geometry::expand(enlarged_box2, indexable);
                    area_type enlarged_area1 = index::area(enlarged_box1);
                    area_type enlarged_area2 = index::area(enlarged_box2);

                    area_type area_increase1 = enlarged_area1 - area1;
                    area_type area_increase2 = enlarged_area2 - area2;

                    // choose group which box area have to be enlarged least or has smaller area or has fewer elements
                    if ( area_increase1 < area_increase2 ||
                         ( area_increase1 == area_increase2 && area1 < area2 ) ||
                         ( area1 == area2 && elements1.size() <= elements2.size() ) )
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
