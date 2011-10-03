// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.Index - R-tree boxes checking visitor
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_ARE_BOXES_OK_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_ARE_BOXES_OK_HPP

#include <boost/geometry/algorithms/equals.hpp>
#include <boost/geometry/extensions/index/rtree/node/node.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree { namespace visitors {

template <typename Value, typename Options, typename Translator, typename Box, typename Allocators>
class are_boxes_ok
    : public rtree::visitor<Value, typename Options::parameters_type, Box, Allocators, typename Options::node_tag, true>::type
    , index::nonassignable
{
    typedef typename rtree::internal_node<Value, typename Options::parameters_type, Box, Allocators, typename Options::node_tag>::type internal_node;
    typedef typename rtree::leaf<Value, typename Options::parameters_type, Box, Allocators, typename Options::node_tag>::type leaf;

public:
    inline are_boxes_ok(Translator const& tr)
        : result(false), m_tr(tr), m_is_root(true)
    {}

    inline void operator()(internal_node const& n)
    {
        typedef typename rtree::elements_type<internal_node>::type elements_type;
        elements_type const& elements = rtree::elements(n);

        if (elements.empty())
        {
            result = false;
            return;
        }

        Box box_bckup = m_box;
        bool is_root_bckup = m_is_root;

        m_is_root = false;

        for ( typename elements_type::const_iterator it = elements.begin();
              it != elements.end() ; ++it)
        {
            m_box = it->first;

            rtree::apply_visitor(*this, *it->second);

            if ( result == false )
                return;
        }

        m_box = box_bckup;
        m_is_root = is_root_bckup;

        Box box_exp;
        geometry::convert(elements.front().first, box_exp);
        for( typename elements_type::const_iterator it = elements.begin() + 1;
            it != elements.end() ; ++it)
        {
            geometry::expand(box_exp, it->first);
        }
        
        result = m_is_root || geometry::equals(box_exp, m_box);
    }

    inline void operator()(leaf const& n)
    {
        typedef typename rtree::elements_type<leaf>::type elements_type;
        elements_type const& elements = rtree::elements(n);

        // non-root node
        if (!m_is_root)
        {
            if ( elements.empty() )
            {
                result = false;
                return;
            }
        
            Box box_exp;
            geometry::convert(m_tr(elements.front()), box_exp);
            for(typename elements_type::const_iterator it = elements.begin() + 1;
                it != elements.end() ; ++it)
            {
                geometry::expand(box_exp, m_tr(*it));
            }

            result = geometry::equals(box_exp, m_box);
        }
        else
            result = true;
    }

    bool result;

private:
    Translator const& m_tr;
    Box m_box;
    bool m_is_root;
};

}}} // namespace detail::rtree::visitors

template <typename Value, typename Options, typename Translator, typename Allocator>
bool are_boxes_ok(rtree<Value, Options, Translator, Allocator> const& tree)
{
    typedef rtree<Value, Options, Translator, Allocator> rt;
    detail::rtree::visitors::are_boxes_ok<
        typename rt::value_type,
        typename rt::options_type,
        typename rt::translator_type,
        typename rt::box_type,
        typename rt::allocators_type
    > v(tree.translator());
    
    tree.apply_visitor(v);

    return v.result;
}

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_ARE_BOXES_OK_HPP
