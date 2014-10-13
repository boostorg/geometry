// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2014 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2014 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2014 Mateusz Loskot, London, UK.
// Copyright (c) 2013-2014 Adam Wulkiewicz, Lodz, Poland.

// This file was modified by Oracle on 2013-2014.
// Modifications copyright (c) 2013-2014, Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle
// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_DISJOINT_AREAL_AREAL_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_DISJOINT_AREAL_AREAL_HPP

#include <boost/geometry/core/point_type.hpp>

#include <boost/geometry/algorithms/within.hpp>
#include <boost/geometry/algorithms/detail/for_each_range.hpp>
#include <boost/geometry/algorithms/detail/point_on_border.hpp>

#include <boost/geometry/algorithms/detail/disjoint/linear_linear.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace disjoint
{


template <typename FirstGeometry, typename SecondGeometry>
inline bool rings_containing(FirstGeometry const& geometry1,
                                 SecondGeometry const& geometry2)
{
    typedef typename point_type<SecondGeometry>::type point_type;
    point_type point;
    geometry::point_on_border<point_type, SecondGeometry>(point, geometry2);
    return geometry::within(point, geometry1);
}



template <typename Geometry1, typename Geometry2>
struct general_areal
{
    static inline
    bool apply(Geometry1 const& geometry1, Geometry2 const& geometry2)
    {
        if ( ! disjoint_linear<Geometry1, Geometry2>::apply(geometry1, geometry2) )
        {
            return false;
        }

        // If there is no intersection of segments, they might located
        // inside each other

        // We check that using a point on the surface, and see if that is inside
        // the other geometry. And vice versa.

        if ( rings_containing(geometry1, geometry2)
          || rings_containing(geometry2, geometry1) )
        {
            return false;
        }

        return true;
    }
};


}} // namespace detail::disjoint
#endif // DOXYGEN_NO_DETAIL




#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


template <typename Areal1, typename Areal2>
struct disjoint<Areal1, Areal2, 2, areal_tag, areal_tag, false>
    : detail::disjoint::general_areal<Areal1, Areal2>
{};


template <typename Areal, typename Box>
struct disjoint<Areal, Box, 2, areal_tag, box_tag, false>
    : detail::disjoint::general_areal<Areal, Box>
{};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_DISJOINT_AREAL_AREAL_HPP
