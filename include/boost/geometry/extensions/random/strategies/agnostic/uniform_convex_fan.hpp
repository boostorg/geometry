// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2019 Tinko Bartels, Berlin, Germany.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_RANDOM_STRATEGIES_AGNOSTIC_UNIFORM_CONVEX_FAN_HPP
#define BOOST_GEOMETRY_EXTENSIONS_RANDOM_STRATEGIES_AGNOSTIC_UNIFORM_CONVEX_FAN_HPP

#include <random>
#include <vector>
#include <cstdlib>
#include <cmath>

#include <boost/geometry/algorithms/equals.hpp>
#include <boost/geometry/algorithms/transform.hpp>
#include <boost/geometry/algorithms/within.hpp>

namespace boost { namespace geometry
{

namespace strategy { namespace uniform_point_distribution {

// The following strategy is suitable for convex rings and polygons with
// non-empty interior.
template
<
    typename Point,
    typename DomainGeometry,
    typename TriangleStrategy,
    typename SideStrategy   //Actually, we need a triangle area strategy here.
>
struct uniform_convex_fan
{
private:
    std::vector<double> accumulated_areas;
    // It is hard to see a reason not to use double here. If a triangles
    // relative size is smaller than doubles epsilon, it is too unlikely to
    // realistically occur in a random sample anyway.
public:
    uniform_convex_fan(DomainGeometry const& g)
	{
        accumulated_areas.push_back(0);
        for (int i = 2 ; i < g.size() ; ++i) {
            accumulated_areas.push_back(
                accumulated_areas.back() +
                std::abs(SideStrategy::template side_value<double, double>(
                         *g.begin(),
                         *(g.begin() + i - 1),
                         *(g.begin() + i))));
        }
    }
    bool equals(DomainGeometry const& l_domain,
                DomainGeometry const& r_domain,
                uniform_convex_fan const& r_strategy) const
    {
        if( l_domain.size() != r_domain.size() ) return false;
        for (int i = 0; i < l_domain.size(); ++i) {
            if( !boost::geometry::equals(*(l_domain.begin() + i),
                                         *(r_domain.begin() + i)))
                return false;
        }
        return true;
    }
    template<typename Gen>
    Point apply(Gen& g, DomainGeometry const& d)
    {
        std::uniform_real_distribution<double> dist(0, 1);
        double r = dist(g) * accumulated_areas.back(),
               s = dist(g);
        std::size_t i = std::distance(
            accumulated_areas.begin(),
            std::lower_bound(accumulated_areas.begin(),
                             accumulated_areas.end(),
                             r));
        return TriangleStrategy::template map
            <
                double
            >(* d.begin(),
              *(d.begin() + i),
              *(d.begin() + i + 1),
              ( r - accumulated_areas[ i - 1 ]) /
                ( accumulated_areas[ i ] - accumulated_areas[ i - 1 ] ),
              s);
    }
    void reset(DomainGeometry const&) {};
};

}} // namespace strategy::uniform_point_distribution

}} // namespace boost::geometry

#endif //  BOOST_GEOMETRY_EXTENSIONS_RANDOM_STRATEGIES_AGNOSTIC_UNIFORM_CONVEX_FAN_HPP
