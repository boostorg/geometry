// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2019 Tinko Bartels, Berlin, Germany.

// Contributed and/or modified by Tinko Bartels,
//   as part of Google Summer of Code 2019 program.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_RANDOM_STRATEGIES_AGNOSTIC_UNIFORM_CONVEX_HULL_REJECTION_HPP
#define BOOST_GEOMETRY_EXTENSIONS_RANDOM_STRATEGIES_AGNOSTIC_UNIFORM_CONVEX_HULL_REJECTION_HPP

#include <random>
#include <vector>

#include <boost/geometry/algorithms/equals.hpp>
#include <boost/geometry/algorithms/transform.hpp>
#include <boost/geometry/algorithms/within.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/algorithms/convex_hull.hpp>

#include <boost/geometry/extensions/random/strategies/agnostic/uniform_convex_polygon_sampler.hpp>

namespace boost { namespace geometry
{

namespace strategy { namespace uniform_point_distribution {

// The following strategy is suitable for geometries for which
// a Triangle sampling strategy can be provided
// a SideStrategy that compues the triangle area can be provided.
template
<
    typename Point,
    typename DomainGeometry,
    typename TriangleStrategy,
    typename SideStrategy   //Actually, we need a triangle area strategy here.
>
struct uniform_convex_hull_rejection
{
private:
    typedef typename point_type<DomainGeometry>::type domain_point_type;
    typedef boost::geometry::model::ring<domain_point_type> ring;
    ring m_hull;
    uniform_convex_polygon_sampler<Point, ring, TriangleStrategy, SideStrategy>
        m_strategy;
public:
    uniform_convex_hull_rejection(DomainGeometry const& d) : m_strategy(m_hull)
    {
        boost::geometry::convex_hull(d, m_hull);
        m_strategy =
            uniform_convex_polygon_sampler
                <
                    Point,
                    ring,
                    TriangleStrategy,
                    SideStrategy
                >(m_hull);
    }
    bool equals(DomainGeometry const& l_domain,
                DomainGeometry const& r_domain,
                uniform_convex_hull_rejection const& r_strategy) const
    {
        return boost::geometry::equals(l_domain, r_domain)
            && m_strategy.equals(l_domain, r_domain, r_strategy.m_strategy);
    }
    template<typename Generator>
    Point apply(Generator& g, DomainGeometry const& d)
    {
        Point p;
        do{
            p = m_strategy.apply(g, m_hull);
        }while( !boost::geometry::within(p, d) );
        return p;
    }
    void reset(DomainGeometry const&) {};
};

}} // namespace strategy::uniform_point_distribution

}} // namespace boost::geometry

#endif //  BOOST_GEOMETRY_EXTENSIONS_RANDOM_STRATEGIES_AGNOSTIC_UNIFORM_CONVEX_HULL_REJECTION_HPP
