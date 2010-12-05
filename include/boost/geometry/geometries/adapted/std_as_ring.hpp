// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Bruno Lalande 2008, 2009
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_GEOMETRIES_ADAPTED_STD_AS_RING_HPP
#define BOOST_GEOMETRY_GEOMETRIES_ADAPTED_STD_AS_RING_HPP


#ifdef BOOST_GEOMETRY_ADAPTED_STD_AS_POINT_COLLECTION_TAG_DEFINED
#error Include either "std_as_linestring" or "std_as_ring" \
    or "std_as_multi_point" to adapt the std:: containers
#endif

#define BOOST_GEOMETRY_ADAPTED_STD_AS_POINT_COLLECTION_TAG_DEFINED


#include <vector>
#include <deque>
#include <list>
#include <utility>

#include <boost/geometry/core/tags.hpp>

namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_TRAITS_SPECIALIZATIONS
namespace traits
{
    // specialization for an iterator pair
    template <typename T> struct tag< std::pair<T, T> > { typedef ring_tag type; };

    // specialization for the std:: containers: vector, deque, list
    template <typename T> struct tag< std::vector<T> > { typedef ring_tag type; };
    template <typename T> struct tag< std::deque<T> > { typedef ring_tag type; };
    template <typename T> struct tag< std::list<T> > { typedef ring_tag type; };
}
#endif


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_GEOMETRIES_ADAPTED_STD_AS_RING_HPP
