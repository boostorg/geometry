// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.SpatialIndex - n-dimensional distance_sqr between points
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_ALGORITHMS_DISTANCE_SQR_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_ALGORITHMS_DISTANCE_SQR_HPP

namespace boost { namespace geometry { namespace index {

template <typename Point1, typename Point2>
class default_distance_sqr_result
{
    typedef typename select_most_precise<
        typename traits::coordinate_type<Point1>::type,
        long double
    >::type intermediate_type;

public:
    typedef typename select_most_precise<
        typename traits::coordinate_type<Point2>::type,
        intermediate_type
    >::type type;
};

namespace detail {

template <typename Point1, typename Point2, size_t CurrentDimension>
struct distance_sqr_for_each_dimension
{
    BOOST_STATIC_ASSERT(0 < CurrentDimension);
    BOOST_STATIC_ASSERT(CurrentDimension <= traits::dimension<Point1>::value);

    typedef typename default_distance_sqr_result<Point1, Point2>::type result_type;

    static inline result_type apply(Point1 const& p1, Point2 const& p2)
    {
        result_type temp = geometry::get<CurrentDimension - 1>(p1) - geometry::get<CurrentDimension - 1>(p2);
        return distance_sqr_for_each_dimension<Point1, Point2, CurrentDimension - 1>::apply(p1, p2) +
               temp * temp;
    }
};

template <typename Point1, typename Point2>
struct distance_sqr_for_each_dimension<Point1, Point2, 1>
{
    BOOST_STATIC_ASSERT(1 <= traits::dimension<Point1>::value);

    typedef typename default_distance_sqr_result<Point1, Point2>::type result_type;

    static inline result_type apply(Point1 const& p1, Point2 const& p2)
    {
        result_type temp = geometry::get<0>(p1) - geometry::get<0>(p2);
        return temp * temp;
    }
};

} // namespace detail

template <typename Point1, typename Point2>
typename default_distance_sqr_result<Point1, Point2>::type distance_sqr(Point1 const& p1, Point2 const& p2)
{
    BOOST_STATIC_ASSERT(traits::dimension<Point1>::value == traits::dimension<Point2>::value);

    return detail::distance_sqr_for_each_dimension<
               Point1,
               Point2,
               index::traits::dimension<Point1>::value
           >::apply(p1, p2);
}

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_ALGORITHMS_DISTANCE_SQR_HPP
