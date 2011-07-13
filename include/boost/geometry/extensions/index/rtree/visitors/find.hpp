// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.Index - R-tree find
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_FIND_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_FIND_HPP

#include <boost/geometry/extensions/index/rtree/node/node.hpp>

//#include <boost/geometry/extensions/index/rtree/visitors/is_leaf.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree { namespace visitors {

//template <size_t N, typename T>
//class array
//{
//public:
//    inline array()
//        : arr_elements(0)
//    {}
//
//    inline void push(T const& v)
//    {
//        arr[arr_elements++] = v;
//    }
//
//    inline T pop()
//    {
//        assert(0 < arr_elements);
//        return arr[--arr_elements];
//    }
//
//    inline bool empty() const
//    {
//        return 0 == arr_elements;
//    }
//
//    inline size_t size() const
//    {
//        return arr_elements;
//    }
//
//private:
//    boost::array<T, N> arr;
//    size_t arr_elements;
//};
//
//template <typename Cont>
//class dynamic
//{
//public:
//    typedef typename Cont::value_type value_type;
//
//    inline void push(value_type const& v)
//    {
//        cont.push_back(v);
//    }
//
//    inline value_type pop()
//    {
//        value_type v = cont.back();
//        cont.pop_back();
//        return v;
//    }
//
//    inline bool empty() const
//    {
//        return cont.empty();
//    }
//
//    inline size_t size() const
//    {
//        return cont.size();
//    }
//
//    inline void clear()
//    {
//        cont.clear();
//    }
//
//private:
//    Cont cont;
//};
//
//template <size_t N, typename Cont>
//class array_semi_dynamic
//{
//public:
//    typedef typename Cont::value_type value_type;
//
//    inline array_semi_dynamic()
//        : arr_elements(0)
//    {}
//
//    inline void push(value_type const& v)
//    {
//        if ( arr_elements < N )
//            arr[arr_elements++] = v;
//        else
//            cont.push_back(v);
//    }
//
//    inline value_type pop()
//    {
//        if ( !cont.empty() )
//        {
//            value_type v = cont.back();
//            cont.pop_back();
//            return v;
//        }
//        else
//        {
//            assert(0 < arr_elements);
//            return arr[--arr_elements];
//        }
//    }
//
//    inline bool empty() const
//    {
//        return cont.empty() && 0 == arr_elements;
//    }
//
//    inline size_t size() const
//    {
//        return arr_elements + cont.size();
//    }
//
//private:
//    boost::array<value_type, N> arr;
//    size_t arr_elements;
//    Cont cont;
//};

// rtree spatial query visitor

template <typename Value, typename Options, typename Translator, typename Box, typename Geometry, typename OutIter>
struct find : public rtree::visitor<Value, typename Options::parameters_type, Box, typename Options::node_tag, true>::type
{
    typedef typename rtree::node<Value, typename Options::parameters_type, Box, typename Options::node_tag>::type node;
    typedef typename rtree::internal_node<Value, typename Options::parameters_type, Box, typename Options::node_tag>::type internal_node;
    typedef typename rtree::leaf<Value, typename Options::parameters_type, Box, typename Options::node_tag>::type leaf;

    inline find(Translator const& t, Geometry const& g, OutIter out_it)
        : tr(t), geom(g), out_iter(out_it)
    {}

    inline void operator()(internal_node const& n)
    {
        //typedef typename internal_node::children_type children_type;

        //array_semi_dynamic<1024, std::deque<node*> > nodes;
        ////array<1024, node*> nodes;
        ////dynamic< std::deque<node*> > nodes;
        //
        //for (typename children_type::const_iterator it = n.children.begin();
        //    it != n.children.end(); ++it)
        //{
        //    if ( geometry::intersects(it->first, geom) )
        //    {
        //        nodes.push(it->second);
        //    }
        //}

        //while ( !nodes.empty() )
        //{
        //    node *n = nodes.pop();

        //    if ( !boost::apply_visitor(visitors::is_leaf<Value, Box, Tag>(), *n) )
        //    {
        //        internal_node &in = boost::get<internal_node>(*n);

        //        for (typename children_type::const_iterator it = in.children.begin();
        //            it != in.children.end(); ++it)
        //        {
        //            if ( geometry::intersects(it->first, geom) )
        //            {
        //                nodes.push(it->second);
        //            }
        //        }
        //    }
        //    else
        //    {
        //        operator()(boost::get<leaf>(*n));
        //    }
        //}

        typedef typename rtree::elements_type<internal_node>::type elements_type;
        elements_type const& elements = rtree::elements(n);

        for (typename elements_type::const_iterator it = elements.begin();
            it != elements.end(); ++it)
        {
            if ( geometry::intersects(it->first, geom) )
                rtree::apply_visitor(*this, *it->second);
        }
    }

    inline void operator()(leaf const& n)
    {
        typedef typename rtree::elements_type<leaf>::type elements_type;
        elements_type const& elements = rtree::elements(n);

        for (typename elements_type::const_iterator it = elements.begin();
            it != elements.end(); ++it)
        {
            if ( geometry::intersects(tr(*it), geom) )
            {
                out_iter = *it;
                ++out_iter;
            }
        }
    }

    Translator const& tr;
    Geometry const& geom;
    OutIter out_iter;
};

}}} // namespace detail::rtree::visitors

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_VISITORS_FIND_HPP
