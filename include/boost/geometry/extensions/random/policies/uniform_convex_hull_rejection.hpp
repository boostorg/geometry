// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2020 Tinko Bartels, Berlin, Germany.

// Contributed and/or modified by Tinko Bartels,
//   as part of Google Summer of Code 2019 program.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_RANDOM_POLICIES_UNIFORM_CONVEX_HULL_REJECTION_HPP
#define BOOST_GEOMETRY_EXTENSIONS_RANDOM_POLICIES_UNIFORM_CONVEX_HULL_REJECTION_HPP

#include <random>
#include <vector>

#include <boost/geometry/algorithms/equals.hpp>
#include <boost/geometry/algorithms/transform.hpp>
#include <boost/geometry/algorithms/within.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/algorithms/convex_hull.hpp>

#include <boost/geometry/strategies/cartesian/area.hpp>
#include <boost/geometry/strategies/geographic/area.hpp>
#include <boost/geometry/strategies/spherical/area.hpp>

#include <boost/geometry/strategies/cartesian/point_in_poly_winding.hpp>
#include <boost/geometry/strategies/spherical/point_in_poly_winding.hpp>
#include <boost/geometry/strategies/geographic/point_in_poly_winding.hpp>

#include <boost/geometry/extensions/random/strategies/cartesian/uniform_point_distribution_triangle.hpp>
#include <boost/geometry/extensions/random/policies/uniform_convex_polygon.hpp>

namespace boost { namespace geometry
{

namespace policy { namespace uniform_point_distribution
{

// The following strategy is suitable for geometries for which
// a Triangle sampling strategy can be provided
// a SideStrategy that computes the triangle area can be provided.
template
<
    typename Point,
    typename DomainGeometry,
    typename TriangleStrategy = void,
    typename AreaStrategy = void,
//  typename ConvexHullStrategy
    typename WithinStrategy = void
>
struct convex_hull_rejection
{
private:
    typedef typename boost::mpl::if_
                <
                    boost::is_void<TriangleStrategy>,
                    typename boost::geometry::strategy::uniform_point_distribution::services
                        ::default_triangle_strategy
                        <
                            Point, typename point_type<DomainGeometry>::type
                        >::type,
                    TriangleStrategy   
                >::type triangle_strategy;
    typedef typename boost::mpl::if_
                <
                    boost::is_void<AreaStrategy>,
                    typename boost::geometry::strategy::area::services::default_strategy
                        <
                            typename cs_tag<DomainGeometry>::type
                        >::type,
                    AreaStrategy
                >::type area_strategy;
    typedef typename boost::mpl::if_
                <
                    boost::is_void<WithinStrategy>,
                    typename boost::geometry::strategy::within::services::default_strategy
                        <
                            Point,
                            DomainGeometry
                        >::type,
                    WithinStrategy
                >::type within_strategy;
    typedef typename point_type<DomainGeometry>::type domain_point_type;
    typedef boost::geometry::model::ring<domain_point_type> ring;
    ring m_hull;
    convex_polygon<Point, ring, triangle_strategy, area_strategy> m_policy;
    within_strategy m_within_strategy;
public:
    convex_hull_rejection(DomainGeometry const& d,
                          triangle_strategy const& t = triangle_strategy(),
                          area_strategy const& a = area_strategy(),
                          within_strategy const& w = within_strategy()) : m_within_strategy(w)
	
    {
        boost::geometry::convex_hull(d, m_hull);
        m_policy = convex_polygon
                <
                    Point,
                    ring,
                    triangle_strategy,
                    area_strategy
                >(m_hull, t, a);
    }
    bool equals(DomainGeometry const& l_domain,
                DomainGeometry const& r_domain,
                convex_hull_rejection const& r_policy) const
    {
        return boost::geometry::equals(l_domain, r_domain)
            && m_policy.equals(l_domain, r_domain, r_policy.m_policy);
    }
    template<typename Generator>
    Point apply(Generator& g, DomainGeometry const& d)
    {
        Point p;
        do {
            p = m_policy.apply(g, m_hull);
        } while( !boost::geometry::within(p, d, m_within_strategy) );
        return p;
    }
    void reset(DomainGeometry const&) {};
};

}} // namespace policy::uniform_point_distribution

}} // namespace boost::geometry

#endif //  BOOST_GEOMETRY_EXTENSIONS_RANDOM_STRATEGIES_AGNOSTIC_UNIFORM_CONVEX_HULL_REJECTION_HPP
