// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.Index - R-tree printing visitor
//
// Copyright 2008 Federico J. Fernandez.
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_PRINT_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_PRINT_HPP

#include <iostream>

#include <boost/geometry/extensions/index/rtree/rtree_node.hpp>

namespace boost { namespace geometry { namespace index {

namespace visitors {

namespace dispatch {

template <typename Point, size_t Dimension>
struct rtree_print_point
{
    BOOST_STATIC_ASSERT(0 < Dimension);

    static inline void apply(std::ostream & os, Point const& p)
    {
        rtree_print_point<Point, Dimension - 1>::apply(os, p);

        os << ", " << geometry::get<Dimension - 1>(p);
    }
};

template <typename Point>
struct rtree_print_point<Point, 1>
{
    static inline void apply(std::ostream & os, Point const& p)
    {
        os << geometry::get<0>(p);
    }
};

template <typename Box, size_t Corner, size_t Dimension>
struct rtree_print_corner
{
    BOOST_STATIC_ASSERT(0 < Dimension);

    static inline void apply(std::ostream & os, Box const& b)
    {
        rtree_print_corner<Box, Corner, Dimension - 1>::apply(os, b);

        os << ", " << geometry::get<Corner, Dimension - 1>(b);
    }
};

template <typename Box, size_t Corner>
struct rtree_print_corner<Box, Corner, 1>
{
    static inline void apply(std::ostream & os, Box const& b)
    {
        os << geometry::get<Corner, 0>(b);
    }
};

template <typename Indexable, typename Tag>
struct rtree_print_indexable
{
};

template <typename Indexable>
struct rtree_print_indexable<Indexable, box_tag>
{
    typedef typename geometry::traits::point_type<Indexable>::type point_type;
    static const size_t dimension = geometry::traits::dimension<point_type>::value;

    static inline void apply(std::ostream &os, Indexable const& i)
    {
        os << '(';
        rtree_print_corner<Indexable, min_corner, dimension>::apply(os, i);
        os << ")x(";
        rtree_print_corner<Indexable, max_corner, dimension>::apply(os, i);
        os << ')';
    }
};

template <typename Indexable>
struct rtree_print_indexable<Indexable, point_tag>
{
    static const size_t dimension = geometry::traits::dimension<Indexable>::value;

    static inline void apply(std::ostream &os, Indexable const& i)
    {
        os << '(';
        rtree_print_point<Indexable, dimension>::apply(os, i);
        os << ')';
    }
};

} // namespace dispatch

namespace detail {

template <typename Indexable>
inline void rtree_print_indexable(std::ostream & os, Indexable const& i)
{
    dispatch::rtree_print_indexable<
        Indexable,
        typename geometry::traits::tag<Indexable>::type
    >::apply(os, i);
}

} // namespace detail

template <typename Value, typename Translator, typename Box, typename Tag>
struct rtree_print : public boost::static_visitor<>
{
    typedef typename index::detail::rtree_internal_node<Value, Box, Tag>::type internal_node;
    typedef typename index::detail::rtree_leaf<Value, Box, Tag>::type leaf;

    inline rtree_print(std::ostream & o, Translator const& t)
        : os(o), tr(t), level(0)
    {}

    inline void operator()(internal_node const& n)
    {
        typedef typename internal_node::children_type children_type;

        os << " --> Node --------" << std::endl;
        os << "  Address: " << this << std::endl;
        os << "  Level: " << level << std::endl;
        os << "  Children: " << n.children.size() << std::endl;
        os << "  | ";

        for (typename children_type::const_iterator it = n.children.begin();
            it != n.children.end(); ++it)
        {
            detail::rtree_print_indexable(os, it->first);
            os << " | ";
        }
        os << std::endl;
        os << " --< Node --------" << std::endl;

        os << " Children: " << std::endl;

        size_t level_backup = level;
        ++level;

        for (typename children_type::const_iterator it = n.children.begin();
            it != n.children.end(); ++it)
        {
            boost::apply_visitor(*this, *it->second);
        }

        level = level_backup;
    }

    inline void operator()(leaf const& n)
    {
        typedef typename leaf::values_type values_type;

        os << "\t" << " --> Leaf --------" << std::endl;
        os << "\t" << "  Values: " << n.values.size() << std::endl;
        for (typename values_type::const_iterator it = n.values.begin();
            it != n.values.end(); ++it)
        {
            os << "\t" << "  | ";
            detail::rtree_print_indexable(os, tr(*it));
            os << " | " << std::endl;;
        }
        os << "\t" << " --< Leaf --------" << std::endl;

    }

    std::ostream & os;
    Translator const& tr;

    size_t level;
};

} // namespace visitors

template <typename Value, typename Translator, typename Box, typename Tag>
std::ostream & print(rtree<Value, Translator, Box, Tag> const& tree, std::ostream & os = std::cout)
{
    typedef typename rtree<Value, Translator, Box, Tag>::translator_type translator_type;
    typedef typename rtree<Value, Translator, Box, Tag>::box_type box_type;
    visitors::rtree_print<Value, translator_type, box_type, Tag> print_v(os, tree.get_translator());
    tree.apply_visitor(print_v);
    return os;
}

template <typename Value, typename Translator, typename Box, typename Tag>
std::ostream & operator<<(std::ostream & os, rtree<Value, Translator, Box, Tag> const& tree)
{
    typedef typename rtree<Value, Translator, Box, Tag>::translator_type translator_type;
    typedef typename rtree<Value, Translator, Box, Tag>::box_type box_type;
    visitors::rtree_print<Value, translator_type, box_type, Tag> print_v(os, tree.get_translator());
    tree.apply_visitor(print_v);
    return os;
}

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_PRINT_HPP
