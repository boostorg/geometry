// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_FOR_EACH_HPP
#define BOOST_GEOMETRY_ALGORITHMS_FOR_EACH_HPP


#include <algorithm>

#include <boost/range.hpp>
#include <boost/typeof/typeof.hpp>

#include <boost/geometry/core/exterior_ring.hpp>
#include <boost/geometry/core/interior_rings.hpp>
#include <boost/geometry/core/is_multi.hpp>

#include <boost/geometry/geometries/concepts/check.hpp>

#include <boost/geometry/geometries/segment.hpp>

#include <boost/geometry/util/add_const_if_c.hpp>


namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace for_each
{


template <typename Point, typename Functor, bool IsConst>
struct fe_point_per_point
{
    static inline Functor apply(
                typename add_const_if_c<IsConst, Point>::type& point, Functor f)
    {
        f(point);
        return f;
    }
};


template <typename Point, typename Functor, bool IsConst>
struct fe_point_per_segment
{
    static inline Functor apply(
                typename add_const_if_c<IsConst, Point>::type& point, Functor f)
    {
        return f;
    }
};


template <typename Range, typename Functor, bool IsConst>
struct fe_range_per_point
{
    static inline Functor apply(
                    typename add_const_if_c<IsConst, Range>::type& range,
                    Functor f)
    {
        return (std::for_each(boost::begin(range), boost::end(range), f));
    }
};


template <typename Range, typename Functor, bool IsConst>
struct fe_range_per_segment
{
    static inline Functor apply(
                typename add_const_if_c<IsConst, Range>::type& range,
                Functor f)
    {
        typedef typename add_const_if_c
            <
                IsConst,
                typename point_type<Range>::type
            >::type point_type;

        BOOST_AUTO(it, boost::begin(range));
        BOOST_AUTO(previous, it++);
        while(it != boost::end(range))
        {
            model::referring_segment<point_type> s(*previous, *it);
            f(s);
            previous = it++;
        }

        return f;
    }
};


template <typename Polygon, typename Functor, bool IsConst>
struct fe_polygon_per_point
{
    typedef typename add_const_if_c<IsConst, Polygon>::type poly_type;

    static inline Functor apply(poly_type& poly, Functor f)
    {
        typedef fe_range_per_point
                <
                    typename ring_type<Polygon>::type,
                    Functor,
                    IsConst
                > per_ring;

        f = per_ring::apply(exterior_ring(poly), f);

        typename interior_return_type<poly_type>::type rings
                    = interior_rings(poly);
        for (BOOST_AUTO(it, boost::begin(rings)); it != boost::end(rings); ++it)
        {
            f = per_ring::apply(*it, f);
        }

        return f;
    }

};


template <typename Polygon, typename Functor, bool IsConst>
struct fe_polygon_per_segment
{
    typedef typename add_const_if_c<IsConst, Polygon>::type poly_type;

    static inline Functor apply(poly_type& poly, Functor f)
    {
        typedef fe_range_per_segment
            <
                typename ring_type<Polygon>::type,
                Functor,
                IsConst
            > per_ring;

        f = per_ring::apply(exterior_ring(poly), f);

        typename interior_return_type<poly_type>::type rings
                    = interior_rings(poly);
        for (BOOST_AUTO(it, boost::begin(rings)); it != boost::end(rings); ++it)
        {
            f = per_ring::apply(*it, f);
        }

        return f;
    }

};


}} // namespace detail::for_each
#endif // DOXYGEN_NO_DETAIL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template
<
    typename Tag,
    bool IsMulti,
    typename Geometry,
    typename Functor,
    bool IsConst
>
struct for_each_point {};


template <typename Point, typename Functor, bool IsConst>
struct for_each_point<point_tag, false, Point, Functor, IsConst>
    : detail::for_each::fe_point_per_point<Point, Functor, IsConst>
{};


