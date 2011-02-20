// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Bruno Lalande 2008, 2009
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_GEOMETRIES_ADAPTED_STD_AS_LINESTRING_HPP
#define BOOST_GEOMETRY_GEOMETRIES_ADAPTED_STD_AS_LINESTRING_HPP


#ifdef BOOST_GEOMETRY_ADAPTED_STD_AS_POINT_COLLECTION_TAG_DEFINED
#error Include either "std_as_linestring" or "std_as_ring" \
    or "std_as_multi_point" to adapt the std:: containers
#endif

#define BOOST_GEOMETRY_ADAPTED_STD_AS_POINT_COLLECTION_TAG_DEFINED


#include <vector>
#include <deque>
#include <list>
#include <utility>


#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/mutable_range.hpp>
#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tags.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace util
{
    struct std_as_linestring
    {
        typedef linestring_tag type;
    };

}
#endif


#ifndef DOXYGEN_NO_TRAITS_SPECIALIZATIONS
namespace traits
{
    // specialization for an iterator pair (read only)
    template <typename P> struct tag< std::pair<P, P> > : util::std_as_linestring {};

    // specializations for the std:: containers: vector, deque, list
    template <typename P> struct tag< std::vector<P> > : util::std_as_linestring {};
    template <typename P> struct tag< std::deque<P> > : util::std_as_linestring {};
    template <typename P> struct tag< std::list<P> > : util::std_as_linestring {};

}
#endif


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_GEOMETRIES_ADAPTED_STD_AS_LINESTRING_HPP
