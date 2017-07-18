// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2015 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2015, 2017.
// Modifications copyright (c) 2015-2017 Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_DIRECITON_CODE_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_DIRECITON_CODE_HPP


#include <boost/geometry/core/access.hpp>
#include <boost/geometry/util/math.hpp>
#include <boost/geometry/util/select_coordinate_type.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail
{


// TODO: remove
template <std::size_t Index, typename Point1, typename Point2>
inline int sign_of_difference(Point1 const& point1, Point2 const& point2)
{
    return
        math::equals(geometry::get<Index>(point1), geometry::get<Index>(point2))
        ?
        0
        :
        (geometry::get<Index>(point1) > geometry::get<Index>(point2) ? 1 : -1);
}


template <typename Point, typename CSTag = typename cs_tag<Point>::type>
struct direction_code_impl
{
    template <typename Point1, typename Point2>
    static inline int apply(Point1 const& segment_a, Point1 const& segment_b,
                            Point2 const& p)
    {
        typedef typename geometry::select_coordinate_type
            <
                Point1, Point2
            >::type calc_t;

        if ( (math::equals(geometry::get<0>(segment_b), geometry::get<0>(segment_a))
           && math::equals(geometry::get<1>(segment_b), geometry::get<1>(segment_a)))
          || (math::equals(geometry::get<0>(segment_b), geometry::get<0>(p))
           && math::equals(geometry::get<1>(segment_b), geometry::get<1>(p))) )
        {
            return 0;
        }

        calc_t x1 = geometry::get<0>(segment_b) - geometry::get<0>(segment_a);
        calc_t y1 = geometry::get<1>(segment_b) - geometry::get<1>(segment_a);
        calc_t x2 = geometry::get<0>(segment_b) - geometry::get<0>(p);
        calc_t y2 = geometry::get<1>(segment_b) - geometry::get<1>(p);

        calc_t ax = (std::min)(math::abs(x1), math::abs(x2));
        calc_t ay = (std::min)(math::abs(y1), math::abs(y2));

        int s1 = 0, s2 = 0;
        if (ax >= ay)
        {
            s1 = x1 > 0 ? 1 : -1;
            s2 = x2 > 0 ? 1 : -1;
        }
        else
        {
            s1 = y1 > 0 ? 1 : -1;
            s2 = y2 > 0 ? 1 : -1;
        }

        return s1 == s2 ? -1 : 1;
    }
};

// Gives sense of direction for point p, collinear w.r.t. segment (a,b)
// Returns -1 if p goes backward w.r.t (a,b), so goes from b in direction of a
// Returns 1 if p goes forward, so extends (a,b)
// Returns 0 if p is equal with b, or if (a,b) is degenerate
// Note that it does not do any collinearity test, that should be done before
template <typename Point1, typename Point2>
inline int direction_code(Point1 const& segment_a, Point1 const& segment_b,
                          Point2 const& p)
{
    return direction_code_impl<Point1>::apply(segment_a, segment_b, p);
}


} // namespace detail
#endif //DOXYGEN_NO_DETAIL


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_DIRECITON_CODE_HPP
