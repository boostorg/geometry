// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2019 Tinko Bartels, Berlin, Germany.

// Contributed and/or modified by Tinko Bartels,
//   as part of Google Summer of Code 2019 program.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_RANDOM_STRATEGIES_SPHERICAL_UNIFORM_POINT_DISTRIBUTION_BOX_HPP
#define BOOST_GEOMETRY_EXTENSIONS_RANDOM_STRATEGIES_SPHERICAL_UNIFORM_POINT_DISTRIBUTION_BOX_HPP

#include <random>

#include <boost/mpl/assert.hpp>
#include <boost/mpl/int.hpp>

#include <boost/geometry/algorithms/equals.hpp>

#include <boost/geometry/extensions/random/strategies/uniform_point_distribution_box.hpp>

namespace boost { namespace geometry
{

namespace strategy { namespace uniform_point_distribution
{

template
<
    typename Point,
    typename DomainGeometry,
    std::size_t Dim
>
struct spherical_box
{
    BOOST_MPL_ASSERT_MSG
        (
            false, NOT_IMPLEMENTED_FOR_THIS_TYPE
            , (types<boost::mpl::int_<Dim>>)
        );
};

template
<
    typename Point,
    typename DomainGeometry
>
struct spherical_box<Point, DomainGeometry, 2>
{
    template<typename CalculationType>
    static Point apply(DomainGeometry const& d, std::array<CalculationType, 2> const& f)
    {
        Point out;
        set_from_radian<0>(out, get_as_radian<0, 0>(d) + 
                                    f[0] * ( get_as_radian<1, 0>(d) - get_as_radian<0, 0>(d) ));
        CalculationType lat1 = get_as_radian<0, 1>(d);
        CalculationType lat2 = get_as_radian<1, 1>(d);
        CalculationType x1   = (1.0 - std::cos(lat1)) / 2;
        CalculationType x2   = (1.0 - std::cos(lat2)) / 2;
	CalculationType lo   = std::min(x1, x2);
	CalculationType hi   = std::max(x1, x2);
        CalculationType x    = lo + f[1] * (hi - lo);
        set_from_radian<1>(out, std::acos(1.0 - 2.0 * x));
        return out;
    }
};

template
<
    typename Point,
    typename DomainGeometry
>
struct spherical_box<Point, DomainGeometry, 3>
{
    template<typename CalculationType>
    static Point apply(DomainGeometry const& d, std::array<CalculationType, 3> const& f)
    {
        Point out = spherical_box<Point, DomainGeometry, 2>::apply(d, {{f[0], f[1]}});
        CalculationType r1 = get<0, 2>(d);
        CalculationType r2 = get<1, 2>(d);
	CalculationType lo = r1 * r1 * r1;
	CalculationType hi = r2 * r2 * r2;
        set<2>(out, std::cbrt( lo + f[2] * ( hi - lo ) ));
        return out;
    }
};

namespace services {

template
<
    typename Point,
    typename DomainGeometry
>
struct default_box_strategy
<
    Point,
    DomainGeometry,
    2,
    spherical_tag
>
{
    typedef spherical_box<Point, DomainGeometry, 2> type;
};

template
<
    typename Point,
    typename DomainGeometry
>
struct default_box_strategy
<
    Point,
    DomainGeometry,
    3,
    spherical_tag
>
{
    typedef spherical_box<Point, DomainGeometry, 3> type;
};

} // namespace services

}} // namespace strategy::uniform_point_distribution

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_EXTENSIONS_RANDOM_STRATEGIES_SPHERICAL_UNIFORM_POINT_DISTRIBUTION_BOX_HPP
