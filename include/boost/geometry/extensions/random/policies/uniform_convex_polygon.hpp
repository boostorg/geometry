// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2020 Tinko Bartels, Berlin, Germany.

// Contributed and/or modified by Tinko Bartels,
//   as part of Google Summer of Code 2019 program.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_RANDOM_POLICIES_UNIFORM_CONVEX_POLYGON_HPP
#define BOOST_GEOMETRY_EXTENSIONS_RANDOM_POLICIES_UNIFORM_CONVEX_POLYGON_HPP

#include <random>
#include <vector>
#include <cstdlib>
#include <cmath>

#include <boost/range/size.hpp>
#include <boost/geometry/algorithms/equals.hpp>
#include <boost/geometry/algorithms/transform.hpp>

#include <boost/geometry/strategies/cartesian/area.hpp>
#include <boost/geometry/strategies/geographic/area.hpp>
#include <boost/geometry/strategies/spherical/area.hpp>

#include <boost/geometry/extensions/random/strategies/cartesian/uniform_point_distribution_triangle.hpp>

namespace boost { namespace geometry
{

namespace policy { namespace uniform_point_distribution
{

// The following policy is suitable for convex rings and polygons with
// non-empty interior.
template
<
    typename Point,
    typename DomainGeometry,
    typename TriangleStrategy = void,
    typename AreaStrategy = void,
    typename CalculationType = void
>
struct convex_polygon
{
private:
    typedef typename boost::mpl::if_
                <
                    boost::is_void<CalculationType>,
                    typename select_most_precise
                        <
                            typename coordinate_type<Point>::type,
                            double
                        >::type,
                    CalculationType
                >::type ct;
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
                    boost::is_void<TriangleStrategy>,
                    typename boost::geometry::strategy::uniform_point_distribution::services
                        ::default_triangle_strategy
                        <
                            Point, typename point_type<DomainGeometry>::type
                        >::type,
                    TriangleStrategy    
                >::type triangle_strategy;
    typedef typename area_strategy::template state<DomainGeometry> area_state;
    typedef typename area_strategy::template result_type<DomainGeometry> area_result;
    std::vector<ct> m_accumulated_areas;
    triangle_strategy m_triangle;
    // It is hard to see a reason not to use double here. If a triangle's
    // relative size is smaller than doubles epsilon, it is too unlikely to
    // realistically occur in a random sample.
public:
    convex_polygon() {}
    convex_polygon(DomainGeometry const& d,
                   triangle_strategy const& t = triangle_strategy(),
                   area_strategy const& a = area_strategy()) :
        m_triangle(t)
    {
	m_accumulated_areas.reserve(boost::size(d) - 1);
        m_accumulated_areas.push_back(0);
        for (std::size_t i = 2 ; i < boost::size(d) ; ++i) {
            area_state state;
            a.apply(*d.begin(), *(d.begin() + i - 1), state);
	    a.apply(*(d.begin() + i - 1), *(d.begin() + i), state);
	    a.apply(*(d.begin() + i), *d.begin(), state);
	    ct s;
            if(boost::geometry::point_order<DomainGeometry>::value == boost::geometry::clockwise)
	    {
            	s = area_strategy::result(state);
	    } else {
		s = -area_strategy::result(state);
	    }
            m_accumulated_areas.push_back(m_accumulated_areas.back() + s);
        }
    }
    bool equals(DomainGeometry const& l_domain,
                DomainGeometry const& r_domain,
                convex_polygon const& r_strategy) const
    {
        if( boost::size(l_domain) != boost::size(r_domain) )
        {
            return false;
        }
        for (std::size_t i = 0; i < boost::size(l_domain); ++i)
        {
            if( !boost::geometry::equals(*(l_domain.begin() + i),
                                         *(r_domain.begin() + i)))
            {
                return false;
            }
        }
        return true;
    }
    template<typename Generator>
    Point apply(Generator& g, DomainGeometry const& d)
    {
        std::uniform_real_distribution<ct> dist(0, 1);
        ct r = dist(g) * m_accumulated_areas.back();
        ct s = dist(g);
        std::size_t i = std::distance(
            m_accumulated_areas.begin(),
            std::lower_bound(m_accumulated_areas.begin(),
                             m_accumulated_areas.end(),
                             r));
        return m_triangle.template apply
            <
                ct
            >(* d.begin(),
              *(d.begin() + i),
              *(d.begin() + i + 1),
              ( r - m_accumulated_areas[ i - 1 ]) /
                ( m_accumulated_areas[ i ] - m_accumulated_areas[ i - 1 ] ),
              s);
    }
    void reset(DomainGeometry const&) {};
};

}} // namespace policy::uniform_point_distribution

}} // namespace boost::geometry

#endif //  BOOST_GEOMETRY_EXTENSIONS_RANDOM_POLICIES_UNIFORM_CONVEX_POLYGON_HPP
