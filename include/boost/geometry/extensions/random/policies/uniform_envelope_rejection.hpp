// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2020 Tinko Bartels, Berlin, Germany.

// Contributed and/or modified by Tinko Bartels,
//   as part of Google Summer of Code 2019 program.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_RANDOM_POLICIES_UNIFORM_ENVELOPE_REJECTION_HPP
#define BOOST_GEOMETRY_EXTENSIONS_RANDOM_POLICIES_UNIFORM_ENVELOPE_REJECTION_HPP

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
#include <boost/geometry/extensions/random/strategies/spherical/uniform_point_distribution_box.hpp>

namespace boost { namespace geometry
{

namespace policy { namespace uniform_point_distribution
{

template
<
    typename Point,
    typename DomainGeometry,
    typename BoxStrategy = void,
    typename EnvelopeStrategy = void,
    typename WithinStrategy = void,
    typename CalculationType = void
>
struct envelope_rejection
{
private:
    typedef model::box<typename point_type<DomainGeometry>::type> envelope_type;
    typedef typename boost::mpl::if_
                <
                    boost::is_void<BoxStrategy>,
                    typename boost::geometry::strategy::uniform_point_distribution::services::
                        default_box_strategy
                            <
                                Point,
                                envelope_type
                            >::type,
                    BoxStrategy
                >::type box_strategy;
    typedef typename boost::mpl::if_
                <
                    boost::is_void<EnvelopeStrategy>,
                    typename boost::geometry::strategy::envelope::services::default_strategy
                        <
                            typename tag<DomainGeometry>::type,
                            typename cs_tag<DomainGeometry>::type
                         >::type,
                    EnvelopeStrategy
                >::type envelope_strategy;
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
    within_strategy m_within_strategy;
    envelope_type m_env;
public:
    envelope_rejection(DomainGeometry const& d,
                       box_strategy const& b = box_strategy(),
		       envelope_strategy const& e = envelope_strategy(),
                       within_strategy const& w = within_strategy()) :
        m_box_strategy(b),
        m_within_strategy(w),
        m_env(return_envelope<envelope_type, DomainGeometry>(d, e)) {}
    bool equals(DomainGeometry const& l_domain,
                DomainGeometry const& r_domain,
                envelope_rejection const&) const
    {
        return boost::geometry::equals(l_domain, r_domain);
    }
    template<typename Generator>
    Point apply(Generator& g, DomainGeometry const& d)
    {
        typedef typename point_type<DomainGeometry>::type domain_point;
        domain_point p;
        do {
            std::array<ct, dimension<DomainGeometry>::value> fractions;
            std::uniform_real_distribution<ct> dist(0, 1);
            std::generate(fractions.begin(), fractions.end(), std::bind(dist, std::ref(g)));
            p = m_box_strategy.template apply<ct>(m_env, fractions);
        } while( ! ::boost::geometry::within(p, d, m_within_strategy) );
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
    typename MultiOrSingle,
    typename CsTag
>
struct default_policy
<
    Point,
    DomainGeometry,
    areal_tag,
    MultiOrSingle,
    2,
    CsTag
>
{
    typedef envelope_rejection<Point, DomainGeometry> type;
};

template
<
    typename Point,
    typename DomainGeometry,
    typename MultiOrSingle,
    typename CsTag
>
struct default_policy
<
    Point,
    DomainGeometry,
    volumetric_tag,
    MultiOrSingle,
    3,  
    CsTag
>
{
    typedef envelope_rejection<Point, DomainGeometry> type;
};

} // namespace services

}} // namespace policy::uniform_point_distribution

}} // namespace boost::geometry

#endif //  BOOST_GEOMETRY_EXTENSIONS_RANDOM_POLICIES_UNIFORM_ENVELOPE_REJECTION_HPP
