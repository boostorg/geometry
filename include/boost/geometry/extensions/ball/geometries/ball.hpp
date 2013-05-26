// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2012 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2012 Mateusz Loskot, London, UK.

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_BALL_GEOMETRIES_BALL_HPP
#define BOOST_GEOMETRY_EXTENSIONS_BALL_GEOMETRIES_BALL_HPP

#include <cstddef>

#include <boost/geometry/algorithms/assign.hpp>
#include <boost/geometry/algorithms/convert.hpp>
#include <boost/geometry/geometries/concepts/point_concept.hpp>


#include <boost/geometry/extensions/ball/core/tags.hpp>
#include <boost/geometry/extensions/ball/algorithms/assign.hpp>
#include <boost/geometry/extensions/ball/geometries/concepts/ball_concept.hpp>

namespace boost { namespace geometry
{

namespace model
{


/*!
    \brief Class nsphere: defines a circle or a sphere: a point with radius
    \ingroup Geometry
    \details It can be a circle (2D), a sphere (3D), or higher (hypersphere) or lower.
    According to Wikipedia this name is the most appropriate. It was mentioned on the Boost list.
    An alternative is the more fancy name "sphercle" but that might be a bit too much an invention.
    \note Circle is currently used for selections, for example polygon_in_circle. Currently not all
    algorithms are implemented for n-spheres.
    \tparam P point type of the center
    \tparam T number type of the radius
 */
template <typename P,
          typename R = typename traits::coordinate_type<P>::type>
class ball
{
    BOOST_CONCEPT_ASSERT( (concept::Point<P>) );

public:

    typedef R radius_type;
    typedef typename coordinate_type<P>::type coordinate_type;

    ball()
        : m_radius(0)
    {
        assign_value(m_center, coordinate_type());
    }

    ball(P const& center, R const& radius)
        : m_radius(radius)
    {
        geometry::convert(center, m_center);
    }

    inline P const& center() const { return m_center; }
    inline R const& radius() const { return m_radius; }

    inline P& center() { return m_center; }
    inline R& radius() { return m_radius; }

private:

    P m_center;
    R m_radius;
};


} // namespace model

// Traits specializations for n-sphere above
#ifndef DOXYGEN_NO_TRAITS_SPECIALIZATIONS
namespace traits
{

template <typename Point, typename RadiusType>
struct tag<model::ball<Point, RadiusType> >
{
    typedef ball_tag type;
};

template <typename Point, typename RadiusType>
struct point_type<model::ball<Point, RadiusType> >
{
    typedef Point type;
};

template <typename Point, typename RadiusType>
struct radius_type<model::ball<Point, RadiusType> >
{
    typedef RadiusType type;
};

template <typename Point, typename Radius, std::size_t Dimension>
struct indexed_access<model::ball<Point, Radius>, center_point, Dimension>
{
    typedef typename geometry::coordinate_type<Point>::type coordinate_type;
    typedef coordinate_type type;

    static inline coordinate_type get(model::ball<Point, Radius> const& b)
    {
        return geometry::get<Dimension>(b.center());
    }

    static inline void set(model::ball<Point, Radius>& b, coordinate_type const& value)
    {
        geometry::set<Dimension>(b.center(), value);
    }
};

template <typename Point, typename Radius, std::size_t Dimension>
struct indexed_access<model::ball<Point, Radius>, radius_value, Dimension>
{
    typedef Radius radius_type;
    typedef radius_type type;

    static inline radius_type get(model::ball<Point, Radius> const& b)
    {
        return b.radius();
    }

    static inline void set(model::ball<Point, Radius>& b, radius_type const& value)
    {
        b.radius() = value;
    }
};

} // namespace traits
#endif // DOXYGEN_NO_TRAITS_SPECIALIZATIONS

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_EXTENSIONS_BALL_GEOMETRIES_BALL_HPP
