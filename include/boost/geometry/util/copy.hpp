// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2011 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2011 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2011 Mateusz Loskot, London, UK.

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_UTIL_COPY_HPP
#define BOOST_GEOMETRY_UTIL_COPY_HPP

#if defined(BOOST_MSVC_FULL_VER)
#pragma message ("copy_coordinates is deprecated. Use 'convert'")
#else
#warning "copy_coordinates is deprecated. Use 'convert'"
#endif



#include <cstddef>

#include <boost/concept/requires.hpp>
#include <boost/concept_check.hpp>

#include <boost/geometry/geometries/concepts/point_concept.hpp>

#include <boost/numeric/conversion/cast.hpp>


namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace copy
{

template <typename Src, typename Dst, std::size_t D, std::size_t N>
struct copy_coordinates
{
    static inline void copy(Src const& source, Dst& dest)
    {
        typedef typename coordinate_type<Dst>::type coordinate_type;

        set<D>(dest, boost::numeric_cast<coordinate_type>(get<D>(source)));
        copy_coordinates<Src, Dst, D + 1, N>::copy(source, dest);
    }
};

template <typename Src, typename Dst, std::size_t N>
struct copy_coordinates<Src, Dst, N, N>
{
    static inline void copy(Src const& , Dst& )
    {
    }
};

}} // namespace detail::copy
#endif // DOXYGEN_NO_DETAIL


// Deprecated!
// This is the same as "convert", but this is specific to point, convert is more generic
template <typename Src, typename Dst>
inline void copy_coordinates(Src const& source, Dst& dest)
{
    BOOST_CONCEPT_ASSERT( (concept::ConstPoint<Src>) );
    BOOST_CONCEPT_ASSERT( (concept::Point<Dst>) );


    //assert_dimension_equal<Dst, Src>();
    detail::copy::copy_coordinates
        <
            Src,
            Dst,
            0,
            dimension<Src>::type::value
        >::copy(source, dest);
}

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_UTIL_COPY_HPP
