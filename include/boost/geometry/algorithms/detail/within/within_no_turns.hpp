// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2012 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2012 Mateusz Loskot, London, UK.

// This file was modified by Oracle on 2013.
// Modifications copyright (c) 2013, Oracle and/or its affiliates.

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_WITHIN_WITHIN_NO_TURNS_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_WITHIN_WITHIN_NO_TURNS_HPP

#include <boost/geometry/algorithms/detail/point_on_border.hpp>
#include <boost/geometry/algorithms/detail/within/point_in_geometry.hpp>

namespace boost { namespace geometry {

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace within {

// returns true if G1 is within G2
// this function should be called only if there are no intersection points
// otherwise it may return invalid result
// e.g. when non-first point of G1 is outside G2 or when some rings of G1 are the same as rings of G2

template <typename G1, typename G2, typename T1 = typename geometry::tag<G1>::type, typename T2 = typename geometry::tag<G2>::type>
struct within_no_turns_dispatch
{
    template <typename S> static inline
    bool apply(G1 const& g1, G2 const& g2, S const& s)
    {
        typedef typename geometry::point_type<G1>::type point1_type;
        point1_type p;
        if ( !geometry::point_on_border(p, g1) )
            return false;

        return point_in_geometry(p, g2, s) >= 0;
    }
};

template <typename G1, typename G2>
struct within_no_turns_dispatch<G1, G2, ring_tag, polygon_tag>
{
    template <typename S> static inline
    bool apply(G1 const& g1, G2 const& g2, S const& s)
    {
        typedef typename geometry::point_type<G1>::type point1_type;
        typedef typename geometry::point_type<G2>::type point2_type;
        point1_type p;
        if ( !geometry::point_on_border(p, g1) )
            return false;
        // check if one of ring points is outside the polygon
        if ( point_in_geometry(p, g2, s) < 0 )
            return false;
        // Now check if holes of G2 aren't inside G1
        typedef typename boost::range_const_iterator
            <
                typename geometry::interior_type<G2>::type
            >::type iterator;
        for ( iterator it = boost::begin(geometry::interior_rings(g2)) ;
              it != boost::end(geometry::interior_rings(g2)) ;
              ++it )
        {
            point2_type p;
            if ( !geometry::point_on_border(p, *it) )
                return false;
            if ( point_in_geometry(p, g1, s) > 0 )
                return false;
        }
        return true;
    }
};

template <typename G1, typename G2>
struct within_no_turns_dispatch<G1, G2, polygon_tag, polygon_tag>
{
    template <typename S> static inline
    bool apply(G1 const& g1, G2 const& g2, S const& s)
    {
        typedef typename geometry::point_type<G1>::type point1_type;
        typedef typename geometry::point_type<G2>::type point2_type;
        point1_type p;
        if ( !geometry::point_on_border(p, g1) )
            return false;
        // check if one of ring points is outside the polygon
        if ( point_in_geometry(p, g2, s) < 0 )
            return false;
        // Now check if holes of G2 aren't inside G1
        typedef typename boost::range_const_iterator
            <
                typename geometry::interior_type<G2>::type
            >::type iterator2;
        for ( iterator2 it = boost::begin(geometry::interior_rings(g2)) ;
              it != boost::end(geometry::interior_rings(g2)) ;
              ++it )
        {
            point2_type p2;
            if ( !geometry::point_on_border(p2, *it) )
                return false;
            // if the hole of G2 is inside G1
            if ( point_in_geometry(p2, g1, s) > 0 )
            {
                // if it's also inside one of the G1 holes, it's ok
                bool ok = false;
                typedef typename boost::range_const_iterator
                    <
                        typename geometry::interior_type<G1>::type
                    >::type iterator1;
                for ( iterator1 it1 = boost::begin(geometry::interior_rings(g1)) ;
                      it1 != boost::end(geometry::interior_rings(g1)) ;
                      ++it1 )
                {
                    if ( point_in_geometry(p2, *it1, s) < 0 )
                    {
                        ok = true;
                        break;
                    }
                }
                if ( !ok )
                    return false;
            }
        }
        return true;
    }
};

// TODO: later move it to directory boost/geometry/multi/algorithms/detail/within

template <typename G1,
          typename G2,
          typename T1 = typename geometry::tag<G1>::type,
          typename T2 = typename geometry::tag<G2>::type,
          bool IsMulti1 = boost::is_base_of<geometry::multi_tag, T1>::value,
          bool IsMulti2 = boost::is_base_of<geometry::multi_tag, T2>::value>
struct within_no_turns_multi_dispatch
{
    template <typename S> static inline
    bool apply(G1 const& g1, G2 const& g2, S const& s)
    {
        return within_no_turns_dispatch<G1, G2>::apply(g1, g2, s);
    }
};

template <typename G1, typename G2, typename T1, typename T2>
struct within_no_turns_multi_dispatch<G1, G2, T1, T2, true, false>
{
    template <typename S> static inline
    bool apply(G1 const& g1, G2 const& g2, S const& s)
    {
        // All values of G1 must be inside G2
        typedef typename boost::range_value<G1>::type subgeometry1;
        typedef typename boost::range_const_iterator<G1>::type iterator;
        for ( iterator it = boost::begin(g1) ; it != boost::end(g1) ; ++it )
        {
            if ( !within_no_turns_dispatch<subgeometry1, G2>::apply(*it, g2, s) )
                return false;
        }
        return true;
    }
};

template <typename G1, typename G2, typename T1, typename T2>
struct within_no_turns_multi_dispatch<G1, G2, T1, T2, false, true>
{
    template <typename S> static inline
    bool apply(G1 const& g1, G2 const& g2, S const& s)
    {
        // G1 must be within at least one value of G2
        typedef typename boost::range_value<G2>::type subgeometry2;
        typedef typename boost::range_const_iterator<G2>::type iterator;
        for ( iterator it = boost::begin(g2) ; it != boost::end(g2) ; ++it )
        {
            if ( within_no_turns_dispatch<G1, subgeometry2>::apply(g1, *it, s) )
                return true;
        }
        return false;
    }
};

template <typename G1, typename G2, typename T1, typename T2>
struct within_no_turns_multi_dispatch<G1, G2, T1, T2, true, true>
{
    template <typename S> static inline
    bool apply(G1 const& g1, G2 const& g2, S const& s)
    {
        // each value of G1 must be inside at least one value of G2
        typedef typename boost::range_value<G1>::type subgeometry1;
        typedef typename boost::range_const_iterator<G1>::type iterator;
        for ( iterator it = boost::begin(g1) ; it != boost::end(g1) ; ++it )
        {
            if ( !within_no_turns_multi_dispatch<subgeometry1, G2>::apply(*it, g2, s) )
                return false;
        }
        return true;
    }
};

template <typename G1, typename G2, typename S> inline
bool within_no_turns(G1 const& g1, G2 const& g2, S const& s)
{
    return within_no_turns_multi_dispatch<G1, G2>::apply(g1, g2, s);
}

}} // namespace detail::within
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_WITHIN_WITHIN_NO_TURNS_HPP
