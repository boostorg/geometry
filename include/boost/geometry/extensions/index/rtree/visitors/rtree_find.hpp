// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.Index - R-tree find
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_RTREE_FIND_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_RTREE_FIND_HPP

#include <boost/geometry/algorithms/intersects.hpp>

#include <boost/geometry/extensions/index/rtree/rtree_node.hpp>

namespace boost { namespace geometry { namespace index {

namespace visitors {

// rtree spatial query visitor

template <typename Value, typename Translator, typename Box, typename Tag, typename Geometry>
struct rtree_find : public boost::static_visitor<>
{
    typedef typename index::detail::rtree_internal_node<Value, Box, Tag>::type internal_node;
    typedef typename index::detail::rtree_leaf<Value, Box, Tag>::type leaf;

    inline rtree_find(Geometry const& g, Translator const& t)
        : geom(g), tr(t)
    {}

    inline void operator()(internal_node const& n)
    {
        typedef typename internal_node::children_type children_type;

        for (typename children_type::const_iterator it = n.children.begin();
            it != n.children.end(); ++it)
        {
            if ( geometry::intersects(it->first, geom) )
                boost::apply_visitor(*this, *it->second);
        }
    }

    inline void operator()(leaf const& n)
    {
        typedef typename leaf::values_type values_type;

        for (typename values_type::const_iterator it = n.values.begin();
            it != n.values.end(); ++it)
        {
            if ( geometry::intersects(tr(*it), geom) )
                result.push_back(*it);
        }
    }

    Geometry const& geom;
    Translator const& tr;
    std::vector<Value> result;
};

} // namespace visitors

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_RTREE_FIND_HPP
