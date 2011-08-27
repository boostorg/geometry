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

#include <algorithm>
#include <boost/tuple/tuple.hpp>

#include <boost/geometry/geometry.hpp>

#include <boost/geometry/extensions/index/assert.hpp>
#include <boost/geometry/extensions/index/nonassignable.hpp>

#include <boost/geometry/extensions/index/translator/translator.hpp>

#include <boost/geometry/extensions/index/rtree/options.hpp>
#include <boost/geometry/extensions/index/rtree/predicates.hpp>
//#include <boost/geometry/extensions/index/rtree/filters.hpp>

#include <boost/geometry/extensions/index/rtree/node/node.hpp>

#include <boost/geometry/extensions/index/rtree/visitors/find.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/query.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/destroy.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/insert.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/remove.hpp>

#include <boost/geometry/extensions/index/rtree/linear/linear.hpp>
#include <boost/geometry/extensions/index/rtree/quadratic/quadratic.hpp>
#include <boost/geometry/extensions/index/rtree/rstar/rstar.hpp>

namespace boost { namespace geometry { namespace index {

// TODO copying

template <
    typename Value,
    typename Parameters,
    typename Translator = translator::def<Value>
>
class rtree
    : public boost::noncopyable
{
public:
    typedef Value value_type;
    typedef Translator translator_type;
    typedef typename translator_type::indexable_type indexable_type;
    typedef typename index::default_box_type<indexable_type>::type box_type;
    
    typedef typename detail::rtree::options_type<Parameters>::type options_type;
    typedef typename options_type::node_tag node_tag;

    typedef typename detail::rtree::node<value_type, typename options_type::parameters_type, box_type, node_tag>::type node;
    typedef typename detail::rtree::internal_node<value_type, typename options_type::parameters_type, box_type, node_tag>::type internal_node;
    typedef typename detail::rtree::leaf<value_type, typename options_type::parameters_type, box_type, node_tag>::type leaf;

    inline explicit rtree(translator_type const& translator = translator_type())
        : m_values_count(0)
        , m_root(0)
        , m_leafs_level(0)
        , m_translator(translator)
    {
        m_root = detail::rtree::create_node(leaf());
    }

    ~rtree()
    {
        detail::rtree::visitors::destroy<value_type, options_type, translator_type, box_type> del_v;
        detail::rtree::apply_visitor(del_v, *m_root);
    }

    template <typename Predicates, typename OutIter>
    inline void query(Predicates const& pred, OutIter out_it) const
    {
        detail::rtree::visitors::query<value_type, options_type, translator_type, box_type, Predicates, OutIter>
            find_v(m_translator, pred, out_it);

        detail::rtree::apply_visitor(find_v, *m_root);
    }

    // TODO: delete find method
    template <typename Geometry, typename OutIter>
    inline void find(Geometry const& geom, OutIter out_it) const
    {
        detail::rtree::visitors::find<value_type, options_type, translator_type, box_type, Geometry, OutIter>
            find_v(m_translator, geom, out_it);

        detail::rtree::apply_visitor(find_v, *m_root);
    }

    void insert(value_type const& value)
    {
        // TODO: awulkiew - assert for correct value

        detail::rtree::visitors::insert<value_type, value_type, options_type, translator_type, box_type, typename options_type::insert_tag>
            insert_v(m_root, m_leafs_level, value, m_translator);

        detail::rtree::apply_visitor(insert_v, *m_root);

        ++m_values_count;
    }

    void remove(value_type const& value)
    {
        // TODO: awulkiew - assert for correct value

        BOOST_GEOMETRY_INDEX_ASSERT(0 < m_values_count, "can't remove, there is no elements in the rtree");

        detail::rtree::visitors::remove<value_type, options_type, translator_type, box_type>
            remove_v(m_root, m_leafs_level, value, m_translator);

        detail::rtree::apply_visitor(remove_v, *m_root);

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
        BOOST_GEOMETRY_INDEX_ASSERT(false, "not implemented");
    }

    template <typename Visitor>
    void apply_visitor(Visitor & visitor) const
    {
        detail::rtree::apply_visitor(visitor, *m_root);
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
    node *m_root;
    size_t m_leafs_level;
    translator_type m_translator;
};

template <typename Value, typename Options, typename Translator>
void insert(rtree<Value, Options, Translator> & tree, Value const& v)
{
    tree.insert(v);
}

template <typename Value, typename Options, typename Translator>
void remove(rtree<Value, Options, Translator> & tree, Value const& v)
{
    tree.remove(v);
}

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_RTREE_HPP
