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

#include <boost/tuple/tuple.hpp>

#include <boost/geometry/extensions/index/algorithms/area.hpp>
#include <boost/geometry/extensions/index/algorithms/union_area.hpp>

#include <boost/geometry/extensions/index/rtree/node.hpp>
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

        assert(2 <= elements_count);

        seed1 = 0;
        seed2 = 1;
        area_type greatest_free_area = 0;
        for ( size_t i = 0 ; i < elements_count ; ++i )
        {
            for ( size_t j = i + 1 ; j < elements_count ; ++j )
            {
                indexable_type & ind1 = rtree::element_indexable(elements[i], tr);
                indexable_type & ind2 = rtree::element_indexable(elements[j], tr);

                box_type enlarged_box;
                geometry::convert(ind1);
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

// TODO: awulkiew - redistribute

} // namespace detail

}}} // namespace detail::rtree::visitors

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_QUADRATIC_REDISTRIBUTE_ELEMENTS_HPP
