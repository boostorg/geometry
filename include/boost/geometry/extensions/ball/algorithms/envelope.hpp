// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2012 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2012 Mateusz Loskot, London, UK.

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_BALL_ALGORITHMS_ENVELOPE_HPP
#define BOOST_GEOMETRY_EXTENSIONS_BALL_ALGORITHMS_ENVELOPE_HPP


#include <boost/geometry/algorithms/envelope.hpp>

#include <boost/geometry/extensions/ball/core/tags.hpp>


namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace envelope
{

/// Calculate envelope of an n-sphere, circle or sphere (currently only for Cartesian 2D points)
struct envelope_ball
{
    template <typename Ball, typename Box, typename Strategy>
    static inline void apply(Ball const& nsphere, Box& mbr, Strategy const&)
    {
        apply(nsphere, mbr);
    }

    template <typename Ball, typename Box>
    static inline void apply(Ball const& nsphere, Box& mbr)
    {
        assert_dimension<Ball, 2>();
        assert_dimension<Box, 2>();

        typename radius_type<Ball>::type radius = get<radius_value, 0>(nsphere);
        set<min_corner, 0>(mbr, get<center_point, 0>(nsphere) - radius);
        set<min_corner, 1>(mbr, get<center_point, 1>(nsphere) - radius);
        set<max_corner, 0>(mbr, get<center_point, 0>(nsphere) + radius);
        set<max_corner, 1>(mbr, get<center_point, 1>(nsphere) + radius);
    }
};


}} // namespace detail::envelope
#endif // DOXYGEN_NO_DETAIL

#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


template <typename Ball>
struct envelope<Ball, ball_tag>
    : detail::envelope::envelope_ball
{};


} // namespace dispatch
#endif



}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_EXTENSIONS_BALL_ALGORITHMS_ENVELOPE_HPP
