// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.Index - R-tree loading visitor
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_LOAD_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_LOAD_HPP

#include <iostream>

#include <boost/geometry/extensions/index/rtree/node.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree { namespace visitors {

template <typename Value, typename Translator, typename Box, typename Tag>
struct load;

template <typename Translator, typename Box, typename Tag>
struct load<
    std::pair<
        boost::geometry::model::box<
            boost::geometry::model::point<float, 2, boost::geometry::cs::cartesian>
        >,
        size_t
    >,
    typename Translator,
    Box,
    Tag
>
: public boost::static_visitor<>
{
    typedef boost::geometry::model::point<float, 2, boost::geometry::cs::cartesian> point_type;

    typedef std::pair<
        boost::geometry::model::box<
            point_type
        >,
        size_t
    > value_type;

    typedef typename rtree::node<value_type, Box, Tag>::type node;
    typedef typename rtree::internal_node<value_type, Box, Tag>::type internal_node;
    typedef typename rtree::leaf<value_type, Box, Tag>::type leaf;

    inline load(std::istream & i, Translator const& t)
        : is(i), tr(t)
    {}

    inline void operator()(internal_node & n)
    {
        std::string node_type;
        float min_x, min_y, max_x, max_y;
        size_t c;

        is >> node_type;
        is >> min_x;
        is >> min_y;
        is >> max_x;
        is >> max_y;
        is >> c;

        Box b(point_type(min_x, min_y), point_type(max_x, max_y));
        node * new_n = 0;

        if ( node_type == "i" )
            new_n = rtree::create_node(internal_node());
        else if ( node_type == "l" )
            new_n = rtree::create_node(leaf());
        else
            assert(0);

        n.children.push_back(std::make_pair(b, new_n));

        for ( size_t i = 0 ; i < c ; ++i )
            boost::apply_visitor(*this, *new_n);
    }

    inline void operator()(leaf & n)
    {
        std::string node_type;
        float min_x, min_y, max_x, max_y;
        size_t id;

        is >> node_type;
        is >> min_x;
        is >> min_y;
        is >> max_x;
        is >> max_y;
        is >> id;

        assert(id == "v");

        Box b(point_type(min_x, min_y), point_type(max_x, max_y));
        n.values.push_back(std::make_pair(b, id));
    }

    std::istream & is;
    Translator const& tr;
};

}}} // namespace detail::rtree::visitors

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_LOAD_HPP
