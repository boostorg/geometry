// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2008-2011 Bruno Lalande, Paris, France.
// Copyright (c) 2007-2011 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2009-2011 Mateusz Loskot, London, UK.

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_GEOMETRIES_ADAPTED_C_ARRAY_AS_RING_HPP
#define BOOST_GEOMETRY_GEOMETRIES_ADAPTED_C_ARRAY_AS_RING_HPP


#ifdef BOOST_GEOMETRY_ADAPTED_C_ARRAY_AS_POINT_COLLECTION_TAG_DEFINED
#error Include either "c_array_as_linestring" or "c_array_as_ring" \
    or "c_array_as_multi_point" to adapt a c array
#endif

#define BOOST_GEOMETRY_ADAPTED_C_ARRAY_AS_POINT_COLLECTION_TAG_DEFINED


#include <cstddef>

#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tags.hpp>

namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_TRAITS_SPECIALIZATIONS
namespace traits
{

    template <typename T, std::size_t PointCount, std::size_t DimensionCount>
    struct tag< T[PointCount][DimensionCount] >
    {
        typedef ring_tag type;
    };

} // namespace traits
#endif // DOXYGEN_NO_TRAITS_SPECIALIZATIONS


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_GEOMETRIES_ADAPTED_C_ARRAY_AS_RING_HPP
