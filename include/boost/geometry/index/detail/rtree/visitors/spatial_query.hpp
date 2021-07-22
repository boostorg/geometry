// Boost.Geometry Index
//
// R-tree spatial query visitor implementation
//
// Copyright (c) 2011-2014 Adam Wulkiewicz, Lodz, Poland.
//
// This file was modified by Oracle on 2019-2021.
// Modifications copyright (c) 2019-2021 Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_INDEX_DETAIL_RTREE_VISITORS_SPATIAL_QUERY_HPP
#define BOOST_GEOMETRY_INDEX_DETAIL_RTREE_VISITORS_SPATIAL_QUERY_HPP

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree { namespace visitors {

template <typename MembersHolder, typename Predicates, typename OutIter>
struct spatial_query
    : public MembersHolder::visitor_const
{
    typedef typename MembersHolder::parameters_type parameters_type;
    typedef typename MembersHolder::translator_type translator_type;
    typedef typename MembersHolder::allocators_type allocators_type;

    typedef typename index::detail::strategy_type<parameters_type>::type strategy_type;

    typedef typename MembersHolder::node node;
    typedef typename MembersHolder::internal_node internal_node;
    typedef typename MembersHolder::leaf leaf;

    typedef typename allocators_type::node_pointer node_pointer;
    typedef typename allocators_type::size_type size_type;

    inline spatial_query(parameters_type const& par, translator_type const& t, Predicates const& p, OutIter out_it)
        : tr(t), pred(p), out_iter(out_it), found_count(0), strategy(index::detail::get_strategy(par))
    {}

    size_type apply(node_pointer root)
    {
        rtree::apply_visitor(*this, *root);

        for (;;)
        {
            if (m_internal_stack.empty())
            {
                break;
            }

            node_pointer ptr = m_internal_stack.back();
            m_internal_stack.pop_back();
            rtree::apply_visitor(*this, *ptr);
        }

        return found_count;
    }

    inline void operator()(internal_node const& n)
    {
        namespace id = index::detail;

        // traverse nodes meeting predicates
        for (auto const& p : rtree::elements(n))
        {
            // if node meets predicates
            // 0 - dummy value
            if (id::predicates_check<id::bounds_tag>(pred, 0, p.first, strategy))
            {
                m_internal_stack.push_back(p.second);
            }
        }
    }

    inline void operator()(leaf const& n)
    {
        namespace id = index::detail;

        // get all values meeting predicates
        for (auto const& v : rtree::elements(n))
        {
            // if value meets predicates
            if (id::predicates_check<id::value_tag>(pred, v, tr(v), strategy))
            {
                *out_iter = v;
                ++out_iter;

                ++found_count;
            }
        }
    }

private:
    translator_type const& tr;

    Predicates pred;

    std::vector<node_pointer> m_internal_stack;

    OutIter out_iter;
    size_type found_count;

    strategy_type strategy;
};

template <typename MembersHolder, typename Predicates>
class spatial_query_incremental
    : public MembersHolder::visitor_const
{
    typedef typename MembersHolder::value_type value_type;
    typedef typename MembersHolder::parameters_type parameters_type;
    typedef typename MembersHolder::translator_type translator_type;
    typedef typename MembersHolder::allocators_type allocators_type;

    typedef typename index::detail::strategy_type<parameters_type>::type strategy_type;

public:
    typedef typename MembersHolder::node node;
    typedef typename MembersHolder::internal_node internal_node;
    typedef typename MembersHolder::leaf leaf;

    typedef typename allocators_type::size_type size_type;
    typedef typename allocators_type::const_reference const_reference;
    typedef typename allocators_type::node_pointer node_pointer;

    typedef typename rtree::elements_type<internal_node>::type::const_iterator internal_iterator;
    typedef typename rtree::elements_type<leaf>::type leaf_elements;
    typedef typename rtree::elements_type<leaf>::type::const_iterator leaf_iterator;

    inline spatial_query_incremental()
        : m_translator(NULL)
//        , m_pred()
        , m_values(NULL)
        , m_current()
//        , m_strategy()
    {}

    inline spatial_query_incremental(parameters_type const& params, translator_type const& t, Predicates const& p)
        : m_translator(::boost::addressof(t))
        , m_pred(p)
        , m_values(NULL)
        , m_current()
        , m_strategy(index::detail::get_strategy(params))
    {}

    inline void operator()(internal_node const& n)
    {
        typedef typename rtree::elements_type<internal_node>::type elements_type;
        elements_type const& elements = rtree::elements(n);

        m_internal_stack.push_back(std::make_pair(elements.begin(), elements.end()));
    }

    inline void operator()(leaf const& n)
    {
        m_values = ::boost::addressof(rtree::elements(n));
        m_current = rtree::elements(n).begin();
    }

    const_reference dereference() const
    {
        BOOST_GEOMETRY_INDEX_ASSERT(m_values, "not dereferencable");
        return *m_current;
    }

    void initialize(node_pointer root)
    {
        rtree::apply_visitor(*this, *root);
        search_value();
    }

    void increment()
    {
        ++m_current;
        search_value();
    }

    void search_value()
    {
        namespace id = index::detail;
        for (;;)
        {
            // if leaf is choosen, move to the next value in leaf
            if ( m_values )
            {
                if ( m_current != m_values->end() )
                {
                    // return if next value is found
                    value_type const& v = *m_current;
                    if (id::predicates_check<id::value_tag>(m_pred, v, (*m_translator)(v), m_strategy))
                    {
                        return;
                    }

                    ++m_current;
                }
                // no more values, clear current leaf
                else
                {
                    m_values = 0;
                }
            }
            // if leaf isn't choosen, move to the next leaf
            else
            {
                // return if there is no more nodes to traverse
                if ( m_internal_stack.empty() )
                    return;

                // no more children in current node, remove it from stack
                if ( m_internal_stack.back().first == m_internal_stack.back().second )
                {
                    m_internal_stack.pop_back();
                    continue;
                }

                internal_iterator it = m_internal_stack.back().first;
                ++m_internal_stack.back().first;

                // next node is found, push it to the stack
                if (id::predicates_check<id::bounds_tag>(m_pred, 0, it->first, m_strategy))
                {
                    rtree::apply_visitor(*this, *(it->second));
                }
            }
        }
    }

    bool is_end() const
    {
        return 0 == m_values;
    }

    friend bool operator==(spatial_query_incremental const& l, spatial_query_incremental const& r)
    {
        return (l.m_values == r.m_values) && (0 == l.m_values || l.m_current == r.m_current );
    }

private:

    const translator_type * m_translator;

    Predicates m_pred;

    std::vector< std::pair<internal_iterator, internal_iterator> > m_internal_stack;
    const leaf_elements * m_values;
    leaf_iterator m_current;

    strategy_type m_strategy;
};

}}} // namespace detail::rtree::visitors

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_INDEX_DETAIL_RTREE_VISITORS_SPATIAL_QUERY_HPP
