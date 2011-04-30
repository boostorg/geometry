// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.Index - R-tree saving visitor
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_SAVE_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_SAVE_HPP

#include <iostream>

#include <boost/geometry/extensions/index/rtree/node.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree { namespace visitors {

template <typename Value, typename Translator, typename Box, typename Tag>
struct save;

template <typename Translator, typename Box, typename Tag>
struct save<
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

    inline save(std::ostream & o, Translator const& t)
        : os(o), tr(t)
    {}

    inline void operator()(internal_node & n)
    {
        os << n.children.size() << '\n';

        for ( size_t i = 0 ; i < n.children.size() ; ++i )
        {
            if ( boost::apply_visitor(visitors::is_leaf<value_type, Box, Tag>(), *(n.children[i].second)) )
                os << "l ";
            else
                os << "i ";
            os << geometry::get<min_corner, 0>(n.children[i].first) << ' ';
            os << geometry::get<min_corner, 1>(n.children[i].first) << ' ';
            os << geometry::get<max_corner, 0>(n.children[i].first) << ' ';
            os << geometry::get<max_corner, 1>(n.children[i].first) << ' ';

            boost::apply_visitor(*this, *(n.children[i].second));
        }
    }

    inline void operator()(leaf & n)
    {
        os << n.values.size() << '\n';

        for ( size_t i = 0 ; i < n.values.size() ; ++i )
        {
            os << "v ";
            os << geometry::get<min_corner, 0>(n.values[i].first) << ' ';
            os << geometry::get<min_corner, 1>(n.values[i].first) << ' ';
            os << geometry::get<max_corner, 0>(n.values[i].first) << ' ';
            os << geometry::get<max_corner, 1>(n.values[i].first) << ' ';
            os << n.values[i].second << '\n';
        }
    }

    std::ostream & os;
    Translator const& tr;
};

}}} // namespace detail::rtree::visitors

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_SAVE_HPP
