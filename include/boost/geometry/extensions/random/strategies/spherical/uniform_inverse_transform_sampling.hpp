// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2019 Tinko Bartels, Berlin, Germany.

// Contributed and/or modified by Tinko Bartels,
//   as part of Google Summer of Code 2019 program.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_RANDOM_STRATEGIES_SPHERICAL_UNIFORM_INVERSE_TRANSFORM_SAMPLING_HPP
#define BOOST_GEOMETRY_EXTENSIONS_RANDOM_STRATEGIES_SPHERICAL_UNIFORM_INVERSE_TRANSFORM_SAMPLING_HPP

#include <random>
#include <random>

#include <boost/geometry/algorithms/equals.hpp>

#include <boost/geometry/extensions/random/strategies/uniform_point_distribution.hpp>

namespace boost { namespace geometry
{

namespace strategy { namespace uniform_point_distribution
{

template
<
    typename Point,
    typename DomainGeometry,
    int Dim
>
struct uniform_inverse_transform_sampling
{};

template
<
    typename Point,
    typename DomainGeometry
>
struct uniform_inverse_transform_sampling<Point, DomainGeometry, 2>
{
    uniform_inverse_transform_sampling(DomainGeometry const& d) {}
    bool equals(DomainGeometry const& l_domain,
                DomainGeometry const& r_domain,
                uniform_inverse_transform_sampling const& r_strategy) const
    {
        return boost::geometry::equals(l_domain.domain(), r_domain.domain());
    }
    template<typename Generator>
    Point apply(Generator& g, DomainGeometry const& d)
    {
        Point out;
        typedef typename coordinate_type<Point>::type coordinate_type;
        typedef typename select_most_precise
            <
                coordinate_type,
                double
            >::type computation_type;
        std::uniform_real_distribution<computation_type> lon_dist(
            get_as_radian<0, 0>(d),
            get_as_radian<1, 0>(d));
        set_from_radian<0>(out, lon_dist(g));

        coordinate_type lat1 = get_as_radian<0, 1>(d);
        coordinate_type lat2 = get_as_radian<1, 1>(d);
        coordinate_type x1   = (1.0 - std::cos(lat1)) / 2,
                        x2   = (1.0 - std::cos(lat2)) / 2;
        std::uniform_real_distribution<computation_type> x_dist(
            std::min(x1, x2),
            std::max(x1, x2));
        coordinate_type x = x_dist(g);
        set_from_radian<1>(out, std::acos(1.0 - 2.0 * x));
        return out;
    }
    void reset(DomainGeometry const&) {};
};

template
<
    typename Point,
    typename DomainGeometry
>
struct uniform_inverse_transform_sampling<Point, DomainGeometry, 3>
{
    uniform_inverse_transform_sampling(DomainGeometry const& d) {}
    bool equals(DomainGeometry const& l_domain,
                DomainGeometry const& r_domain,
                uniform_inverse_transform_sampling const& r_strategy) const
    {
        return boost::geometry::equals(l_domain.domain(), r_domain.domain());
    }
    template<typename Generator>
    Point apply(Generator& g, DomainGeometry const& d)
    {
        uniform_inverse_transform_sampling<Point, DomainGeometry, 2> helper(d);
        Point out = helper.apply(g, d);
        typedef typename coordinate_type<Point>::type coordinate_type;
        typedef typename select_most_precise
            <
                coordinate_type,
                double
            >::type computation_type;
        coordinate_type r1 = get<0, 2>(d);
        coordinate_type r2 = get<1, 2>(d);
        std::uniform_real_distribution<computation_type>
            r_dist( r1 * r1 * r1 , r2 * r2 * r2 );
        set<2>(out, std::cbrt(r_dist(g)));
        return out;
    }
    void reset(DomainGeometry const&) {};
};

namespace services {

template
<
    typename Point,
    typename DomainGeometry
>
struct default_strategy
<
    Point,
    DomainGeometry,
    box_tag,
    single_tag, //There are no MultiBoxes right now
    2,
    spherical_tag
> : public uniform_inverse_transform_sampling<Point, DomainGeometry, 2> {
    typedef uniform_inverse_transform_sampling<Point, DomainGeometry, 2> base;
    using base::base;
};

template
<
    typename Point,
    typename DomainGeometry
>
struct default_strategy
<
    Point,
    DomainGeometry,
    box_tag,
    single_tag, //There are no MultiBoxes right now
    3,
    spherical_tag
> : public uniform_inverse_transform_sampling<Point, DomainGeometry, 3> {
    typedef uniform_inverse_transform_sampling<Point, DomainGeometry, 3> base;
    using base::base;
};

} // namespace services

}} // namespace strategy::uniform_point_distribution

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_EXTENSIONS_RANDOM_STRATEGIES_SPHERICAL_UNIFORM_INVERSE_TRANSFORM_SAMPLING_HPP
