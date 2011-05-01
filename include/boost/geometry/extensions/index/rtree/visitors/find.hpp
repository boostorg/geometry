// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.Index - R-tree find
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_FIND_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_FIND_HPP

#include <boost/geometry/algorithms/intersects.hpp>

#include <boost/geometry/extensions/index/rtree/node.hpp>

#include <boost/geometry/extensions/index/rtree/visitors/is_leaf.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree { namespace visitors {

// rtree spatial query visitor

template <typename Value, typename Translator, typename Box, typename Tag, typename Geometry, typename OutIter>
struct find : public boost::static_visitor<>
{
    typedef typename rtree::node<Value, Box, Tag>::type node;
    typedef typename rtree::internal_node<Value, Box, Tag>::type internal_node;
    typedef typename rtree::leaf<Value, Box, Tag>::type leaf;

    inline find(Translator const& t, Geometry const& g, OutIter out_it)
        : tr(t), geom(g), out_iter(out_it)
    {}

    inline void operator()(internal_node const& n)
    {
        /*typedef typename internal_node::children_type children_type;

        std::deque<node*> nodes;
        
        for (typename children_type::const_iterator it = n.children.begin();
            it != n.children.end(); ++it)
        {
            if ( geometry::intersects(it->first, geom) )
            {
                nodes.push_back(it->second);
            }
        }

        while ( !nodes.empty() )
        {
            node *n = nodes.back();
            nodes.pop_back();

            if ( !boost::apply_visitor(visitors::is_leaf<Value, Box, Tag>(), *n) )
            {
                internal_node &in = boost::get<internal_node>(*n);

                for (typename children_type::const_iterator it = in.children.begin();
                    it != in.children.end(); ++it)
                {
                    if ( geometry::intersects(it->first, geom) )
                    {
                        nodes.push_back(it->second);
                    }
                }
            }
            else
            {
                operator()(boost::get<leaf>(*n));
            }
        }
        */

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
            {
                out_iter = *it;
                ++out_iter;
            }
        }
    }

    Translator const& tr;
    Geometry const& geom;
    OutIter out_iter;
};

}}} // namespace detail::rtree::visitors

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_VISITORS_FIND_HPP
