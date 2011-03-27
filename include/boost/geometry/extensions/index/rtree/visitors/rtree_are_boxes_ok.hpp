// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.Index - R-tree boxes checking visitor
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_RTREE_ARE_BOXES_OK_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_RTREE_ARE_BOXES_OK_HPP

#include <boost/geometry/algorithms//equals.hpp>
#include <boost/geometry/extensions/index/rtree/rtree_node.hpp>

namespace boost { namespace geometry { namespace index {

namespace visitors {

template <typename Value, typename Translator, typename Box, typename Tag>
class rtree_are_boxes_ok : public boost::static_visitor<bool>
{
    typedef typename index::detail::rtree_internal_node<Value, Box, Tag>::type internal_node;
    typedef typename index::detail::rtree_leaf<Value, Box, Tag>::type leaf;

public:
    inline rtree_are_boxes_ok(Translator const& tr)
        : m_tr(tr), m_is_root(true)
    {}

    inline bool operator()(internal_node const& n)
    {
        if (n.children.empty())
            return false;

        Box box_bckup = m_box;
        bool is_root_bckup = m_is_root;

        m_is_root = false;

        for ( internal_node::children_type::const_iterator it = n.children.begin();
              it != n.children.end() ; ++it)
        {
            m_box = it->first;

            if ( false == boost::apply_visitor(*this, *it->second) )
                return false;
        }

        m_box = box_bckup;
        m_is_root = is_root_bckup;

        Box result;
        geometry::convert(n.children.front().first, result);
        for(internal_node::children_type::const_iterator it = n.children.begin() + 1;
            it != n.children.end() ; ++it)
        {
            geometry::expand(result, it->first);
        }
        
        return m_is_root || geometry::equals(result, m_box);
    }

    inline bool operator()(leaf const& n)
    {
        if (n.values.empty())
            return false;

        Box result;
        geometry::convert(m_tr(n.values.front()), result);
        for(leaf::values_type::const_iterator it = n.values.begin() + 1;
            it != n.values.end() ; ++it)
        {
            geometry::expand(result, m_tr(*it));
        }

        return m_is_root || geometry::equals(result, m_box);
    }

private:
    Translator const& m_tr;
    Box m_box;
    bool m_is_root;
};

} // namespace visitors

template <typename Value, typename Translator, typename Box, typename Tag>
bool rtree_are_boxes_ok(rtree<Value, Translator, Box, Tag> const& tree)
{
    typedef rtree<Value, Translator, Box, Tag> rt;
    visitors::rtree_are_boxes_ok<
        typename rt::value_type,
        typename rt::translator_type,
        typename rt::box_type,
        Tag> v(tree.get_translator());
    
    return tree.apply_visitor(v);
}

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_RTREE_ARE_BOXES_OK_HPP
