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

#include <boost/geometry/extensions/index/algorithms/is_valid.hpp>

#include <boost/geometry/extensions/index/rtree/visitors/insert.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/remove.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/copy.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/destroy.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/query.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/nearest.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/children_box.hpp>

#include <boost/geometry/extensions/index/rtree/linear/linear.hpp>
#include <boost/geometry/extensions/index/rtree/quadratic/quadratic.hpp>
#include <boost/geometry/extensions/index/rtree/rstar/rstar.hpp>

namespace boost { namespace geometry { namespace index {

// TODO move extensional/debug visitors to the other folder
// move gldraw indexable (+ value with translator) to geometry::index namespace

// TODO: awulkiew - implement iterators ?

// TODO: allow copying of a tree with different template parameters? e.g. Parameters, Translator?

// TODO: should funcions like empty(tree) clear(tree) box(tree) be free functions?
// change name of empty predicate generator - empty()?

// TODO which types should be public and which one should be private?
// nodes, options etc. probably shouldn't be public

// TODO change remove() to erase() or just add erase() ?
// erase works on iterators of this container so this may be confusing with remove(ValIt, ValIt)

template <
    typename Value,
    typename Parameters,
    typename Translator = translator::def<Value>,
    typename Allocator = std::allocator<Value>
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

    typedef Allocator allocator_type;
    typedef typename detail::rtree::allocators<allocator_type, value_type, typename options_type::parameters_type, box_type, node_tag>::type allocators_type;
    typedef typename allocators_type::size_type size_type;

    typedef typename detail::rtree::node<value_type, typename options_type::parameters_type, box_type, allocators_type, node_tag>::type node;
    typedef typename detail::rtree::internal_node<value_type, typename options_type::parameters_type, box_type, allocators_type, node_tag>::type internal_node;
    typedef typename detail::rtree::leaf<value_type, typename options_type::parameters_type, box_type, allocators_type, node_tag>::type leaf;

    inline explicit rtree(translator_type const& translator = translator_type(), Allocator allocator = std::allocator<value_type>())
        : m_values_count(0)
        , m_root(0)
        , m_leafs_level(0)
        , m_translator(translator)
        , m_allocators(allocator)
    {
        create();
    }

    template<typename Iterator>
    inline explicit rtree(Iterator first, Iterator last, translator_type const& translator = translator_type(), Allocator allocator = std::allocator<value_type>())
        : m_values_count(0)
        , m_root(0)
        , m_leafs_level(0)
        , m_translator(translator)
    {
        create();
        insert(first, last);
    }

    inline ~rtree()
    {
        destroy(*this);
    }

    inline rtree(rtree const& src)
        : m_allocators(src.m_allocators)
    {
        try
        {
            copy(src, *this);
        }
        catch(...)
        {
            destroy(*this);
            throw;
        }
    }

    inline rtree & operator=(rtree const& src)
    {
        if ( &src == this )
            return *this;

        destroy(*this);
        
        m_allocators = src.m_allocators;

        try
        {
            copy(src, *this);
        }
        catch(...)
        {
            destroy(*this);
            throw;
        }

        return *this;
    }

    inline void insert(value_type const& value)
    {
        BOOST_GEOMETRY_INDEX_ASSERT(index::is_valid(m_translator(value)), "Indexable is invalid");

        try
        {
            detail::rtree::visitors::insert<
                value_type,
                value_type,
                options_type,
                translator_type,
                box_type,
                allocators_type,
                typename options_type::insert_tag
            > insert_v(m_root, m_leafs_level, value, m_translator, m_allocators);

            detail::rtree::apply_visitor(insert_v, *m_root);

            ++m_values_count;
        }
        catch(...)
        {
            destroy(*this);
            throw;
        }
    }

    template <typename Iterator>
    inline void insert(Iterator first, Iterator last)
    {
        for ( ; first != last ; ++first )
            insert(*first);
    }

    inline void remove(value_type const& value)
    {
        // TODO: awulkiew - assert for correct value (indexable) ?

        BOOST_GEOMETRY_INDEX_ASSERT(0 < m_values_count, "can't remove, there are no elements in the rtree");

        try
        {
            detail::rtree::visitors::remove<
                value_type,
                options_type,
                translator_type,
                box_type,
                allocators_type
            > remove_v(m_root, m_leafs_level, value, m_translator, m_allocators);

            detail::rtree::apply_visitor(remove_v, *m_root);

            --m_values_count;
        }
        catch(...)
        {
            destroy(*this);
            throw;
        }
    }

