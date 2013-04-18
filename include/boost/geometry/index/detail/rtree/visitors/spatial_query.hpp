// Boost.Geometry Index
//
// R-tree spatial query visitor implementation
//
// Copyright (c) 2011-2013 Adam Wulkiewicz, Lodz, Poland.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_INDEX_DETAIL_RTREE_VISITORS_SPATIAL_QUERY_HPP
#define BOOST_GEOMETRY_INDEX_DETAIL_RTREE_VISITORS_SPATIAL_QUERY_HPP

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree { namespace visitors {

template <typename Value, typename Options, typename Translator, typename Box, typename Allocators, typename Predicates, typename OutIter>
struct spatial_query
    : public rtree::visitor<Value, typename Options::parameters_type, Box, Allocators, typename Options::node_tag, true>::type
{
    typedef typename rtree::node<Value, typename Options::parameters_type, Box, Allocators, typename Options::node_tag>::type node;
    typedef typename rtree::internal_node<Value, typename Options::parameters_type, Box, Allocators, typename Options::node_tag>::type internal_node;
    typedef typename rtree::leaf<Value, typename Options::parameters_type, Box, Allocators, typename Options::node_tag>::type leaf;

    typedef typename Allocators::size_type size_type;

    static const unsigned predicates_len = index::detail::predicates_length<Predicates>::value;

    inline spatial_query(Translator const& t, Predicates const& p, OutIter out_it)
        : tr(t), pred(p), out_iter(out_it), found_count(0)
    {}

    inline void operator()(internal_node const& n)
    {
        typedef typename rtree::elements_type<internal_node>::type elements_type;
        elements_type const& elements = rtree::elements(n);

        // traverse nodes meeting predicates
        for (typename elements_type::const_iterator it = elements.begin();
            it != elements.end(); ++it)
        {
            // if node meets predicates
            // 0 - dummy value
            if ( index::detail::predicates_check<index::detail::bounds_tag, 0, predicates_len>(pred, 0, it->first) )
                rtree::apply_visitor(*this, *it->second);
        }
    }

    inline void operator()(leaf const& n)
    {
        typedef typename rtree::elements_type<leaf>::type elements_type;
        elements_type const& elements = rtree::elements(n);

        // get all values meeting predicates
        for (typename elements_type::const_iterator it = elements.begin();
            it != elements.end(); ++it)
        {
            // if value meets predicates
            if ( index::detail::predicates_check<index::detail::value_tag, 0, predicates_len>(pred, *it, tr(*it)) )
            {
                out_iter = *it;
                ++out_iter;

                ++found_count;
            }
        }
    }

    Translator const& tr;

    Predicates pred;

    OutIter out_iter;
    size_type found_count;
};

template <typename Value, typename Options, typename Translator, typename Box, typename Allocators, typename Predicates>
struct spatial_query_incremental
    : public rtree::visitor<Value, typename Options::parameters_type, Box, Allocators, typename Options::node_tag, true>::type
{
    typedef typename rtree::node<Value, typename Options::parameters_type, Box, Allocators, typename Options::node_tag>::type node;
    typedef typename rtree::internal_node<Value, typename Options::parameters_type, Box, Allocators, typename Options::node_tag>::type internal_node;
    typedef typename rtree::leaf<Value, typename Options::parameters_type, Box, Allocators, typename Options::node_tag>::type leaf;

    typedef typename Allocators::size_type size_type;
    typedef typename Allocators::node_pointer node_pointer;

    typedef typename rtree::elements_type<internal_node>::type::const_iterator internal_iterator;
    typedef typename rtree::elements_type<leaf>::type leaf_elements;

    static const unsigned predicates_len = index::detail::predicates_length<Predicates>::value;

    inline spatial_query_incremental(Translator const& t, Predicates const& p)
        : tr(t), pred(p)
        , values(0), value_index(0)
    {}

    inline void operator()(internal_node const& n)
    {
        typedef typename rtree::elements_type<internal_node>::type elements_type;
        elements_type const& elements = rtree::elements(n);

        internal_stack.push_back(std::make_pair(elements.begin(), elements.end()));
    }

    inline void operator()(leaf const& n)
    {
        typedef typename rtree::elements_type<leaf>::type elements_type;
        elements_type const& elements = rtree::elements(n);

        values = &elements;
        value_index = 0;
    }

    Value const& dereference() const
    {
        BOOST_ASSERT_MSG(values, "not dereferencable");
        return (*values)[value_index];
    }

    void increment()
    {
        if ( values )
            ++value_index;

        for (;;)
        {
            // if leaf is choosen, move to the next value in leaf
            if ( values )
            {
                for ( ;; ++value_index )
                {
                    // no more values, clear current leaf
                    if ( values->size() <= value_index )
                    {
                        values = 0;
                        value_index = 0;
                        break;
                    }

                    // return if next value is found
                    Value const& v = (*values)[value_index];
                    if ( index::detail::predicates_check<index::detail::value_tag, 0, predicates_len>(pred, v, tr(v)) )
                        return;
                }
            }

            // move to the next leaf if values aren't set
            while ( !values )
            {
                // return if there is no more nodes to traverse
                if ( internal_stack.empty() )
                    return;

                // no more children in current node, remove it from stack
                if ( internal_stack.back().first == internal_stack.back().second )
                {
                    internal_stack.pop_back();
                    continue;
                }

                internal_iterator it = internal_stack.back().first;
                ++internal_stack.back().first;

                // next node is found, push it to the stack
                if ( index::detail::predicates_check<index::detail::bounds_tag, 0, predicates_len>(pred, 0, it->first) )
                    rtree::apply_visitor(*this, *(it->second));
            }
        }
    }

    friend bool operator==(spatial_query_incremental const& l, spatial_query_incremental const& r)
    {
        return (l.values == r.values) && (l.value_index == r.value_index);
    }

    Translator const& tr;
    Predicates pred;

    boost::container::vector< std::pair<internal_iterator, internal_iterator> > internal_stack;
    const leaf_elements * values;
    size_type value_index;
};

} // namespace visitors

template <typename Value, typename Options, typename Translator, typename Box, typename Allocators, typename Predicates>
class spatial_query_iterator
{
    typedef visitors::spatial_query_incremental<Value, Options, Translator, Box, Allocators, Predicates> visitor_type;
    typedef typename visitor_type::node_pointer node_pointer;

    typedef typename Allocators::allocator_type::template rebind<Value>::other allocator_type;

public:
    typedef std::input_iterator_tag iterator_category;
    typedef const Value value_type;
    typedef Value const& reference;
    typedef typename allocator_type::difference_type difference_type;
    typedef typename allocator_type::const_pointer pointer;

    inline spatial_query_iterator(Translator const& t, Predicates const& p)
        : m_visitor(t, p)
    {}

    inline spatial_query_iterator(node_pointer root, Translator const& t, Predicates const& p)
        : m_visitor(t, p)
    {
        detail::rtree::apply_visitor(m_visitor, *root);
        m_visitor.increment();
    }

    reference operator*() const
    {
        return m_visitor.dereference();
    }

    const value_type * operator->() const
    {
        return boost::addressof(m_visitor.dereference());
    }

    spatial_query_iterator & operator++()
    {
        m_visitor.increment();
        return *this;
    }

    spatial_query_iterator operator++(int)
    {
        spatial_query_iterator temp = *this;
        this->operator++();
        return temp;
    }

    friend bool operator==(spatial_query_iterator const& l, spatial_query_iterator const& r)
    {
        return l.m_visitor == r.m_visitor;
    }

    friend bool operator!=(spatial_query_iterator const& l, spatial_query_iterator const& r)
    {
        return !(l.m_visitor == r.m_visitor);
    }
    
private:
    visitor_type m_visitor;
};

}} // namespace detail::rtree

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_INDEX_DETAIL_RTREE_VISITORS_SPATIAL_QUERY_HPP
