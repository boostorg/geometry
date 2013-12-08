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

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_WITHIN_POINT_IN_GEOMETRY_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_WITHIN_POINT_IN_GEOMETRY_HPP

#include <boost/geometry/algorithms/detail/disjoint/point_point.hpp>

namespace boost { namespace geometry {

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace within {

template <typename Point, typename Range, typename Strategy> inline
int point_in_range(Point const& point, Range const& range, Strategy const& strategy)
{
    typedef typename boost::range_iterator<Range const>::type iterator_type;
    typename Strategy::state_type state;
    iterator_type it = boost::begin(range);
    iterator_type end = boost::end(range);

    bool stop = false;
    for ( iterator_type previous = it++ ;
          it != end && ! stop ;
          ++previous, ++it )
    {
        if ( ! strategy.apply(point, *previous, *it, state) )
        {
            stop = true;
        }
    }

    return strategy.result(state);
}

// checks the relation between a point P and geometry G
// returns 1 if P is in the interior of G
// returns 0 if P is on the boundry of G
// returns -1 if P is in the exterior of G

template <typename G, typename T = typename geometry::tag<G>::type>
struct point_in_geometry_dispatch
{
    BOOST_MPL_ASSERT_MSG(false, NOT_IMPLEMENTED_FOR_THIS_TAG, (T));
};

template <typename Linestring>
struct point_in_geometry_dispatch<Linestring, linestring_tag>
{
    template <typename Point, typename Strategy> static inline
    int apply(Point const& pt, Linestring const& ls, Strategy const& strategy)
    {
        std::size_t count = boost::size(ls);
        if ( 2 <= count )
        {
            if ( 0 != detail::within::point_in_range(pt, ls, strategy) )
                return -1;

            // if the linestring doesn't have a boundary
            if ( detail::equals::equals_point_point(*boost::begin(ls), *(--boost::end(ls))) )
                return 1;
            // else if the point is equal to the one of the terminal points
            else if ( detail::equals::equals_point_point(pt, *boost::begin(ls))
                   || detail::equals::equals_point_point(pt, *(--boost::end(ls))) )
                return 0;
            else
                return 1;
        }
        else if ( 1 == count
               && detail::equals::equals_point_point(pt, *boost::begin(ls)) )
            return 0;

        return -1;
    }
};

template <typename Ring>
struct point_in_geometry_dispatch<Ring, ring_tag>
{
    template <typename Point, typename Strategy> static inline
    int apply(Point const& point, Ring const& ring, Strategy const& strategy)
    {
        static const iterate_direction direction = order_as_direction<geometry::point_order<Ring>::value>::value;
        static const closure_selector closure = geometry::closure<Ring>::value;

        if (int(boost::size(ring))
                < core_detail::closure::minimum_ring_size<closure>::value)
        {
            return -1;
        }

        typedef typename reversible_view<Ring const, direction>::type rev_view_type;
        typedef typename closeable_view
            <
                rev_view_type const, closure
            >::type cl_view_type;
        typedef typename boost::range_iterator<cl_view_type const>::type iterator_type;

        rev_view_type rev_view(ring);
        cl_view_type view(rev_view);

        return point_in_range(point, view, strategy);
    }
};

//// Polygon: in exterior ring, and if so, not within interior ring(s)
template <typename Polygon>
struct point_in_geometry_dispatch<Polygon, polygon_tag>
{
    template <typename Point, typename Strategy>
    static inline int apply(Point const& point, Polygon const& poly,
                            Strategy const& strategy)
    {
        int const code = point_in_geometry_dispatch
            <
                typename ring_type<Polygon>::type
            >::apply(point, exterior_ring(poly), strategy);

        if (code == 1)
        {
            typename interior_return_type<Polygon const>::type rings
                        = interior_rings(poly);
            for (BOOST_AUTO_TPL(it, boost::begin(rings));
                it != boost::end(rings);
                ++it)
            {
                int const interior_code = point_in_geometry_dispatch
                    <
                        typename ring_type<Polygon>::type
                    >::apply(point, *it, strategy);

                if (interior_code != -1)
                {
                    // If 0, return 0 (touch)
                    // If 1 (inside hole) return -1 (outside polygon)
                    // If -1 (outside hole) check other holes if any
                    return -interior_code;
                }
            }
        }
        return code;
    }
};

// 1 - in the interior
// 0 - in the boundry
// -1 - in the exterior
template <typename P, typename G, typename Strategy> inline
int point_in_geometry(P const& p, G const& g, Strategy const& strategy)
{
    BOOST_CONCEPT_ASSERT( (geometry::concept::WithinStrategyPolygonal<Strategy>) );

    return point_in_geometry_dispatch<G>::apply(p, g, strategy);
}

}} // namespace detail::within
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_WITHIN_POINT_IN_GEOMETRY_HPP