    template <typename Iterator>
    inline void remove(Iterator first, Iterator last)
    {
        for ( ; first != last ; ++first )
            remove(*first);
    }

    template <typename Predicates, typename OutIter>
    inline size_type query(Predicates const& pred, OutIter out_it) const
    {
        detail::rtree::visitors::query<value_type, options_type, translator_type, box_type, allocators_type, Predicates, OutIter>
            find_v(m_translator, pred, out_it);

        detail::rtree::apply_visitor(find_v, *m_root);

        return find_v.found_count;
    }

    template <typename DistancePredicate>
    inline size_type nearest(DistancePredicate const& dpred, value_type & v) const
    {
        return nearest_one(dpred, detail::empty(), v);
    }

    template <typename DistancePredicate, typename Predicates>
    inline size_type nearest(DistancePredicate const& dpred, Predicates const& pred, value_type & v) const
    {
        return nearest_one(dpred, pred, v);
    }

    template <typename DistancePredicate, typename OutIter>
    inline size_type nearest(DistancePredicate const& dpred, size_t k, OutIter out_it) const
    {
        return nearest_k(dpred, k, detail::empty(), out_it);
    }

    template <typename DistancePredicate, typename Predicates, typename OutIter>
    inline size_type nearest(DistancePredicate const& dpred, size_t k, Predicates const& pred, OutIter out_it) const
    {
        return nearest_k(dpred, k, pred, out_it);
    }

    inline size_type size() const
    {
        return m_values_count;
    }

    inline bool empty() const
    {
        return 0 == m_values_count;
    }

    inline void clear()
    {
        destroy(*this);
        create();
    }

    inline box_type box() const
    {
        if ( empty() )
        {
            box_type result;
            geometry::assign_inverse(result);
            return result;
        }

        detail::rtree::visitors::children_box<value_type, options_type, translator_type, box_type, allocators_type>
            children_box_v(m_translator);

        detail::rtree::apply_visitor(children_box_v, *m_root);

        return children_box_v.result;
    }

    template <typename Visitor>
    inline void apply_visitor(Visitor & visitor) const
    {
        detail::rtree::apply_visitor(visitor, *m_root);
    }

    inline translator_type const& translator() const
    {
        return  m_translator;
    }

    inline size_type values_count() const
    {
        return m_values_count;
    }

    inline size_type depth() const
    {
        return m_leafs_level;
    }

private:
    inline void create()
    {
        m_root = detail::rtree::create_node<allocators_type, leaf>::apply(m_allocators);
        m_values_count = 0;
        m_leafs_level = 0;
    }

    inline void destroy(rtree & t)
    {
        detail::rtree::visitors::destroy<value_type, options_type, translator_type, box_type, allocators_type> del_v(t.m_root, t.m_allocators);
        detail::rtree::apply_visitor(del_v, *t.m_root);

        t.m_root = 0;
        t.m_values_count = 0;
        t.m_leafs_level = 0;
    }

    inline void copy(rtree const& src, rtree & dst) const
    {
        //dst.m_allocators = src.m_allocators;

        detail::rtree::visitors::copy<value_type, options_type, translator_type, box_type, allocators_type> copy_v(dst.m_allocators);
        detail::rtree::apply_visitor(copy_v, *src.m_root);

        dst.m_root = copy_v.result;
        dst.m_values_count = src.m_values_count;
        dst.m_leafs_level = src.m_leafs_level;
        dst.m_translator = src.m_translator;
    }

    template <typename DistancesPredicates, typename Predicates>
    inline size_type nearest_one(DistancesPredicates const& dpred, Predicates const& pred, value_type & v) const
    {
        typedef typename detail::point_relation<DistancesPredicates>::type point_relation;
        typedef typename detail::relation<point_relation>::value_type point_type;

        typedef detail::rtree::visitors::nearest_one<
            value_type,
            translator_type,
            point_type
        > result_type;

        result_type result;

        detail::rtree::visitors::nearest<
            value_type,
            options_type,
            translator_type,
            box_type,
            allocators_type,
            DistancesPredicates,
            Predicates,
            result_type
        > nearest_v(m_translator, dpred, pred, result);

        detail::rtree::apply_visitor(nearest_v, *m_root);

        return result.get(v);
    }

