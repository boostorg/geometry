// Boost.Geometry Index
//
// R-tree levels validating visitor implementation
//
// Copyright (c) 2011-2012 Adam Wulkiewicz, Lodz, Poland.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_ARE_LEVELS_OK_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_ARE_LEVELS_OK_HPP

#include <boost/geometry/extensions/index/rtree/node/node.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree { namespace visitors {

template <typename Value, typename NodeProxy>
class are_levels_ok
    : public rtree::visitor<
          Value,
          typename NodeProxy::parameters_type,
          typename NodeProxy::box_type,
          typename NodeProxy::allocators_type,
          typename NodeProxy::node_tag,
          true
      >::type
    , index::nonassignable
{
    typedef typename NodeProxy::internal_node internal_node;
    typedef typename NodeProxy::leaf leaf;
    typedef typename NodeProxy::box_type box_type;

public:
    inline are_levels_ok()
        : result(true), m_leafs_level((std::numeric_limits<size_t>::max)()), m_current_level(0)
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

        size_t current_level_backup = m_current_level;
        ++m_current_level;

        for ( typename elements_type::const_iterator it = elements.begin();
              it != elements.end() ; ++it)
        {
            rtree::apply_visitor(*this, *it->second);

            if ( result == false )
                return;
        }

        m_current_level = current_level_backup;
    }

    inline void operator()(leaf const& n)
    {
        typedef typename rtree::elements_type<leaf>::type elements_type;
        elements_type const& elements = rtree::elements(n);

        // empty leaf in non-root node
        if (0 < m_current_level && elements.empty())
        {
            result = false;
            return;
        }

        if ( m_leafs_level == (std::numeric_limits<size_t>::max)() )
        {
            m_leafs_level = m_current_level;
        }
        else if ( m_leafs_level != m_current_level )
        {
            result = false;
        }
    }

    bool result;

private:
    size_t m_leafs_level;
    size_t m_current_level;
};

}}} // namespace detail::rtree::visitors

template <typename Value, typename Parameters, typename Translator, typename Allocator>
bool are_levels_ok(rtree<Value, Parameters, Translator, Allocator> const& tree)
{
    detail::rtree::visitors::are_levels_ok<
        Value,
        detail::rtree::node_proxy<
            Value,
            Parameters,
            Translator,
            Allocator
        >
    > v;
    
    tree.apply_visitor(v);

    return v.result;
}

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_ARE_LEVELS_OK_HPP
