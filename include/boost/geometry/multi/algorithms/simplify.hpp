// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_MULTI_ALGORITHMS_SIMPLIFY_HPP
#define BOOST_GEOMETRY_MULTI_ALGORITHMS_SIMPLIFY_HPP

#include <boost/range.hpp>

#include <boost/geometry/multi/core/tags.hpp>
#include <boost/geometry/multi/core/is_multi.hpp>

#include <boost/geometry/multi/iterators/range_type.hpp>

#include <boost/geometry/multi/algorithms/clear.hpp>
#include <boost/geometry/algorithms/simplify.hpp>


namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace simplify
{

template<typename MultiGeometry, typename Strategy, typename Policy>
struct simplify_multi
{
    static inline void apply(MultiGeometry const& multi, MultiGeometry& out,
                    double max_distance, Strategy const& strategy)
    {
        out.resize(boost::size(multi));

        typename boost::range_iterator<MultiGeometry>::type it_out
                = boost::begin(out);
        for (typename boost::range_iterator<MultiGeometry const>::type it_in
                    = boost::begin(multi);
            it_in != boost::end(multi);
            ++it_in, ++it_out)
        {
            Policy::apply(*it_in, *it_out, max_distance, strategy);
        }
    }
};



}} // namespace detail::simplify
#endif // DOXYGEN_NO_DETAIL




#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template <typename MultiPoint, typename Strategy>
struct simplify<multi_point_tag, MultiPoint, Strategy>
    : detail::simplify::simplify_copy
        <
            MultiPoint,
            Strategy
        >

{};


template <typename MultiLinestring, typename Strategy>
struct simplify<multi_linestring_tag, MultiLinestring, Strategy>
    : detail::simplify::simplify_multi
        <
            MultiLinestring,
            Strategy,
            detail::simplify::simplify_range
                <
                    typename boost::range_value<MultiLinestring>::type,
                    Strategy,
                    2
                >
        >

{};


template <typename MultiPolygon, typename Strategy>
struct simplify<multi_polygon_tag, MultiPolygon, Strategy>
    : detail::simplify::simplify_multi
        <
            MultiPolygon,
            Strategy,
            detail::simplify::simplify_polygon
                <
                    typename boost::range_value<MultiPolygon>::type,
                    Strategy
                >
        >

{};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_MULTI_ALGORITHMS_SIMPLIFY_HPP
