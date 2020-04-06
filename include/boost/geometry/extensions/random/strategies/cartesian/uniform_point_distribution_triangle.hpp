// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2019 Tinko Bartels, Berlin, Germany.

// Contributed and/or modified by Tinko Bartels,
//   as part of Google Summer of Code 2019 program.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_RANDOM_STRATEGIES_CARTESIAN_UNIFORM_POINT_DISTRIBUTION_TRIANGLE_HPP
#define BOOST_GEOMETRY_EXTENSIONS_RANDOM_STRATEGIES_CARTESIAN_UNIFORM_POINT_DISTRIBUTION_TRIANGLE_HPP

#include <random>
#include <cmath>

#include <boost/geometry/arithmetic/arithmetic.hpp>
#include <boost/geometry/algorithms/assign.hpp>
#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/point_type.hpp>

#include <boost/geometry/extensions/random/strategies/uniform_point_distribution_triangle.hpp>

namespace boost { namespace geometry
{

namespace strategy { namespace uniform_point_distribution
{

template
<
    typename Point,
    typename DomainPoint
>
struct cartesian_triangle
{
private:
public:
    template<typename CalculationType>
    static Point apply(DomainPoint const& p1,
                       DomainPoint const& p2,
                       DomainPoint const& p3,
                       CalculationType const& s1,
                       CalculationType const& s2)
    {
        Point out;
        CalculationType r1 = std::sqrt(s1);
        set<0>(out, (1 - r1) * get<0>(p1)
                  + ( r1 * (1 - s2) ) * get<0>(p2)
                  + ( s2 * r1 * get<0>(p3)));
        set<1>(out, (1 - r1) * get<1>(p1)
                  + ( r1 * (1 - s2) ) * get<1>(p2)
                  + ( s2 * r1 * get<1>(p3)));
        return out;
    }
};

namespace services
{

template
<
    typename Point,
    typename DomainPoint
>
struct default_triangle_strategy
<
    Point,
    DomainPoint,
    cartesian_tag
>
{
    typedef cartesian_triangle<Point, DomainPoint> type;
};

} // namespace services

}} // namespace strategy::uniform_point_distribution

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_EXTENSIONS_RANDOM_STRATEGIES_CARTESIAN_UNIFORM_POINT_DISTRIBUTION_TRIANGLE_HPP
