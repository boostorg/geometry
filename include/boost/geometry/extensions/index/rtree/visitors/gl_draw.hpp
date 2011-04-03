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

#include <boost/geometry/extensions/index/rtree/node.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree { namespace visitors {

namespace dispatch {

template <typename Point, size_t Dimension>
struct gl_draw_point
{};

template <typename Point>
struct gl_draw_point<Point, 2>
{
    static inline void apply(Point const& p, size_t level)
    {
        glBegin(GL_POINT);
        glVertex2f(geometry::get<0>(p), geometry::get<1>(p), level);
        glEnd();
    }
};

template <typename Box, size_t Dimension>
struct gl_draw_box
{};

template <typename Box>
struct gl_draw_box<Box, 2>
{
    static inline void apply(Box const& b, size_t level)
    {
        glBegin(GL_LINE_LOOP);
        glVertex3f(geometry::get<min_corner, 0>(b), geometry::get<min_corner, 1>(b), level);
        glVertex3f(geometry::get<max_corner, 0>(b), geometry::get<min_corner, 1>(b), level);
        glVertex3f(geometry::get<max_corner, 0>(b), geometry::get<max_corner, 1>(b), level);
        glVertex3f(geometry::get<min_corner, 0>(b), geometry::get<max_corner, 1>(b), level);
        glEnd();
    }
};

template <typename Indexable, typename Tag>
struct gl_draw_indexable
{
};

template <typename Indexable>
struct gl_draw_indexable<Indexable, box_tag>
{
    static const size_t dimension = index::traits::dimension<Indexable>::value;

    static inline void apply(Indexable const& i, size_t level)
    {
        gl_draw_box<Indexable, dimension>::apply(i, level);
    }
};

template <typename Indexable>
struct gl_draw_indexable<Indexable, point_tag>
{
    static const size_t dimension = index::traits::dimension<Indexable>::value;

    static inline void apply(Indexable const& i, size_t level)
    {
        gl_draw_point<Indexable, dimension>::apply(i, level);
    }
};

} // namespace dispatch

namespace detail {

template <typename Indexable>
inline void gl_draw_indexable(Indexable const& i, size_t level)
{
    dispatch::gl_draw_indexable<
        Indexable,
        typename index::traits::tag<Indexable>::type
    >::apply(i, level);
}

} // namespace detail

template <typename Value, typename Translator, typename Box, typename Tag>
struct gl_draw : public boost::static_visitor<>
{
    typedef typename rtree::internal_node<Value, Box, Tag>::type internal_node;
    typedef typename rtree::leaf<Value, Box, Tag>::type leaf;

    inline gl_draw(Translator const& t)
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
            detail::gl_draw_indexable(it->first, level);
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
            detail::gl_draw_indexable(tr(*it), level);
        }
    }

    Translator const& tr;

    size_t level;
};

}}} // namespace detail::rtree::visitors

template <typename Value, typename Translator, typename Tag>
void gl_draw(rtree<Value, Translator, Tag> const& tree)
{
    typedef typename rtree<Value, Translator, Tag>::value_type value_type;
    typedef typename rtree<Value, Translator, Tag>::translator_type translator_type;
    typedef typename rtree<Value, Translator, Tag>::box_type box_type;
    typedef typename rtree<Value, Translator, Tag>::tag_type tag_type;

    glClear(GL_COLOR_BUFFER_BIT);

    detail::rtree::visitors::gl_draw<value_type, translator_type, box_type, tag_type> gl_draw_v(tree.get_translator());
    tree.apply_visitor(gl_draw_v);

    glFlush();
}

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_GL_DRAW_HPP
