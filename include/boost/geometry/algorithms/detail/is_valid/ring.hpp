// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_IS_VALID_RING_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_IS_VALID_RING_HPP

#include <boost/geometry/core/closure.hpp>
#include <boost/geometry/core/point_order.hpp>

#include <boost/geometry/algorithms/equals.hpp>
#include <boost/geometry/algorithms/not_implemented.hpp>

#include <boost/geometry/views/reversible_view.hpp>
#include <boost/geometry/views/closeable_view.hpp>

#include <boost/geometry/algorithms/detail/is_valid/has_spikes.hpp>
#include <boost/geometry/algorithms/detail/is_valid/linestring.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace is_valid
{


// struct to check whether a ring is topologically closed
template <typename Ring, closure_selector Closure>
struct is_topologically_closed
{};

template <typename Ring>
struct is_topologically_closed<Ring, open>
{
    static inline bool apply(Ring const&)
    {
        return true;
    }
};

template <typename Ring>
struct is_topologically_closed<Ring, closed>
{
    static inline bool apply(Ring const& ring)
    {
        return geometry::equals(*boost::begin(ring), *boost::rbegin(ring));
    }
};


// struct to check if the size is above the minimal one
// (3 for open, 4 for closed)
template <typename Ring, closure_selector Closure>
struct is_below_minimal_size
{};

template <typename Ring>
struct is_below_minimal_size<Ring, closed>
{
    static inline bool apply(Ring const& ring)
    {
        return boost::size(ring) < 4;
    }
};

template <typename Ring>
struct is_below_minimal_size<Ring, open>
{
    static inline bool apply(Ring const& ring)
    {
        return boost::size(ring) < 3;
    }
};






template
<
    typename Ring,
    order_selector PointOrder,
    closure_selector Closure
>
struct is_valid_ring
    : not_implemented<Ring>
{};


template <typename Ring, closure_selector Closure>
struct is_valid_ring<Ring, counterclockwise, Closure>
{
    static inline bool apply(Ring const& ring)
    {
        typedef typename reversible_view
            <
                Ring, iterate_reverse
            >::type reversible_view_type;

        reversible_view_type reversed_ring(const_cast<Ring&>(ring));
        return is_valid_ring<Ring, clockwise, Closure>::apply(reversed_ring);
    }
};

template <typename Ring, closure_selector Closure>
struct is_valid_ring<Ring, clockwise, Closure>
{
    static inline bool apply(Ring const& ring)
    {
        // return invalid if any of the following condition holds:
        // (a) the ring's size is below the minimal one
        // (b) the ring has less than three distinct points
        // (c) the ring is not topologically closed
        // (d) the ring has spikes
        //
        // Note: no need to check if the area is zero. If this is the
        // case, then the ring must have at least two spikes, which is
        // checked by condition (d).
        if ( is_below_minimal_size<Ring, Closure>::apply(ring)
             || !has_three_distinct_points<Ring>::apply(ring)
             || !is_topologically_closed<Ring, Closure>::apply(ring)
             || has_spikes<Ring>::apply(ring) )
        {
            return false;
        }

        // now call self turns to compute self intersections, if any,
        // and analyze them
    }
};


}} // namespace dispatch
#endif // DOXYGEN_NO_DETAIL



#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

// A Ring is a Polygon.
// A Polygon is always a simple geometric object provided that it is valid.
//
// Reference (for polygon validity): OGC 06-103r4 (§6.1.11.1)
template <typename Ring>
struct is_valid<Ring, ring_tag>
    : detail::is_valid::is_valid_ring
        <
            Ring,
            point_order<Ring>::value,
            closure<Ring>::value
        >
{};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_IS_VALID_RING_HPP
