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

namespace boost { namespace geometry
{

namespace strategy { namespace uniform_point_distribution {

//The following strategy is suitable for rings or polygons of three points.
template
<
    typename Point,
    typename DomainGeometry
>
struct uniform_2d_cartesian_triangle
{
private:
    typedef typename point_type<DomainGeometry>::type domain_point_type;
public:
    uniform_2d_cartesian_triangle(DomainGeometry const& d) {}
    bool equals(DomainGeometry const& l_domain,
                DomainGeometry const& r_domain,
                uniform_2d_cartesian_triangle const& r_strategy) const
    {
        return boost::geometry::equals(l_domain.domain(), r_domain.domain());
    }

    template<typename sample_type>
    static Point map(domain_point_type const& p1,
                     domain_point_type const& p2,
                     domain_point_type const& p3,
                     sample_type const& s1,
                     sample_type const& s2)
    {
        Point out;
        sample_type r1 = std::sqrt(s1);
        set<0>(out, (1 - r1) * get<0>(p1)
                  + ( r1 * (1 - s2) ) * get<0>(p2)
                  + ( s2 * r1 * get<0>(p3)));
        set<1>(out, (1 - r1) * get<1>(p1)
                  + ( r1 * (1 - s2) ) * get<1>(p2)
                  + ( s2 * r1 * get<1>(p3)));
        return out;
    }

    template<typename Generator>
    Point apply(Generator& g, DomainGeometry const& d)
    {
        typedef typename select_most_precise
            <
                typename coordinate_type<DomainGeometry>::type,
                double
            >::type sample_type;
        std::uniform_real_distribution<sample_type> real_dist(0, 1);
        sample_type s1 = real_dist(g),
                    s2 = real_dist(g);
        return map(*d.begin(), *(d.begin() + 1), *(d.begin() + 2), s1, s2);
    }
    void reset(DomainGeometry const&) {};
};

}} // namespace strategy::uniform_point_distribution

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_EXTENSIONS_RANDOM_STRATEGIES_CARTESIAN_UNIFORM_POINT_DISTRIBUTION_TRIANGLE_HPP
