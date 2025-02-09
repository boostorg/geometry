// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2019 Tinko Bartels, Berlin, Germany.

// Contributed and/or modified by Tinko Bartels,
//   as part of Google Summer of Code 2019 program.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_TRIANGULATION_GEOMETRIES_VORONOI_ADAPTOR_HPP
#define BOOST_GEOMETRY_EXTENSIONS_TRIANGULATION_GEOMETRIES_VORONOI_ADAPTOR_HPP

#include <boost/geometry/extensions/triangulation/geometries/triangulation.hpp>
#include <boost/geometry/extensions/triangulation/algorithms/delaunay_triangulation.hpp>

namespace boost { namespace geometry
{

namespace model
{

template
<
    typename Triangulation
>
class voronoi_vertex_view {
public:
    typedef typename Triangulation::point_type point_type;
    typedef typename Triangulation::face_iterator face_iterator;
    voronoi_vertex_view(face_iterator f):
        m_p(strategy::delaunay_triangulation::detail::circumcircle_center
            <
                point_type,
                boost::geometry::point_order<typename Triangulation::face_type>::value == clockwise
            >( *f->begin(),
                *(f->begin() + 1),
                *(f->begin() + 2))),
        m_f(f) {}
    point_type const* point() const { return m_p; }
    face_iterator base() const { return m_f; }
    point_type m_p;
private:
    face_iterator m_f;
};

template
<
    typename Triangulation
>
class voronoi_face_view {
public:
    typedef typename Triangulation::point_type point_type;
    typedef typename Triangulation::vertex_iterator vertex_iterator;
    typedef typename Triangulation::face_iterator face_iterator;
    typedef typename std::vector<voronoi_vertex_view<Triangulation>>::const_iterator const_iterator;
    typedef typename std::vector<voronoi_vertex_view<Triangulation>>::iterator iterator;

    typedef voronoi_vertex_view<Triangulation> voronoi_vertex;
    voronoi_face_view(Triangulation& t, vertex_iterator v):m_v(v) {
        struct transforming_back_insert {
            transforming_back_insert(std::back_insert_iterator<std::vector<voronoi_vertex>> bi):
                m_underlying(bi) {};
            std::back_insert_iterator<std::vector<voronoi_vertex>> m_underlying;
            transforming_back_insert& operator*() { return *this; }
            transforming_back_insert& operator++() { return *this; }
            transforming_back_insert& operator++(int) { return *this; }
            transforming_back_insert& operator=(face_iterator const& f)
            {
                m_underlying = voronoi_vertex(f);
                return *this;
            }
        };
        vertex_incident_faces(t, m_v, transforming_back_insert(std::back_inserter(m_voronoi_v)));
    }
    const_iterator begin() const { return m_voronoi_v.begin(); }
    const_iterator end() const { return m_voronoi_v.end(); }
private:
    std::vector<voronoi_vertex> m_voronoi_v;
    vertex_iterator m_v;
};

} // namespace model

#ifndef DOXYGEN_NO_TRAITS_SPECIALIZATIONS
namespace traits
{

template<typename Triangulation>
struct dimension< model::voronoi_vertex_view<Triangulation> > : boost::mpl::int_<2> {};

template<typename Triangulation>
struct tag< model::voronoi_vertex_view<Triangulation> >
{ typedef point_tag type; };

template<typename Triangulation>
struct coordinate_type< model::voronoi_vertex_view<Triangulation> >
{ typedef typename coordinate_type<typename Triangulation::point_type>::type type; };

template<typename Triangulation>
struct coordinate_system< model::voronoi_vertex_view<Triangulation> >
{ typedef typename coordinate_system<typename Triangulation::point_type>::type type; };

template<typename Triangulation, std::size_t Dimension>
struct access<model::voronoi_vertex_view<Triangulation>, Dimension>
{
    static typename coordinate_type<typename Triangulation::point_type>::type
        get(model::voronoi_vertex_view<Triangulation> const& p)
    {
        return boost::geometry::get<Dimension>(p.m_p);
    }
};

template<typename Triangulation> struct tag<model::voronoi_face_view<Triangulation>>
{ typedef ring_tag type; };

template<typename Triangulation> struct point_order<model::voronoi_face_view<Triangulation>>
{ static const order_selector value = counterclockwise; };

template<typename Triangulation>
struct closure<model::voronoi_face_view<Triangulation>>
{
    static const closure_selector value = open;
};

} // namespace traits
#endif // DOXYGEN_NO_TRAITS_SPECIALIZATIONS

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_EXTENSIONS_TRIANGULATION_GEOMETRIES_VORONOI_ADAPTOR_HPP
