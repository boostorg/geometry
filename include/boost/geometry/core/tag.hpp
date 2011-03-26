// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2011 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2011 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2011 Mateusz Loskot, London, UK.

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_CORE_TAG_HPP
#define BOOST_GEOMETRY_CORE_TAG_HPP


#include <boost/mpl/assert.hpp>
#include <boost/type_traits/remove_const.hpp>

#include <boost/geometry/core/tags.hpp>


namespace boost { namespace geometry
{

namespace traits
{

/*!
\brief Traits class to attach a tag to a geometry
\details All geometries should implement a traits::tag<G>::type metafunction to indicate their
    own geometry type.
\ingroup traits
\par Geometries:
    - all geometries
\par Specializations should provide:
    - typedef XXX_tag type; (point_tag, box_tag, ...)
\tparam Geometry geometry
*/
template <typename Geometry, typename Enable = void>
struct tag
{
    typedef void type;
};

} // namespace traits


/*!
\brief Meta-function to get the tag of any geometry type
\details All geometries tell their geometry type (point, linestring, polygon, etc) by implementing
  a tag traits class. This meta-function uses that traits class to retrieve the tag.
\tparam Geometry geometry
\ingroup core
*/
template <typename Geometry>
struct tag
{
    typedef typename traits::tag
        <
            typename boost::remove_const<Geometry>::type
        >::type type;
};

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_CORE_TAG_HPP
