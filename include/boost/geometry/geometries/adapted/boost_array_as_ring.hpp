// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Bruno Lalande 2008, 2009
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_GEOMETRIES_ADAPTED_BOOST_ARRAY_AS_RING_HPP
#define BOOST_GEOMETRY_GEOMETRIES_ADAPTED_BOOST_ARRAY_AS_RING_HPP


#ifdef BOOST_GEOMETRY_ADAPTED_BOOST_ARRAY_TAG_DEFINED
#error Include either "boost_array_as_point" or \
    "boost_array_as_linestring" or "boost_array_as_ring" \
    or "boost_array_as_multi_point" to adapt a boost_array
#endif

#define BOOST_GEOMETRY_ADAPTED_BOOST_ARRAY_TAG_DEFINED


#include <cstddef>

#include <boost/array.hpp>

#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tags.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_TRAITS_SPECIALIZATIONS
namespace traits
{

    template <typename PointType, std::size_t DimensionCount>
    struct tag< boost::array<PointType, DimensionCount> >
    {
        typedef ring_tag type;
    };

}
#endif


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_GEOMETRIES_ADAPTED_BOOST_ARRAY_AS_RING_HPP
