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
\brief Traits class to clear a geometry
\note Might be obsolete as well...
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



} // namespace traits


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_CORE_CONTAINER_ACCESS_HPP
