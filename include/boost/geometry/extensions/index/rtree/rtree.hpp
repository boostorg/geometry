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

#include <boost/geometry/extensions/index/rtree/visitors/find.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/destroy.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/insert.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/remove.hpp>

#include <boost/geometry/extensions/index/rtree/linear/linear.hpp>

#include <boost/geometry/extensions/index/rtree/quadratic/quadratic.hpp>

// TODO: awulkiew - correct implementation
//#include <boost/geometry/extensions/index/rtree/rstar/rstar.hpp>

namespace boost { namespace geometry { namespace index {

// TODO: awulkiew - min and max elems as template parameters?

template <
    typename Value,
    typename Translator = default_parameter,
    typename Tag = linear_tag
>
class rtree
{
public:
    typedef Value value_type;
    typedef typename detail::default_translator_type<value_type, Translator>::type translator_type;
    typedef typename detail::geometry_box_type<typename translator_type::indexable_type>::type box_type;
    typedef Tag tag_type;

    typedef typename detail::rtree::node<value_type, box_type, tag_type>::type node;
    typedef typename detail::rtree::internal_node<value_type, box_type, tag_type>::type internal_node;
    typedef typename detail::rtree::leaf<value_type, box_type, tag_type>::type leaf;

    inline explicit rtree(
        size_t max_elems_per_node = 4,
        size_t min_elems_per_node = 2,
        translator_type const& translator = translator_type()
    )
        : m_values_count(0)
        , m_max_elems_per_node(max_elems_per_node)
        , m_min_elems_per_node(min_elems_per_node)
        , m_root(0)
        , m_translator(translator)
    {
        if ( m_min_elems_per_node < 1 )
            m_min_elems_per_node = 1;
        if ( m_max_elems_per_node < 2 )
            m_max_elems_per_node = 2;

        m_root = detail::rtree::create_node(leaf());
    }

    ~rtree()
    {
        detail::rtree::visitors::destroy<value_type, translator_type, box_type, tag_type> del_v;
        boost::apply_visitor(del_v, *m_root);
    }

    // TODO: awulkiew - change name to query?

    template <typename Geometry, typename OutIter>
    inline void find(Geometry const& geom, OutIter out_it) const
    {
        detail::rtree::visitors::find<value_type, translator_type, box_type, tag_type, Geometry, OutIter>
            find_v(m_translator, geom, out_it);

        boost::apply_visitor(find_v, *m_root);
    }

    void insert(value_type const& value)
    {
        // TODO: awulkiew - assert for correct value

        detail::rtree::visitors::insert<value_type, value_type, translator_type, box_type, tag_type>
            insert_v(m_root, value, m_min_elems_per_node, m_max_elems_per_node, m_translator);

        boost::apply_visitor(insert_v, *m_root);

        ++m_values_count;
    }

    void remove(value_type const& value)
    {
        // TODO: awulkiew - assert for correct value
        assert(0 < m_values_count);

        detail::rtree::visitors::remove<value_type, translator_type, box_type, tag_type>
            remove_v(m_root, value, m_min_elems_per_node, m_max_elems_per_node, m_translator);

        boost::apply_visitor(remove_v, *m_root);

        --m_values_count;
    }

    size_t size() const
    {
        return m_values_count;
    }

    bool empty() const
    {
        // TODO: awulkiew - take root into consideration
        return 0 == m_values_count;
    }

    void clear()
    {
        // TODO: awulkiew - implement
        assert(false);
    }

    template <typename Visitor>
    typename Visitor::result_type apply_visitor(Visitor & visitor) const
    {
        return boost::apply_visitor(visitor, *m_root);
    }

    translator_type const& get_translator() const
    {
        return  m_translator;
    }

    size_t values_count() const
    {
        return m_values_count;
    }

private:
    size_t m_values_count;
    size_t m_max_elems_per_node;
    size_t m_min_elems_per_node;
    node *m_root;
    translator_type m_translator;
};

template <typename Value, typename Translator, typename Tag>
void insert(rtree<Value, Translator, Tag> & tree, Value const& v)
{
    tree.insert(v);
}

template <typename Value, typename Translator, typename Tag>
void remove(rtree<Value, Translator, Tag> & tree, Value const& v)
{
    tree.remove(v);
}

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RSTREE_RSTREE_HPP
