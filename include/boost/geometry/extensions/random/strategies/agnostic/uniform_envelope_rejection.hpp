// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2019 Tinko Bartels, Berlin, Germany.

// Contributed and/or modified by Tinko Bartels,
//   as part of Google Summer of Code 2019 program.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_RANDOM_STRATEGIES_AGNOSTIC_UNIFORM_ENVELOPE_REJECTION_HPP
#define BOOST_GEOMETRY_EXTENSIONS_RANDOM_STRATEGIES_AGNOSTIC_UNIFORM_ENVELOPE_REJECTION_HPP

#include <random>
#include <iterator>

#include <boost/geometry/algorithms/equals.hpp>
#include <boost/geometry/algorithms/transform.hpp>
#include <boost/geometry/algorithms/within.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/core/point_type.hpp>

#include <boost/geometry/extensions/random/strategies/uniform_point_distribution.hpp>
#include <boost/geometry/extensions/random/strategies/cartesian/uniform_point_distribution_box.hpp>
#include <boost/geometry/extensions/random/strategies/spherical/uniform_inverse_transform_sampling.hpp>

namespace boost { namespace geometry
{

namespace strategy { namespace uniform_point_distribution {

template
<
    typename Point,
    typename DomainGeometry,
    typename BoxStrategy = services::default_strategy
        <
            typename point_type<DomainGeometry>::type,
            model::box<typename point_type<DomainGeometry>::type>
        >
>
struct uniform_envelope_rejection
{
private:
	typedef model::box<typename point_type<DomainGeometry>::type> envelope_type;
	envelope_type m_env;
	BoxStrategy m_env_strat;
public:
    uniform_envelope_rejection(DomainGeometry const& d) :
		m_env(return_envelope<envelope_type>(d)),
		m_env_strat(m_env) {}
    bool equals(DomainGeometry const& l_domain,
                DomainGeometry const& r_domain,
                uniform_envelope_rejection const& r_strategy) const
    {
        return boost::geometry::equals(l_domain, r_domain);
    }
    template<typename Generator>
    Point apply(Generator& g, DomainGeometry const& d)
    {
        typedef typename point_type<DomainGeometry>::type domain_point;
        domain_point p;
        do {
            p = m_env_strat.apply(g, m_env);
        } while( !::boost::geometry::within(p, d) );
        Point r;
        boost::geometry::transform(p, r);
        return r;
    }
    void reset(DomainGeometry const&) {};
};

namespace services {

template
<
    typename Point,
    typename DomainGeometry,
    typename MultiOrSingle,
    typename CsTag
>
struct default_strategy
<
    Point,
    DomainGeometry,
    areal_tag,
    MultiOrSingle,
    2,
    CsTag
> : public uniform_envelope_rejection<Point, DomainGeometry> {
    typedef uniform_envelope_rejection<Point, DomainGeometry> base;
    using base::base;
};

template
<
    typename Point,
    typename DomainGeometry,
    typename MultiOrSingle,
    typename CsTag
>
struct default_strategy
<
    Point,
    DomainGeometry,
    volumetric_tag,
    MultiOrSingle,
    3,
    CsTag
> : public uniform_envelope_rejection<Point, DomainGeometry> {
    typedef uniform_envelope_rejection<Point, DomainGeometry> base;
    using base::base;
};

} // namespace services

}} // namespace strategy::uniform_point_distribution

}} // namespace boost::geometry

#endif //  BOOST_GEOMETRY_EXTENSIONS_RANDOM_STRATEGIES_AGNOSTIC_UNIFORM_ENVELOPE_REJECTION_HPP
