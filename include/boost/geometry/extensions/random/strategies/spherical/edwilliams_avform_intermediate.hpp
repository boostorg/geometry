// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2019 Tinko Bartels, Berlin, Germany.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_RANDOM_STRATEGIES_SPHERICAL_EDWILLIAMS_AVFORM_INTERMEDIATE_HPP
#define BOOST_GEOMETRY_EXTENSIONS_RANDOM_STRATEGIES_SPHERICAL_EDWILLIAMS_AVFORM_INTERMEDIATE_HPP

#include <random>
#include <cmath>

#include <boost/geometry/algorithms/equals.hpp>
#include <boost/geometry/arithmetic/arithmetic.hpp>

#include <boost/geometry/extensions/random/strategies/uniform_point_distribution.hpp>

namespace boost { namespace geometry
{

namespace strategy { namespace uniform_point_distribution {

template
<
    typename Point,
    typename DomainGeometry
>
struct edwilliams_avform_intermediate
{
    edwilliams_avform_intermediate(DomainGeometry const& g) {}
    bool equals(DomainGeometry const& l_domain,
                DomainGeometry const& r_domain,
                edwilliams_avform_intermediate const& r_strategy) const
    {
        return boost::geometry::equals(l_domain.domain(), r_domain.domain());
    }

    // The following implementation is adapted from
    // https://www.edwilliams.org/avform.htm#Intermediate
    template
    <
        typename LengthType,
        typename PointIn
    >
    static Point map(PointIn const& p1, PointIn const& p2, LengthType const& f)
    {
        Point out;
        const auto lat1 = get_as_radian<1>(p1);
        const auto lon1 = get_as_radian<0>(p1);
        const auto lat2 = get_as_radian<1>(p2);
        const auto lon2 = get_as_radian<0>(p2);
        LengthType const d = std::acos(
              std::sin(lat1) * std::sin(lat2)
            + std::cos(lat1) * std::cos(lat2) * std::cos(lon1 - lon2));
        LengthType const A = std::sin( ( 1 - f ) * d ) / std::sin(d);
        LengthType const B = std::sin( f * d ) / std::sin( d );
        LengthType const x = A * std::cos(lat1) * std::cos(lon1)
                           + B * std::cos(lat2) * std::cos(lon2);
        LengthType const y = A * std::cos(lat1) * std::sin(lon1)
                           + B * std::cos(lat2) * std::sin(lon2);
        LengthType const z = A * std::sin(lat1) + B * std::sin(lat2);
        LengthType const lat = std::atan2(z, std::sqrt(x * x + y * y));
        LengthType const lon = std::atan2(y, x);
        set_from_radian<1>(out, lat);
        set_from_radian<0>(out, lon);
        return out;
    }

    template<typename Gen>
    Point apply(Gen& g, DomainGeometry const& d)
    {
        typedef typename select_most_precise
            <
                typename coordinate_type<DomainGeometry>::type,
                double
            >::type sample_type;
        std::uniform_real_distribution<sample_type> real_dist(0, 1);
        return map<sample_type>(d, real_dist(g));
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
    segment_tag,
    single_tag,
    2,
    spherical_tag
> : public edwilliams_avform_intermediate<Point, DomainGeometry> {
    typedef edwilliams_avform_intermediate<Point, DomainGeometry> base;
    using base::base;
};

} // namespace services

}} // namespace strategy::uniform_point_distribution

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_EXTENSIONS_RANDOM_STRATEGIES_SPHERICAL_EDWILLIAMS_AVFORM_INTERMEDIATE_HPP
