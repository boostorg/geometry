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
    Translator,
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
        typedef typename rtree::elements_type<internal_node>::type elements_type;
        elements_type & elements = rtree::elements_get(n);

        os << elements.size() << '\n';

        for ( size_t i = 0 ; i < elements.size() ; ++i )
        {
            if ( boost::apply_visitor(visitors::is_leaf<value_type, Box, Tag>(), *(elements[i].second)) )
                os << "l ";
            else
                os << "i ";
            os << geometry::get<min_corner, 0>(elements[i].first) << ' ';
            os << geometry::get<min_corner, 1>(elements[i].first) << ' ';
            os << geometry::get<max_corner, 0>(elements[i].first) << ' ';
            os << geometry::get<max_corner, 1>(elements[i].first) << ' ';

            boost::apply_visitor(*this, *(elements[i].second));
        }
    }

    inline void operator()(leaf & n)
    {
        typedef typename rtree::elements_type<leaf>::type elements_type;
        elements_type & elements = rtree::elements_get(n);

        os << elements.size() << '\n';

        for ( size_t i = 0 ; i < elements.size() ; ++i )
        {
            os << "v ";
            os << geometry::get<min_corner, 0>(elements[i].first) << ' ';
            os << geometry::get<min_corner, 1>(elements[i].first) << ' ';
            os << geometry::get<max_corner, 0>(elements[i].first) << ' ';
            os << geometry::get<max_corner, 1>(elements[i].first) << ' ';
            os << elements[i].second << '\n';
        }
    }

    std::ostream & os;
    Translator const& tr;
};

}}} // namespace detail::rtree::visitors

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_SAVE_HPP
