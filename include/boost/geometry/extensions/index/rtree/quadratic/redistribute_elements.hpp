// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.Index - R-tree quadratic split algorithm implementation
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_QUADRATIC_REDISTRIBUTE_ELEMENTS_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_QUADRATIC_REDISTRIBUTE_ELEMENTS_HPP

#include <algorithm>

#include <boost/geometry/extensions/index/algorithms/area.hpp>
#include <boost/geometry/extensions/index/algorithms/union_area.hpp>

#include <boost/geometry/extensions/index/rtree/node/node.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/insert.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/is_leaf.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree { namespace visitors {

namespace detail {

namespace quadratic {

template <typename Elements, typename Translator, typename Box>
struct pick_seeds
{
    typedef typename Elements::value_type element_type;
    typedef typename rtree::element_indexable_type<element_type, Translator>::type indexable_type;
    typedef typename index::traits::coordinate_type<indexable_type>::type coordinate_type;
    typedef Box box_type;
    typedef typename index::default_area_result<box_type>::type area_type;

    static inline void apply(Elements const& elements,
                             Translator const& tr,
                             size_t & seed1,
                             size_t & seed2)
    {
        size_t elements_count = elements.size();

		BOOST_GEOMETRY_INDEX_ASSERT(2 <= elements_count, "wrong number of elements");

        area_type greatest_free_area = 0;
        seed1 = 0;
        seed2 = 1;

        for ( size_t i = 0 ; i < elements_count - 1 ; ++i )
        {
            for ( size_t j = i + 1 ; j < elements_count ; ++j )
            {
                indexable_type const& ind1 = rtree::element_indexable(elements[i], tr);
                indexable_type const& ind2 = rtree::element_indexable(elements[j], tr);

                box_type enlarged_box;
                geometry::convert(ind1, enlarged_box);
                geometry::expand(enlarged_box, ind2);

                area_type free_area = index::area(enlarged_box) - index::area(ind1) - index::area(ind2);
                
                if ( greatest_free_area < free_area )
                {
                    greatest_free_area = free_area;
                    seed1 = i;
                    seed2 = j;
                }
            }
        }
    }
};

} // namespace quadratic

template <typename Value, typename Options, typename Translator, typename Box>
struct redistribute_elements<Value, Options, Translator, Box, quadratic_tag>
{
    typedef typename rtree::node<Value, typename Options::parameters_type, Box, typename Options::node_tag>::type node;
    typedef typename rtree::internal_node<Value, typename Options::parameters_type, Box, typename Options::node_tag>::type internal_node;
    typedef typename rtree::leaf<Value, typename Options::parameters_type, Box, typename Options::node_tag>::type leaf;

    typedef typename index::default_area_result<Box>::type area_type;

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

        // copy original elements
        elements_type elements_copy = rtree::elements(n);
        
        // calculate initial seeds
        size_t seed1 = 0;
        size_t seed2 = 0;
        quadratic::pick_seeds<elements_type, Translator, Box>::apply(elements_copy, tr, seed1, seed2);

        // prepare nodes' elements containers
        elements_type & elements1 = rtree::elements(n);
        elements_type & elements2 = rtree::elements(second_node);
        elements1.clear();
        assert(elements2.empty());

        // add seeds
        elements1.push_back(elements_copy[seed1]);
        elements2.push_back(elements_copy[seed2]);

        // calculate boxes
        geometry::convert(rtree::element_indexable(elements_copy[seed1], tr), box1);
        geometry::convert(rtree::element_indexable(elements_copy[seed2], tr), box2);

        // remove seeds
        if (seed1 < seed2)
        {
            elements_copy.erase(elements_copy.begin() + seed2);
            elements_copy.erase(elements_copy.begin() + seed1);
        }
        else
        {
            elements_copy.erase(elements_copy.begin() + seed1);
            elements_copy.erase(elements_copy.begin() + seed2);
        }

