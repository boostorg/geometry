// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_MULTI_ALGORITHMS_WITHIN_HPP
#define BOOST_GEOMETRY_MULTI_ALGORITHMS_WITHIN_HPP


#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>

#include <boost/geometry/algorithms/within.hpp>
#include <boost/geometry/multi/core/tags.hpp>

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace within {


template
<
    typename Geometry,
    typename MultiGeometry,
    typename Strategy,
    typename Policy
>
struct geometry_in_multi
{
    static inline bool apply(Geometry const& geometry,
            MultiGeometry const& multi,
            Strategy const& strategy)
    {
        for (typename boost::range_const_iterator<MultiGeometry>::type it
                    = boost::begin(multi);
            it != boost::end(multi);
            ++it)
        {
            // Geometry within a multi: true if within one of them
            if (Policy::apply(geometry, *it, strategy))
            {
                return true;
            }
        }
        return false;
    }
};

}} // namespace detail::within
#endif // DOXYGEN_NO_DETAIL

#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template <typename Point, typename MultiPolygon, typename Strategy>
struct within<point_tag, multi_polygon_tag, Point, MultiPolygon, Strategy>
    : detail::within::geometry_in_multi
        <
            Point,
            MultiPolygon,
            Strategy,
            detail::within::point_in_polygon
                    <
                        Point,
                        typename boost::range_value<MultiPolygon>::type,
                        Strategy
                    >
        >
{};

} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_MULTI_ALGORITHMS_WITHIN_HPP
