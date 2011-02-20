// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Bruno Lalande 2008, 2009
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_GEOMETRIES_ADAPTED_BOOST_ARRAY_AS_LINESTRING_HPP
#define BOOST_GEOMETRY_GEOMETRIES_ADAPTED_BOOST_ARRAY_AS_LINESTRING_HPP


#ifdef BOOST_GEOMETRY_ADAPTED_BOOST_ARRAY_TAG_DEFINED
#error Include either "boost_array_as_point" or \
    "boost_array_as_linestring" or "boost_array_as_ring" \
    or "boost_array_as_multi_point" to adapt a boost_array
#endif

#define BOOST_GEOMETRY_ADAPTED_BOOST_ARRAY_TAG_DEFINED


#include <cstddef>
#include <boost/mpl/assert.hpp>

#include <boost/array.hpp>

#include <boost/geometry/core/mutable_range.hpp>
#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tags.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_TRAITS_SPECIALIZATIONS
namespace traits
{

template <typename Point, std::size_t PointCount>
struct tag< boost::array<Point, PointCount> >
{
    typedef linestring_tag type;
};

// boost::array is immutable with respect to size
// Therefore, prohibit compilation
template <typename Point, std::size_t PointCount>
struct clear< boost::array<Point, PointCount> >
{
    BOOST_MPL_ASSERT_MSG
        (
            false, NOT_IMPLEMENTED_FOR_BOOST_ARRAY_OF, (types<Point>)
        );
};

template <typename Point, std::size_t PointCount>
struct resize< boost::array<Point, PointCount> >
{
    BOOST_MPL_ASSERT_MSG
        (
            false, NOT_IMPLEMENTED_FOR_BOOST_ARRAY_OF, (types<Point>)
        );
};

template <typename Point, std::size_t PointCount>
struct push_back< boost::array<Point, PointCount> >
{
    BOOST_MPL_ASSERT_MSG
        (
            false, NOT_IMPLEMENTED_FOR_BOOST_ARRAY_OF, (types<Point>)
        );
};

}
#endif


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_GEOMETRIES_ADAPTED_BOOST_ARRAY_AS_LINESTRING_HPP
