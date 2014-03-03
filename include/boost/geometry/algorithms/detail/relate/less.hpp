// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2014.
// Modifications copyright (c) 2014, Oracle and/or its affiliates.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_LESS_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_LESS_HPP

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace relate {

// TODO: should this be integrated with geometry::less?

template <typename Point1,
          typename Point2,
          std::size_t I = 0,
          std::size_t D = geometry::dimension<Point1>::value>
struct less_dispatch
{
    static inline bool apply(Point1 const& l, Point2 const& r)
    {
        typename geometry::coordinate_type<Point1>::type
            cl = geometry::get<I>(l);
        typename geometry::coordinate_type<Point2>::type
            cr = geometry::get<I>(r);

        if ( geometry::math::equals(cl, cr) )
        {
            return less_dispatch<Point1, Point2, I + 1, D>::apply(l, r);
        }
        else
        {
            return cl < cr;
        }
    }
};

template <typename Point1, typename Point2, std::size_t D>
struct less_dispatch<Point1, Point2, D, D>
{
    static inline bool apply(Point1 const&, Point2 const&)
    {
        return false;
    }
};

struct less
{
    template <typename Point1, typename Point2>
    inline bool operator()(Point1 const& point1, Point2 const& point2)
    {
        return less_dispatch<Point1, Point2>::apply(point1, point2);
    }
};

}} // namespace detail::relate
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_LESS_HPP
