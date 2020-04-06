// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2020 Tinko Bartels, Berlin, Germany.

// Contributed and/or modified by Tinko Bartels,
//   as part of Google Summer of Code 2019 program.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_RANDOM_POLICIES_UNIFORM_BOX_HPP
#define BOOST_GEOMETRY_EXTENSIONS_RANDOM_POLICIES_UNIFORM_BOX_HPP

#include <random>
#include <array>
#include <functional>
#include <algorithm>

#include <boost/geometry/algorithms/equals.hpp>
#include <boost/geometry/algorithms/transform.hpp>
#include <boost/geometry/algorithms/within.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/core/point_type.hpp>

#include <boost/geometry/extensions/random/policies/uniform_default_policy.hpp>

#include <boost/geometry/extensions/random/strategies/cartesian/uniform_point_distribution_box.hpp>

namespace boost { namespace geometry
{

namespace policy { namespace uniform_point_distribution
{

template
<
    typename Point,
    typename DomainGeometry,
    typename BoxStrategy = void,
    typename CalculationType = void
>
struct box
{
private:
    typedef typename boost::mpl::if_
                <
                    boost::is_void<BoxStrategy>,
                    typename boost::geometry::strategy::uniform_point_distribution::services::
                        default_box_strategy
                            <
                                Point,                       
                                DomainGeometry           
                            >::type,           
                    BoxStrategy
                >::type box_strategy;
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
    box_strategy m_box_strategy;
public:
    box(DomainGeometry const& d, box_strategy const& b = box_strategy()) : m_box_strategy(b) {}
    bool equals(DomainGeometry const& l_domain,
                DomainGeometry const& r_domain,
                box const& r_policy) const
    {
        return boost::geometry::equals(l_domain, r_domain);
    }
    template<typename Generator>
    Point apply(Generator& g, DomainGeometry const& d)
    {
        typedef typename point_type<DomainGeometry>::type domain_point;
        domain_point p;
        std::array<ct, dimension<DomainGeometry>::value> fractions;
        std::uniform_real_distribution<ct> dist(0, 1);
        std::generate(fractions.begin(), fractions.end(), std::bind(dist, std::ref(g)));
        p = m_box_strategy.template apply<>(d, fractions);
        Point r;
        boost::geometry::transform(p, r);
        return r;
    }
    void reset(DomainGeometry const&) {};
};

namespace services
{

template
<
    typename Point,
    typename DomainGeometry,
    int Dim,
    typename CsTag
>
struct default_policy
<
    Point,
    DomainGeometry,
    box_tag,
    single_tag,
    Dim,
    CsTag
>
{
    typedef box<Point, DomainGeometry> type;
};

} // namespace services

}} // namespace policy::uniform_point_distribution

}} // namespace boost::geometry

#endif //  BOOST_GEOMETRY_EXTENSIONS_RANDOM_POLICIES_UNIFORM_BOX_HPP
