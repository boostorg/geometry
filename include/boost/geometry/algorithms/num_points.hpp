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

#include <boost/mpl/assert.hpp>
#include <boost/range.hpp>
#include <boost/typeof/typeof.hpp>

#include <boost/geometry/core/closure.hpp>
#include <boost/geometry/core/is_linear.hpp>
#include <boost/geometry/core/exterior_ring.hpp>
#include <boost/geometry/core/interior_rings.hpp>
#include <boost/geometry/core/ring_type.hpp>

#include <boost/geometry/algorithms/disjoint.hpp>
#include <boost/geometry/geometries/concepts/check.hpp>


namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace num_points
{


template <typename Range>
struct range_count
{
    static inline std::size_t apply(Range const& range, bool add_for_open)
    {
        std::size_t n = boost::size(range);
        if (add_for_open && n > 0)
        {
            closure_selector const s = geometry::closure<Range>::value;

            if (s == open)
            {
                if (geometry::disjoint(*boost::begin(range), *(boost::begin(range) + n - 1)))
                {
                    return n + 1;
                }
            }
        }
        return n;
    }
};

template <typename Geometry, std::size_t D>
struct other_count
{
    static inline std::size_t apply(Geometry const&, bool)
    {
        return D;
    }
};

template <typename Polygon>
struct polygon_count
{
    static inline std::size_t apply(Polygon const& poly, bool add_for_open)
    {
        typedef typename geometry::ring_type<Polygon>::type ring_type;

        std::size_t n = range_count<ring_type>::apply(
                    exterior_ring(poly), add_for_open);

        typename interior_return_type<Polygon const>::type rings
                    = interior_rings(poly);
        for (BOOST_AUTO(it, boost::begin(rings)); it != boost::end(rings); ++it)
        {
            n += range_count<ring_type>::apply(*it, add_for_open);
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
    BOOST_MPL_ASSERT_MSG
        (
            false, NOT_OR_NOT_YET_IMPLEMENTED_FOR_THIS_GEOMETRY_TYPE
            , (types<Geometry>)
        );
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
\brief \brief_calc{number of points}
\ingroup num_points
\details \details_calc{num_points, number of points}.
\tparam Geometry \tparam_geometry
\param geometry \param_geometry
\param add_for_open add one for open geometries (i.e. polygon types which are not closed)
\return \return_calc{number of points}

\qbk{behavior,__point__:[qbk_ret 1]}
\qbk{behavior,__segment__:[qbk_ret 2]}
\qbk{behavior,__box__:[qbk_ret 4]}
\qbk{behavior,__range__:[qbk_ret boost::size(geometry)]}
\qbk{behavior,__other__:[qbk_ret the sum of the number of points of its elements]}
\qbk{complexity,Constant or Linear}
\qbk{compliance,__ogc__}
*/
template <typename Geometry>
inline std::size_t num_points(Geometry const& geometry, bool add_for_open = false)
{
    concept::check<Geometry const>();

    return dispatch::num_points
        <
            typename tag<Geometry>::type,
            is_linear<Geometry>::value,
            Geometry
        >::apply(geometry, add_for_open);
}

}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_NUM_POINTS_HPP
