// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2009-2011 Mateusz Loskot, London, UK.
// Copyright (c) 2009-2011 Barend Gehrels, Amsterdam, the Netherlands.

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_CENTROID_WEIGHTED_LENGTH_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_CENTROID_WEIGHTED_LENGTH_HPP

#include <cstddef>

#include <boost/array.hpp>
#include <boost/geometry/algorithms/distance.hpp>
#include <boost/geometry/util/select_most_precise.hpp>
#include <boost/geometry/strategies/centroid.hpp>
#include <boost/geometry/strategies/default_distance_result.hpp>


namespace boost { namespace geometry
{

// Note: when calling the namespace "centroid", it sometimes,
// somehow, in gcc, gives compilation problems (confusion with function centroid).

namespace strategy { namespace centroid
{

namespace detail
{

template <typename Type, std::size_t DimensionCount>
struct weighted_length_sums
{
    Type length;
    boost::array<Type, DimensionCount> average_sum;

    inline weighted_length_sums()
        : length(Type())
    {
        for (std::size_t i = 0; i < DimensionCount; i++)
        {
            average_sum[i] = Type();
        }
    }
};
}

template
<
    typename Point,
    typename PointOfSegment = Point
>
class weighted_length
{
private :
    typedef typename select_most_precise
        <
            typename default_distance_result<Point>::type,
            typename default_distance_result<PointOfSegment>::type
        >::type distance_type;

public :
    typedef detail::weighted_length_sums<distance_type, 2> state_type;

    static inline void apply(PointOfSegment const& p1,
            PointOfSegment const& p2, state_type& state)
    {
        distance_type const d = geometry::distance(p1, p2);
        state.length += d;

        distance_type two(2);

        // Might be made generic for N dimensions using specializations
        distance_type const mx = (get<0>(p1) + get<0>(p2)) / two;
        distance_type const my = (get<1>(p1) + get<1>(p2)) / two;
        state.average_sum[0] += d * mx;
        state.average_sum[1] += d * my;
    }

    static inline bool result(state_type const& state, Point& centroid)
    {
        distance_type const zero = distance_type();
        if (! geometry::math::equals(state.length, zero))
        {
            set<0>(centroid, state.average_sum[0] / state.length);
            set<1>(centroid, state.average_sum[1] / state.length);
            return true;
        }

        return false;
    }

};

#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS

namespace services
{

// Register this strategy for linestrings and polygons, in two or three dimensions
template <typename Point, typename Geometry>
struct default_strategy
<
    cartesian_tag,
    linear_tag,
    2,
    Point,
    Geometry
>
{
    typedef weighted_length
        <
            Point,
            typename point_type<Geometry>::type
        > type;
};



} // namespace services


#endif // DOXYGEN_NO_STRATEGY_SPECIALIZATIONS


}} // namespace strategy::centroid


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_STRATEGIES_CARTESIAN_CENTROID_WEIGHTED_LENGTH_HPP
