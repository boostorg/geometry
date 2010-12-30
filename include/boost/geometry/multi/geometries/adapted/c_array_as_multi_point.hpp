// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2009, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_GEOMETRIES_ADAPTED_C_ARRAY_AS_MULTI_POINT_HPP
#define BOOST_GEOMETRY_GEOMETRIES_ADAPTED_C_ARRAY_AS_MULTI_POINT_HPP


#ifdef BOOST_GEOMETRY_ADAPTED_C_ARRAY_AS_POINT_COLLECTION_TAG_DEFINED
#error Include either "c_array_as_linestring" or "c_array_as_ring" \
    or "c_array_as_multi_point" to adapt a c array
#endif

#define BOOST_GEOMETRY_ADAPTED_C_ARRAY_AS_POINT_COLLECTION_TAG_DEFINED


#include <cstddef>

#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/multi/core/tags.hpp>

namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_TRAITS_SPECIALIZATIONS
namespace traits
{

    template <typename T, std::size_t PointCount, std::size_t DimensionCount>
    struct tag< T[PointCount][DimensionCount] >
    {
        typedef multi_point_tag type;
    };

} // namespace traits
#endif // DOXYGEN_NO_TRAITS_SPECIALIZATIONS


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_GEOMETRIES_ADAPTED_C_ARRAY_AS_MULTI_POINT_HPP
