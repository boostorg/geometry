// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_NUM_POINTS_HPP
#define BOOST_GEOMETRY_ALGORITHMS_NUM_POINTS_HPP

#include <cstddef>

#include <boost/range.hpp>
#include <boost/type_traits/remove_const.hpp>

#include <boost/geometry/core/is_linear.hpp>
#include <boost/geometry/core/interior_rings.hpp>

#include <boost/geometry/geometries/concepts/check.hpp>


namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace num_points {


template <typename Range>
struct range_count
{
    static inline std::size_t apply(Range const& range)
    {
        return boost::size(range);
    }
};

template <typename Geometry, std::size_t D>
struct other_count
{
    static inline std::size_t apply(Geometry const& )
    {
        return D;
    }
};

template <typename Polygon>
struct polygon_count
{
    static inline std::size_t apply(Polygon const& poly)
    {
        std::size_t n = boost::size(exterior_ring(poly));
        typedef typename boost::range_const_iterator
            <
                typename interior_type<Polygon>::type
            >::type iterator;

        for (iterator it = boost::begin(interior_rings(poly));
             it != boost::end(interior_rings(poly));
             ++it)
        {
            n += boost::size(*it);
        }

        return n;
    }
};

}} // namespace detail::num_points
#endif // DOXYGEN_NO_DETAIL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template <typename GeometryTag, bool Linear, typename Geometry>
struct num_points
{
};

template <typename GeometryTag, typename Geometry>
struct num_points<GeometryTag, true, Geometry>
        : detail::num_points::range_count<Geometry>
{
};

template <typename Geometry>
struct num_points<point_tag, false, Geometry>
        : detail::num_points::other_count<Geometry, 1>
{
};

template <typename Geometry>
struct num_points<box_tag, false, Geometry>
        : detail::num_points::other_count<Geometry, 4>
{
};

template <typename Geometry>
struct num_points<segment_tag, false, Geometry>
        : detail::num_points::other_count<Geometry, 2>
{
};


template <typename Geometry>
struct num_points<polygon_tag, false, Geometry>
        : detail::num_points::polygon_count<Geometry>
{
};

} // namespace dispatch
#endif


/*!
    \brief get number of points
    \ingroup access
    \tparam Geometry geometry type
    \param geometry the geometry to get number of points from
    \return number of points
    \note For linestrings/rings also boost::size or .size() could be used, however,
        for polygons this is less obvious. So this function is provided. Besides that
        it is described by OGC (numPoints)
*/
template <typename Geometry>
inline std::size_t num_points(Geometry const& geometry)
{
    concept::check<const Geometry>();

    typedef typename boost::remove_const<Geometry>::type ncg_type;

    return dispatch::num_points
        <
            typename tag<Geometry>::type,
            is_linear<ncg_type>::value,
            ncg_type
        >::apply(geometry);
}

}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_NUM_POINTS_HPP
