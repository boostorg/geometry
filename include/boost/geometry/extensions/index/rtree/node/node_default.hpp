// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.Index - R-tree polymorphic nodes
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_NODE_NODE_DEFAULT_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_NODE_NODE_DEFAULT_HPP

#include <vector>

#include <boost/geometry/algorithms/expand.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree {

// visitor forward declaration
template <typename Value, typename Parameters, typename Box, typename Tag, bool IsVisitableConst>
struct visitor_poly;

// nodes types

template <typename Value, typename Parameters, typename Box, typename Tag>
struct node_poly
{
    virtual ~node_poly() {}
    virtual void apply_visitor(visitor_poly<Value, Parameters, Box, Tag, false> &) = 0;
    virtual void apply_visitor(visitor_poly<Value, Parameters, Box, Tag, true> &) const = 0;
};

template <typename Value, typename Parameters, typename Box, typename Tag>
struct internal_node_poly : public node_poly<Value, Parameters, Box, Tag>
{
    typedef std::vector<
        std::pair<Box, node_poly<Value, Parameters, Box, Tag> *>
    > elements_type;

    void apply_visitor(visitor_poly<Value, Parameters, Box, Tag, false> & v) { v(*this); }
    void apply_visitor(visitor_poly<Value, Parameters, Box, Tag, true> & v) const { v(*this); }

    elements_type elements;
};

template <typename Value, typename Parameters, typename Box, typename Tag>
struct leaf_poly : public node_poly<Value, Parameters, Box, Tag>
{
    typedef std::vector<Value> elements_type;

    void apply_visitor(visitor_poly<Value, Parameters, Box, Tag, false> & v) { v(*this); }
    void apply_visitor(visitor_poly<Value, Parameters, Box, Tag, true> & v) const { v(*this); }

    elements_type elements;
};

// nodes traits

template <typename Value, typename Parameters, typename Box, typename Tag>
struct node
{
    typedef node_poly<Value, Parameters, Box, Tag> type;
};

template <typename Value, typename Parameters, typename Box, typename Tag>
struct internal_node
{
    typedef internal_node_poly<Value, Parameters, Box, Tag> type;
};

template <typename Value, typename Parameters, typename Box, typename Tag>
struct leaf
{
    typedef leaf_poly<Value, Parameters, Box, Tag> type;
};

// nodes conversion

template <typename Derived, typename Parameters, typename Value, typename Box, typename Tag>
inline Derived & get(node_poly<Value, Parameters, Box, Tag> & n)
{
    assert(dynamic_cast<Derived*>(&n));
    return static_cast<Derived&>(n);
}

template <typename Derived, typename Parameters, typename Value, typename Box, typename Tag>
inline Derived * get(node_poly<Value, Parameters, Box, Tag> * n)
{
    assert(dynamic_cast<Derived*>(n));
    return static_cast<Derived*>(n);
}

// visitor

template <typename Value, typename Parameters, typename Box, typename Tag>
struct visitor_poly<Value, Parameters, Box, Tag, true>
{
    typedef typename internal_node<Value, Parameters, Box, Tag>::type internal_node;
    typedef typename leaf<Value, Parameters, Box, Tag>::type leaf;

    virtual ~visitor_poly() {}

    virtual void operator()(internal_node const&) = 0;
    virtual void operator()(leaf const&) = 0;
};

template <typename Value, typename Parameters, typename Box, typename Tag>
struct visitor_poly<Value, Parameters, Box, Tag, false>
{
    typedef typename internal_node<Value, Parameters, Box, Tag>::type internal_node;
    typedef typename leaf<Value, Parameters, Box, Tag>::type leaf;

    virtual ~visitor_poly() {}

    virtual void operator()(internal_node &) = 0;
    virtual void operator()(leaf &) = 0;
};

// visitor traits

template <typename Value, typename Parameters, typename Box, typename Tag, bool IsVisitableConst>
struct visitor
{
    typedef visitor_poly<Value, Parameters, Box, Tag, IsVisitableConst> type;
};

template <typename Visitor, typename Visitable>
inline void apply_visitor(Visitor &v, Visitable & n)
{
    n.apply_visitor(v);
}

// element's indexable type

template <typename Value, typename Translator>
struct element_indexable_type
{
	typedef typename Translator::indexable_type type;
};

template <typename Value, typename Parameters, typename Box, typename Tag, typename Translator>
struct element_indexable_type<
    std::pair<Box, node_poly<Value, Parameters, Box, Tag> *>,
    Translator
>
{
    typedef Box type;
};

// element's indexable getter

template <typename Value, typename Translator>
inline typename Translator::indexable_type const&
	element_indexable(Value const& el, Translator const& tr)
{
	return tr(el);
}

template <typename Value, typename Parameters, typename Box, typename Tag, typename Translator>
inline Box const&
element_indexable(
    std::pair< Box, node_poly<Value, Parameters, Box, Tag> *> const& el,
    Translator const&)
{
    return el.first;
}

// create leaf node

template <typename Value, typename Parameters, typename Box, typename Tag>
inline typename node<Value, Parameters, Box, Tag>::type *
create_node(leaf_poly<Value, Parameters, Box, Tag> const& l)
{
	typedef typename node<Value, Parameters, Box, Tag>::type node;
	node * n = new leaf_poly<Value, Parameters, Box, Tag>(l);
	return n;
}

// create internal node

template <typename Value, typename Parameters, typename Box, typename Tag>
inline typename node<Value, Parameters, Box, Tag>::type *
create_node(internal_node_poly<Value, Parameters, Box, Tag> const& in)
{
	typedef typename node<Value, Parameters, Box, Tag>::type node;
	node * n = new internal_node_poly<Value, Parameters, Box, Tag>(in);
	return n;
}

// default node

template <typename Value, typename Parameters, typename Box, typename Tag>
inline void delete_node(node_poly<Value, Parameters, Box, Tag> * n)
{
	delete n;
}

// nodes elements

template <typename Node>
struct elements_type
{
	typedef typename Node::elements_type type;
};

template <typename Node>
inline typename elements_type<Node>::type &
	elements(Node & n)
{
	return n.elements;
}

template <typename Node>
inline typename elements_type<Node>::type const&
	elements(Node const& n)
{
	return n.elements;
}

// elements box

template <typename Box, typename FwdIter, typename Translator>
inline Box elements_box(FwdIter first, FwdIter last, Translator const& tr)
{
	Box result;

	if (first == last)
	{    
		geometry::assign_zero(result);
		return result;
	}

	geometry::convert(element_indexable(*first, tr), result);
	++first;

	for ( ; first != last ; ++first )
		geometry::expand(result, element_indexable(*first, tr));

	return result;
}

}} // namespace detail::rtree

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_NODE_NODE_DEFAULT_HPP
