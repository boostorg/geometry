// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.Index - R-tree implementation
//
// Copyright 2008 Federico J. Fernandez.
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_RTREE_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_RTREE_HPP

#include <boost/geometry/extensions/index/default_parameter.hpp>

#include <boost/geometry/extensions/index/rtree/filters.hpp>

#include <boost/geometry/extensions/index/rtree/visitors/rtree_find.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/rtree_delete.hpp>

#include <boost/geometry/extensions/index/rtree/rstar/rstar.hpp>

namespace boost { namespace geometry { namespace index {

// TODO: awulkiew - min and max elems as template parameters?

template <
    typename Value,
    typename Translator = default_parameter,
    typename Box = default_parameter,
    typename Tag = rtree_rstar_tag
>
class rtree
{
public:
    typedef Value value_type;
    typedef typename detail::default_translator_type<Value, Translator>::type translator_type;
    typedef typename detail::default_box_type<typename translator_type::indexable_type, Box>::type box_type;

    typedef typename detail::rtree_node<Value, box_type, rtree_rstar_tag>::type node;
    typedef typename detail::rtree_internal_node<Value, box_type, rtree_rstar_tag>::type internal_node;
    typedef typename detail::rtree_leaf<Value, box_type, rtree_rstar_tag>::type leaf;

    inline explicit rtree(size_t max_elems_per_node = 4, size_t min_elems_per_node = 2, translator_type const& translator = translator_type())
        : m_values_count(0)
        , m_max_elems_per_node(max_elems_per_node)
        , m_min_elems_per_node(min_elems_per_node)
        , m_root(0)
        , m_translator(translator)
    {
        if ( min_elems_per_node < 2 )
            min_elems_per_node = 2;

        // TODO: awulkiew - reconsider following assert
        assert(2 * m_min_elems_per_node <= m_max_elems_per_node);

        m_root = detail::rtree_create_node(leaf());
    }

    ~rtree()
    {
        visitors::rtree_delete<Value, translator_type, box_type, Tag> del_v;
        boost::apply_visitor(del_v, *m_root);
    }

    template <typename Geometry>
    inline std::vector<Value> find(Geometry const& geom) const
    {
        visitors::rtree_find<Value, translator_type, box_type, Tag, Geometry> find_v(geom, m_translator);
        boost::apply_visitor(find_v, *m_root);
        return find_v.result;
    }

    void insert(Value const& value)
    {
        visitors::rtree_insert<Value, translator_type, box_type, Tag>
            insert_v(m_root, value, m_min_elems_per_node, m_max_elems_per_node, m_translator);

        boost::apply_visitor(insert_v, *m_root);

        ++m_values_count;
    }

    template <typename Visitor>
    void apply_visitor(Visitor & visitor) const
    {
        boost::apply_visitor(visitor, *m_root);
    }

    translator_type const& get_translator() const
    {
        return  m_translator;
    }

private:
    size_t m_values_count;
    size_t m_max_elems_per_node;
    size_t m_min_elems_per_node;
    node *m_root;
    translator_type m_translator;
};

template <typename Value, typename Translator, typename Box, typename Tag>
void insert(rtree<Value, Translator, Box, Tag> & tree, Value const& v)
{
    tree.insert(v);
}

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RSTREE_RSTREE_HPP