template <typename Linestring, typename Functor, bool IsConst>
struct for_each_point<linestring_tag, false, Linestring, Functor, IsConst>
    : detail::for_each::fe_range_per_point<Linestring, Functor, IsConst>
{};


template <typename Ring, typename Functor, bool IsConst>
struct for_each_point<ring_tag, false, Ring, Functor, IsConst>
    : detail::for_each::fe_range_per_point<Ring, Functor, IsConst>
{};


template <typename Polygon, typename Functor, bool IsConst>
struct for_each_point<polygon_tag, false, Polygon, Functor, IsConst>
    : detail::for_each::fe_polygon_per_point<Polygon, Functor, IsConst>
{};


template
<
    typename Tag,
    bool IsMulti,
    typename Geometry,
    typename Functor,
    bool IsConst
>
struct for_each_segment {};

template <typename Point, typename Functor, bool IsConst>
struct for_each_segment<point_tag, false, Point, Functor, IsConst>
    : detail::for_each::fe_point_per_segment<Point, Functor, IsConst>
{};


template <typename Linestring, typename Functor, bool IsConst>
struct for_each_segment<linestring_tag, false, Linestring, Functor, IsConst>
    : detail::for_each::fe_range_per_segment<Linestring, Functor, IsConst>
{};


template <typename Ring, typename Functor, bool IsConst>
struct for_each_segment<ring_tag, false, Ring, Functor, IsConst>
    : detail::for_each::fe_range_per_segment<Ring, Functor, IsConst>
{};


template <typename Polygon, typename Functor, bool IsConst>
struct for_each_segment<polygon_tag, false, Polygon, Functor, IsConst>
    : detail::for_each::fe_polygon_per_segment<Polygon, Functor, IsConst>
{};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


/*!
    \brief Calls functor for geometry
    \ingroup for_each
    \param geometry geometry to loop through
    \param f functor to use
    \details Calls the functor the specified \b const geometry
*/
template<typename Geometry, typename Functor>
inline Functor for_each_point(Geometry const& geometry, Functor f)
{
    concept::check<Geometry const>();

    return dispatch::for_each_point
        <
            typename tag<Geometry>::type,
            is_multi<Geometry>::type::value,
            Geometry,
            Functor,
            true
        >::apply(geometry, f);
}


/*!
    \brief Calls functor for geometry
    \ingroup for_each
    \param geometry geometry to loop through
    \param f functor to use
    \details Calls the functor for the specified geometry
*/
template<typename Geometry, typename Functor>
inline Functor for_each_point(Geometry& geometry, Functor f)
{
    concept::check<Geometry>();

    return dispatch::for_each_point
        <
            typename tag<Geometry>::type,
            is_multi<Geometry>::type::value,
            Geometry,
            Functor,
            false
        >::apply(geometry, f);
}


/*!
    \brief Calls functor for segments on linestrings, rings, polygons, ...
    \ingroup for_each
    \param geometry geometry to loop through
    \param f functor to use
    \details Calls the functor all \b const segments of the
        specified \b const geometry
*/
template<typename Geometry, typename Functor>
inline Functor for_each_segment(Geometry const& geometry, Functor f)
{
    concept::check<Geometry const>();

    return dispatch::for_each_segment
        <
            typename tag<Geometry>::type,
            is_multi<Geometry>::type::value,
            Geometry,
            Functor,
            true
        >::apply(geometry, f);
}


/*!
    \brief Calls functor for segments on linestrings, rings, polygons, ...
    \ingroup for_each
    \param geometry geometry to loop through
    \param f functor to use
    \details Calls the functor all segments of the specified geometry
*/
template<typename Geometry, typename Functor>
inline Functor for_each_segment(Geometry& geometry, Functor f)
{
    concept::check<Geometry>();

    return dispatch::for_each_segment
        <
            typename tag<Geometry>::type,
            is_multi<Geometry>::type::value,
            Geometry,
            Functor,
            false
        >::apply(geometry, f);
}


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_FOR_EACH_HPP
