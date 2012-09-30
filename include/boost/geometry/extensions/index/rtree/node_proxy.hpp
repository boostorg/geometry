// Boost.Geometry Index
//
// R-tree node proxy
//
// Copyright (c) 2011-2012 Adam Wulkiewicz, Lodz, Poland.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_NODE_PROXY_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_NODE_PROXY_HPP

namespace boost { namespace geometry { namespace index { namespace detail { namespace rtree {

template <
    typename Value,
    typename Parameters,
    typename Translator = translator::def<Value>,
    typename Allocator = std::allocator<Value>
>
class node_proxy
{
public:
    typedef Value value_type;
    typedef Translator translator_type;
    typedef typename translator::indexable_type<Translator>::type indexable_type;
    typedef typename index::default_box_type<indexable_type>::type box_type;

    typedef typename detail::rtree::options_type<Parameters>::type options_type;
    typedef typename options_type::parameters_type parameters_type;
    typedef typename options_type::node_tag node_tag;

    typedef Allocator allocator_type;

    typedef detail::rtree::allocators<
        allocator_type, value_type, parameters_type, box_type, node_tag
    > allocators_type;

    typedef typename allocators_type::size_type size_type;

    typedef typename detail::rtree::node<
        value_type, parameters_type, box_type, allocators_type, node_tag
    >::type node;

    typedef typename detail::rtree::internal_node<
        value_type, parameters_type, box_type, allocators_type, node_tag
    >::type internal_node;

    typedef typename detail::rtree::leaf<
        value_type, parameters_type, box_type, allocators_type, node_tag
    >::type leaf;

    node_proxy(parameters_type const& parameters, translator_type const& translator, Allocator allocator)
        : m_parameters(parameters)
        , m_translator(translator)
        , m_allocators(allocator)
    {}

    // HMMMM - trzeba zwracac uwage na translator::return_type
//    template <typename Element>
//    typename element_indexable_type<Element>::type const&
//    rtree::element_indexable(m_element, m_translator));

    typename translator_type::result_type
    indexable(value_type const& v) const
    {
        return m_translator(v);
    }

    typename element_indexable_type<
        typename internal_node::elements_type::value_type,
        translator_type
    >::type const&
    indexable(typename internal_node::elements_type::value_type const& el) const
    {
        return element_indexable(el, m_translator);
    }

    bool equals(value_type const& v1, value_type const& v2) const
    {
        return m_translator.equals(v1, v2);
    }

    template <typename FwdIter>
    box_type elements_box(FwdIter first, FwdIter last) const
    {
        BOOST_GEOMETRY_INDEX_ASSERT(first != last, "Can't calculate element's box");

        box_type result;

        geometry::convert(element_indexable(*first, m_translator), result);
        ++first;

        for ( ; first != last ; ++first )
            geometry::expand(result, element_indexable(*first, m_translator));

        return result;
    }

    parameters_type const& parameters() const
    {
        return m_parameters;
    }

    translator_type const& translator() const
    {
        return m_translator;
    }

    allocators_type & allocators()
    {
        return m_allocators;
    }

    allocator_type & allocator()
    {
        return m_allocators.allocator;
    }

private:    
    parameters_type m_parameters;
    translator_type m_translator;
    allocators_type m_allocators;
};

template <typename Node, typename Value, typename Parameters, typename Translator, typename Allocator>
typename node_proxy<Value, Parameters, Translator, Allocator>::node *
    create(node_proxy<Value, Parameters, Translator, Allocator> & np)
{
    return detail::rtree::create_node<
        typename node_proxy<Value, Parameters, Translator, Allocator>::allocators_type,
        Node
    >::apply(np.allocators());
}

template <typename Node, typename Value, typename Parameters, typename Translator, typename Allocator>
void destroy(
    typename node_proxy<Value, Parameters, Translator, Allocator>::node * n,
    node_proxy<Value, Parameters, Translator, Allocator> & np)
{
    return detail::rtree::destroy_node<
        typename node_proxy<Value, Parameters, Translator, Allocator>::allocators_type,
        Node
    >::apply(np.allocators(), n);
}

}}}}} // namespace boost::geometry::index::detail::rtree

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_NODE_PROXY_HPP