        // initialize areas
        area_type area1 = index::area(box1);
        area_type area2 = index::area(box2);

        size_t remaining = elements_copy.size();

        // redistribute the rest of the elements
        while ( !elements_copy.empty() )
        {
            typename elements_type::reverse_iterator el_it = elements_copy.rbegin();
            bool insert_into_group1 = false;

            size_t elements1_count = elements1.size();
            size_t elements2_count = elements2.size();

            // if there is small number of elements left and the number of elements in node is lesser than min_elems
            // just insert them to this node
            if ( elements1_count + remaining <= min_elems )
            {
                insert_into_group1 = true;
            }
            else if ( elements2_count + remaining <= min_elems )
            {
                insert_into_group1 = false;
            }
            // insert the best element
            else
            {
                // find element with minimum groups areas increses differences
                area_type area_increase1 = 0;
                area_type area_increase2 = 0;
                el_it = pick_next(elements_copy.rbegin(), elements_copy.rend(),
                                  box1, box2, area1, area2, tr,
                                  area_increase1, area_increase2);

                if ( area_increase1 < area_increase2 ||
                     ( area_increase1 == area_increase2 && area1 < area2 ) ||
                     ( area1 == area2 && elements1_count <= elements2_count ) )
                {
                    insert_into_group1 = true;
                }
                else
                {
                    insert_into_group1 = false;
                }
            }

            // move element to the choosen group
            element_type const& elem = *el_it;
            indexable_type const& indexable = rtree::element_indexable(elem, tr);

            if ( insert_into_group1 )
            {
                elements1.push_back(elem);
                geometry::expand(box1, indexable);
                area1 = index::area(box1);
            }
            else
            {
                elements2.push_back(elem);
                geometry::expand(box2, indexable);
                area2 = index::area(box2);
            }

            assert(!elements_copy.empty());
            elements_copy.erase(--el_it.base());

            assert(0 < remaining);
            --remaining;
        }
    }

    // TODO: awulkiew - change following function to static member of the pick_next class?

    template <typename It>
    static inline It pick_next(It first, It last,
                               Box const& box1, Box const& box2,
                               area_type const& area1, area_type const& area2,
                               Translator const& tr,
                               area_type & out_area_increase1, area_type & out_area_increase2)
    {
        typedef typename boost::iterator_value<It>::type element_type;
        typedef typename rtree::element_indexable_type<element_type, Translator>::type indexable_type;

        area_type greatest_area_incrase_diff = 0;
        It out_it = first;
        out_area_increase1 = 0;
        out_area_increase2 = 0;
        
        // find element with greatest difference between increased group's boxes areas
        for ( It el_it = first ; el_it != last ; ++el_it )
        {
            indexable_type const& indexable = rtree::element_indexable(*el_it, tr);

            // calculate enlarged boxes and areas
            Box enlarged_box1(box1);
            Box enlarged_box2(box2);
            geometry::expand(enlarged_box1, indexable);
            geometry::expand(enlarged_box2, indexable);
            area_type enlarged_area1 = index::area(enlarged_box1);
            area_type enlarged_area2 = index::area(enlarged_box2);

            area_type area_incrase1 = (enlarged_area1 - area1);
            area_type area_incrase2 = (enlarged_area2 - area2);

            area_type area_incrase_diff = area_incrase1 < area_incrase2 ?
                area_incrase2 - area_incrase1 : area_incrase1 - area_incrase2;

            if ( greatest_area_incrase_diff < area_incrase_diff )
            {
                greatest_area_incrase_diff = area_incrase_diff;
                out_it = el_it;
                out_area_increase1 = area_incrase1;
                out_area_increase2 = area_incrase2;
            }
        }

        return out_it;
    }
};

} // namespace detail

}}} // namespace detail::rtree::visitors

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_QUADRATIC_REDISTRIBUTE_ELEMENTS_HPP
