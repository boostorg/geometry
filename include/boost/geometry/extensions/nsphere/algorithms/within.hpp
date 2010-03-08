// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_NSPHERE_ALGORITHMS_WITHIN_HPP
#define BOOST_GEOMETRY_EXTENSIONS_NSPHERE_ALGORITHMS_WITHIN_HPP


#include <boost/geometry/algorithms/make.hpp>
#include <boost/geometry/algorithms/within.hpp>

#include <boost/geometry/multi/core/tags.hpp>

#include <boost/geometry/extensions/nsphere/core/access.hpp>
#include <boost/geometry/extensions/nsphere/core/tags.hpp>
#include <boost/geometry/extensions/nsphere/algorithms/assign.hpp>


namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace within {



//-------------------------------------------------------------------------------------------------------
// Implementation for n-spheres. Supports circles or spheres, in 2 or 3 dimensions, in Euclidian system
// Circle center might be of other point-type as geometry
// Todo: implement as strategy
//-------------------------------------------------------------------------------------------------------
template<typename P, typename C>
inline bool point_in_circle(P const& p, C const& c)
{
    assert_dimension<C, 2>();

    typedef typename point_type<C>::type point_type;
    typedef typename strategy_distance
        <
            typename cs_tag<P>::type,
            typename cs_tag<point_type>::type,
            P,
            point_type
        >::type strategy_type;
    typedef typename strategy_type::return_type return_type;

    P const center = geometry::make<P>(get<0>(c), get<1>(c));
    strategy_type distance;
    return_type const r = geometry::distance(p, center);
    return_type const rad = make_distance_result<return_type>(get_radius<0>(c));

    return r < rad;
}
/// 2D version
template<typename T, typename C>
inline bool point_in_circle(const T& c1, const T& c2, C const& c)
{
    typedef typename point_type<C>::type point_type;

    point_type p = geometry::make<point_type>(c1, c2);
    return point_in_circle(p, c);
}

template<typename B, typename C>
inline bool box_in_circle(B const& b, C const& c)
{
    typedef typename point_type<B>::type point_type;

    // Currently only implemented for 2d geometries
    assert_dimension<point_type, 2>();
    assert_dimension<C, 2>();

    // Box: all four points must lie within circle

    // Check points lower-left and upper-right, then lower-right and upper-left
    return point_in_circle(get<min_corner, 0>(b), get<min_corner, 1>(b), c)
        && point_in_circle(get<max_corner, 0>(b), get<max_corner, 1>(b), c)
        && point_in_circle(get<min_corner, 0>(b), get<max_corner, 1>(b), c)
        && point_in_circle(get<max_corner, 0>(b), get<min_corner, 1>(b), c);
}

// Generic "range-in-circle", true if all points within circle
template<typename R, typename C>
inline bool range_in_circle(R const& range, C const& c)
{
    assert_dimension<R, 2>();
    assert_dimension<C, 2>();

    for (typename boost::range_const_iterator<R>::type it = boost::begin(range);
         it != boost::end(range); ++it)
    {
        if (! point_in_circle(*it, c))
        {
            return false;
        }
    }

    return true;
}

template<typename Y, typename C>
inline bool polygon_in_circle(Y const& poly, C const& c)
{
    return range_in_circle(exterior_ring(poly), c);
}



template<typename I, typename C>
inline bool multi_polygon_in_circle(const I& m, const C& c)
{
    for (typename I::const_iterator i = m.begin(); i != m.end(); i++)
    {
        if (! polygon_in_circle(*i, c))
        {
            return false;
        }
    }
    return true;
}



}} // namespace detail::within
#endif // DOXYGEN_NO_DETAIL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


template <typename P, typename Circle, typename Strategy>
struct within<point_tag, nsphere_tag, P, Circle, Strategy>
{
    static inline bool apply(P const& p, Circle const& c)
    {
        return detail::within::point_in_circle(p, c);
    }
};

template <typename Box, typename Circle, typename Strategy>
struct within<box_tag, nsphere_tag, Box, Circle, Strategy>
{
    static inline bool apply(Box const& b, Circle const& c)
    {
        return detail::within::box_in_circle(b, c);
    }
};

template <typename Linestring, typename Circle, typename Strategy>
struct within<linestring_tag, nsphere_tag, Linestring, Circle, Strategy>
{
    static inline bool apply(Linestring const& ln, Circle const& c)
    {
        return detail::within::range_in_circle(ln, c);
    }
};

template <typename Ring, typename Circle, typename Strategy>
struct within<ring_tag, nsphere_tag, Ring, Circle, Strategy>
{
    static inline bool apply(Ring const& r, Circle const& c)
    {
        return detail::within::range_in_circle(r, c);
    }
};

template <typename Polygon, typename Circle, typename Strategy>
struct within<polygon_tag, nsphere_tag, Polygon, Circle, Strategy>
{
    static inline bool apply(Polygon const& poly, Circle const& c)
    {
        return detail::within::polygon_in_circle(poly, c);
    }
};

template <typename M, typename C, typename Strategy>
struct within<multi_polygon_tag, nsphere_tag, M, C, Strategy>
{
    static inline bool apply(const M& m, const C& c)
    {
        return detail::within::multi_polygon_in_circle(m, c);
    }
};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_EXTENSIONS_NSPHERE_ALGORITHMS_WITHIN_HPP