    template <typename DistancesPredicates, typename Predicates, typename OutIter>
    inline size_type nearest_k(DistancesPredicates const& dpred, size_t k, Predicates const& pred, OutIter out_it) const
    {
        typedef typename detail::point_relation<DistancesPredicates>::type point_relation;
        typedef typename detail::relation<point_relation>::value_type point_type;

        typedef detail::rtree::visitors::nearest_k<
            value_type,
            translator_type,
            point_type
        > result_type;

        result_type result(k);

        detail::rtree::visitors::nearest<
            value_type,
            options_type,
            translator_type,
            box_type,
            allocators_type,
            DistancesPredicates,
            Predicates,
            result_type
        > nearest_v(m_translator, dpred, pred, result);

        detail::rtree::apply_visitor(nearest_v, *m_root);

        return result.get(out_it);
    }

    size_type m_values_count;
    node *m_root;
    size_type m_leafs_level;
    translator_type m_translator;
    allocators_type m_allocators;
};

template <typename Value, typename Options, typename Translator, typename Allocator>
inline void insert(rtree<Value, Options, Translator, Allocator> & tree, Value const& v)
{
    tree.insert(v);
}

template<typename Value, typename Options, typename Translator, typename Allocator, typename Iterator>
inline void insert(rtree<Value, Options, Translator, Allocator> & tree, Iterator first, Iterator last)
{
    tree.insert(first, last);
}

template <typename Value, typename Options, typename Translator, typename Allocator>
inline void remove(rtree<Value, Options, Translator, Allocator> & tree, Value const& v)
{
    tree.remove(v);
}

template<typename Value, typename Options, typename Translator, typename Allocator, typename Iterator>
inline void remove(rtree<Value, Options, Translator, Allocator> & tree, Iterator first, Iterator last)
{
    tree.remove(first, last);
}

template <typename Value, typename Options, typename Translator, typename Allocator, typename Predicates, typename OutIter>
inline size_t query(rtree<Value, Options, Translator, Allocator> const& tree, Predicates const& pred, OutIter out_it)
{
    return tree.query(pred, out_it);
}

template <typename Value, typename Options, typename Translator, typename Allocator, typename DistancesPredicates>
inline size_t nearest(rtree<Value, Options, Translator, Allocator> const& tree, DistancesPredicates const& dpred, Value & v)
{
    return tree.nearest(dpred, v);
}

template <typename Value, typename Options, typename Translator, typename Allocator, typename DistancesPredicates, typename Predicates>
inline size_t nearest(rtree<Value, Options, Translator, Allocator> const& tree, DistancesPredicates const& dpred, Predicates const& pred, Value & v)
{
    return tree.nearest(dpred, pred, v);
}

template <typename Value, typename Options, typename Translator, typename Allocator, typename DistancesPredicates, typename OutIter>
inline size_t nearest(rtree<Value, Options, Translator, Allocator> const& tree, DistancesPredicates const& dpred, size_t k, OutIter out_it)
{
    return tree.nearest(dpred, k, out_it);
}

template <typename Value, typename Options, typename Translator, typename Allocator, typename DistancesPredicates, typename Predicates, typename OutIter>
inline size_t nearest(rtree<Value, Options, Translator, Allocator> const& tree, DistancesPredicates const& dpred, size_t k, Predicates const& pred, OutIter out_it)
{
    return tree.nearest(dpred, k, pred, out_it);
}

template <typename Value, typename Options, typename Translator, typename Allocator>
inline void clear(rtree<Value, Options, Translator, Allocator> & tree)
{
    return tree.clear();
}

template <typename Value, typename Options, typename Translator, typename Allocator>
inline size_t size(rtree<Value, Options, Translator, Allocator> const& tree)
{
    return tree.size();
}

template <typename Value, typename Options, typename Translator, typename Allocator>
inline bool empty(rtree<Value, Options, Translator, Allocator> const& tree)
{
    return tree.empty();
}

template <typename Value, typename Options, typename Translator, typename Allocator>
inline typename rtree<Value, Options, Translator, Allocator>::box_type
box(rtree<Value, Options, Translator, Allocator> const& tree)
{
    return tree.box();
}

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_RTREE_HPP
