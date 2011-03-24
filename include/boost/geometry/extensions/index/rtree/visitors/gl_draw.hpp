// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.Index - R-tree GL-draw visitor
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_GL_DRAW_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_GL_DRAW_HPP

#include <iostream>

#include <boost/geometry/extensions/index/rtree/rtree_node.hpp>

namespace boost { namespace geometry { namespace index {

namespace visitors {

namespace dispatch {

template <typename Point, size_t Dimension>
struct rtree_gl_draw_point
{};

template <typename Point>
struct rtree_gl_draw_point<Point, 2>
{
    static inline void apply(Point const& p)
    {
        glBegin(GL_POINT);
        glVertex2f(geometry::get<0>(p), geometry::get<1>(p));
        glEnd();
    }
};

template <typename Box, size_t Dimension>
struct rtree_gl_draw_box
{};

template <typename Box>
struct rtree_gl_draw_box<Box, 2>
{
    static inline void apply(Box const& b)
    {
        glBegin(GL_LINE_LOOP);
        glVertex2f(geometry::get<min_corner, 0>(b), geometry::get<min_corner, 1>(b));
        glVertex2f(geometry::get<max_corner, 0>(b), geometry::get<min_corner, 1>(b));
        glVertex2f(geometry::get<max_corner, 0>(b), geometry::get<max_corner, 1>(b));
        glVertex2f(geometry::get<min_corner, 0>(b), geometry::get<max_corner, 1>(b));
        glEnd();
    }
};

template <typename Indexable, typename Tag>
struct rtree_gl_draw_indexable
{
};

template <typename Indexable>
struct rtree_gl_draw_indexable<Indexable, box_tag>
{
    typedef typename geometry::traits::point_type<Indexable>::type point_type;
    static const size_t dimension = geometry::traits::dimension<point_type>::value;

    static inline void apply(Indexable const& i)
    {
        rtree_gl_draw_box<Indexable, dimension>::apply(i);
    }
};

template <typename Indexable>
struct rtree_gl_draw_indexable<Indexable, point_tag>
{
    static const size_t dimension = geometry::traits::dimension<Indexable>::value;

    static inline void apply(std::ostream &os, Indexable const& i)
    {
        rtree_gl_draw_point<Indexable, dimension>::apply(i);
    }
};

} // namespace dispatch

namespace detail {

template <typename Indexable>
inline void rtree_gl_draw_indexable(Indexable const& i)
{
    dispatch::rtree_gl_draw_indexable<
        Indexable,
        typename geometry::traits::tag<Indexable>::type
    >::apply(i);
}

} // namespace detail

template <typename Value, typename Translator, typename Box, typename Tag>
struct rtree_gl_draw : public boost::static_visitor<>
{
    typedef typename index::detail::rtree_internal_node<Value, Box, Tag>::type internal_node;
    typedef typename index::detail::rtree_leaf<Value, Box, Tag>::type leaf;

    inline rtree_gl_draw(Translator const& t)
        : tr(t), level(0)
    {}

    inline void operator()(internal_node const& n)
    {
        typedef typename internal_node::children_type children_type;

        if ( level == 0 )
            glColor3f(1.0f, 0.0f, 0.0f);
        else if ( level == 1 )
            glColor3f(0.0f, 1.0f, 0.0f);
        else if ( level == 2 )
            glColor3f(0.0f, 0.0f, 1.0f);
        else if ( level == 3 )
            glColor3f(1.0f, 1.0f, 0.0f);
        else if ( level == 4 )
            glColor3f(1.0f, 0.0f, 1.0f);
        else if ( level == 5 )
            glColor3f(0.0f, 1.0f, 1.0f);
        else
            glColor3f(0.5f, 0.5f, 0.5f);

        for (typename children_type::const_iterator it = n.children.begin();
            it != n.children.end(); ++it)
        {
            detail::rtree_gl_draw_indexable(it->first);
        }
        
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

        glColor3f(1.0f, 1.0f, 1.0f);

        for (typename values_type::const_iterator it = n.values.begin();
            it != n.values.end(); ++it)
        {
            detail::rtree_gl_draw_indexable(tr(*it));
        }
    }

    Translator const& tr;

    size_t level;
};

} // namespace visitors

template <typename Value, typename Translator, typename Box, typename Tag>
void gl_draw(rtree<Value, Translator, Box, Tag> const& tree)
{
    typedef typename rtree<Value, Translator, Box, Tag>::translator_type translator_type;
    typedef typename rtree<Value, Translator, Box, Tag>::box_type box_type;

    glClear(GL_COLOR_BUFFER_BIT);

    visitors::rtree_gl_draw<Value, translator_type, box_type, Tag> gl_draw_v(tree.get_translator());
    tree.apply_visitor(gl_draw_v);

    glFlush();
}

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_GL_DRAW_HPP
