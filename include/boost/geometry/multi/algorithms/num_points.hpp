// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef BOOST_GEOMETRY_MULTI_ALGORITHMS_NUM_POINTS_HPP
#define BOOST_GEOMETRY_MULTI_ALGORITHMS_NUM_POINTS_HPP


#include <boost/range.hpp>

#include <boost/geometry/multi/core/tags.hpp>
#include <boost/geometry/algorithms/num_points.hpp>


namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace num_points
{


template <typename MultiGeometry>
struct multi_count
{
    static inline size_t apply(MultiGeometry const& geometry, bool add_for_open)
    {
        typedef typename boost::range_value<MultiGeometry>::type geometry_type;
        typedef typename boost::range_iterator
            <
                MultiGeometry const
            >::type iterator_type;

        std::size_t n = 0;
        for (iterator_type it = boost::begin(geometry);
            it != boost::end(geometry);
            ++it)
        {
            n += dispatch::num_points
                <
                    typename tag<geometry_type>::type,
                    geometry_type
                >::apply(*it, add_for_open);
        }
        return n;
    }
};


}} // namespace detail::num_points
#endif


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


template <typename Geometry>
struct num_points<multi_tag, Geometry>
    : detail::num_points::multi_count<Geometry> {};


} // namespace dispatch
#endif


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_MULTI_ALGORITHMS_NUM_POINTS_HPP
