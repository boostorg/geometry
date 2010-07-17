// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_CORE_CONTAINER_ACCESS_HPP
#define BOOST_GEOMETRY_CORE_CONTAINER_ACCESS_HPP

#include <cstddef>

#include <boost/type_traits/remove_const.hpp>
#include <boost/concept_check.hpp>

#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/core/point_type.hpp>
#include <boost/geometry/core/tag.hpp>


namespace boost { namespace geometry
{


namespace traits
{


/*!
\brief Traits class, optional, might be implemented to clear a geometry
\details If a geometry type should not use the std ".clear()"
    then it can specialize the "use_std" traits class to false,
    it should then implement (a.o.) clear
\ingroup traits
\par Geometries:
    - linestring
    - linear_ring
\par Specializations should provide:
    - apply
 */
template <typename Geometry>
struct clear
{
    static inline void apply(Geometry& geometry)
    {
        geometry.clear();
    }
};


/*!
\brief Traits class, optional, might be implemented to append a point
\details If a geometry type should not use the std "push_back"
    then it can specialize the "use_std" traits class to false,
    it should then implement (a.o.) append_point
\ingroup traits
\par Geometries:
    - linestring
    - linear_ring
\par Specializations should provide:
    - apply
 */
template <typename Geometry, typename Point>
struct append_point
{
    static inline void apply(Geometry& geometry, Point const& point, int , int )
    {
        typename geometry::point_type<Geometry>::type copy;
        copy_coordinates(point, copy);
        geometry.push_back(copy);
    }
};


} // namespace traits


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_CORE_CONTAINER_ACCESS_HPP
