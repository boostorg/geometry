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
#include <boost/geometry/extensions/index/rtree/filters.hpp>

#include <boost/geometry/extensions/index/rtree/node/node.hpp>

#include <boost/geometry/extensions/index/rtree/visitors/query.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/nearest.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/destroy.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/insert.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/remove.hpp>

#include <boost/geometry/extensions/index/rtree/linear/linear.hpp>
#include <boost/geometry/extensions/index/rtree/quadratic/quadratic.hpp>
#include <boost/geometry/extensions/index/rtree/rstar/rstar.hpp>

namespace boost { namespace geometry { namespace index {

// TODO copying
// TODO move extensional/debug visitors to the other folder

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

        // TODO: awulkiew - move this into the destroy visitor?
        detail::rtree::delete_node(m_root);
    }

    template <typename Predicates, typename OutIter>
    inline size_t query(Predicates const& pred, OutIter out_it) const
    {
        detail::rtree::visitors::query<value_type, options_type, translator_type, box_type, Predicates, OutIter>
            find_v(m_translator, pred, out_it);

        detail::rtree::apply_visitor(find_v, *m_root);

        return find_v.found_count;
    }

    // TODO: awulkiew - change Point to Geometry?
    // return number of elements instead of bool?

    template <typename DistancePredicate>
    inline size_t nearest(DistancePredicate const& p, value_type & v) const
    {
        return nearest_one(p, detail::empty(), v);
    }

    template <typename DistancePredicate, typename Predicates>
    inline size_t nearest(DistancePredicate const& p, Predicates const& pred, value_type & v) const
    {
        return nearest_one(p, pred, v);
    }

    template <typename DistancePredicate, typename OutIter>
    inline size_t nearest(DistancePredicate const& p, size_t k, OutIter out_it) const
    {
        return nearest_k(p, k, detail::empty(), out_it);
    }

    template <typename DistancePredicate, typename Predicates, typename OutIter>
    inline size_t nearest(DistancePredicate const& p, size_t k, Predicates const& pred, OutIter out_it) const
    {
        return nearest_k(p, k, pred, out_it);
    }

    inline void insert(value_type const& value)
    {
        // TODO: awulkiew - assert for correct value

        detail::rtree::visitors::insert<value_type, value_type, options_type, translator_type, box_type, typename options_type::insert_tag>
            insert_v(m_root, m_leafs_level, value, m_translator);

        detail::rtree::apply_visitor(insert_v, *m_root);

        ++m_values_count;
    }

    inline void remove(value_type const& value)
    {
        // TODO: awulkiew - assert for correct value

        BOOST_GEOMETRY_INDEX_ASSERT(0 < m_values_count, "can't remove, there is no elements in the rtree");

        detail::rtree::visitors::remove<value_type, options_type, translator_type, box_type>
            remove_v(m_root, m_leafs_level, value, m_translator);

        detail::rtree::apply_visitor(remove_v, *m_root);

        --m_values_count;
    }

    inline size_t size() const
    {
        return m_values_count;
    }

    inline bool empty() const
    {
        return 0 == m_values_count;
    }

    // TODO: awulkiew
    // clear()
    // aabb/box/get_box/etc.
    // iterators, begin/end/etc.

    //inline void clear()
    //{
    //    // TODO: awulkiew - implement
    //    BOOST_GEOMETRY_INDEX_ASSERT(false, "not implemented");
    //}

    template <typename Visitor>
    inline void apply_visitor(Visitor & visitor) const
    {
        detail::rtree::apply_visitor(visitor, *m_root);
    }

    inline translator_type const& get_translator() const
    {
        return  m_translator;
    }

    inline size_t values_count() const
    {
        return m_values_count;
    }

    inline size_t depth() const
    {
        return m_leafs_level;
    }

private:
    template <typename DistancePredicate, typename Predicates>
    inline size_t nearest_one(DistancePredicate const& p, Predicates const& pred, value_type & v) const
    {
        typedef detail::rtree::visitors::nearest_one<
            value_type,
            translator_type,
            typename detail::distance_point<DistancePredicate>::type
        > result_type;

        result_type result;

        detail::rtree::visitors::nearest<
            value_type,
            options_type,
            translator_type,
            box_type,
            DistancePredicate,
            Predicates,
            result_type
        > nearest_v(m_translator, p, pred, result);

        detail::rtree::apply_visitor(nearest_v, *m_root);

        return result.get(v);
    }

    template <typename DistancePredicate, typename Predicates, typename OutIter>
    inline size_t nearest_k(DistancePredicate const& p, size_t k, Predicates const& pred, OutIter out_it) const
    {
        typedef detail::rtree::visitors::nearest_k<
            value_type,
            translator_type,
            typename detail::distance_point<DistancePredicate>::type
        > result_type;

        result_type result(k);

        detail::rtree::visitors::nearest<
            value_type,
            options_type,
            translator_type,
            box_type,
            DistancePredicate,
            Predicates,
            result_type
        > nearest_v(m_translator, p, pred, result);

        detail::rtree::apply_visitor(nearest_v, *m_root);

        return result.get(out_it);
    }

    size_t m_values_count;
    node *m_root;
    size_t m_leafs_level;
    translator_type m_translator;
};

template <typename Value, typename Options, typename Translator>
inline void insert(rtree<Value, Options, Translator> & tree, Value const& v)
{
    tree.insert(v);
}

template <typename Value, typename Options, typename Translator>
inline void remove(rtree<Value, Options, Translator> & tree, Value const& v)
{
    tree.remove(v);
}

template <typename Value, typename Options, typename Translator, typename Predicates, typename OutIter>
inline size_t query(rtree<Value, Options, Translator> const& tree, Predicates const& pred, OutIter out_it)
{
    return tree.query(pred, out_it);
}

template <typename Value, typename Options, typename Translator, typename Point>
inline size_t nearest(rtree<Value, Options, Translator> const& tree, Point const& pt, Value & v)
{
    return tree.nearest(pt, v);
}

template <typename Value, typename Options, typename Translator, typename Point, typename Predicates>
inline size_t nearest(rtree<Value, Options, Translator> const& tree, Point const& pt, Predicates const& pred, Value & v)
{
    return tree.nearest(pt, pred, v);
}

template <typename Value, typename Options, typename Translator, typename Point, typename OutIter>
inline size_t nearest(rtree<Value, Options, Translator> const& tree, Point const& pt, size_t k, OutIter out_it)
{
    return tree.nearest(pt, k, out_it);
}

template <typename Value, typename Options, typename Translator, typename Point, typename Predicates, typename OutIter>
inline size_t nearest(rtree<Value, Options, Translator> const& tree, Point const& pt, size_t k, Predicates const& pred, OutIter out_it)
{
    return tree.nearest(pt, k, pred, out_it);
}

template <typename Value, typename Options, typename Translator>
inline size_t size(rtree<Value, Options, Translator> const& tree)
{
    return tree.size();
}

template <typename Value, typename Options, typename Translator>
inline bool empty(rtree<Value, Options, Translator> const& tree)
{
    return tree.empty();
}

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_RTREE_HPP
