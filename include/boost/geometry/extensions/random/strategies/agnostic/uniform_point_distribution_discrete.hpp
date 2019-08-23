// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2019 Tinko Bartels, Berlin, Germany.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_RANDOM_STRATEGIES_AGNOSTIC_UNIFORM_POINT_DISTRIBUTION_DISCRETE_HPP
#define BOOST_GEOMETRY_EXTENSIONS_RANDOM_STRATEGIES_AGNOSTIC_UNIFORM_POINT_DISTRIBUTION_DISCRETE_HPP

#include <random>
#include <iterator>

#include <boost/geometry/algorithms/equals.hpp>
#include <boost/geometry/algorithms/transform.hpp>

#include <boost/geometry/extensions/random/strategies/uniform_point_distribution.hpp>

namespace boost { namespace geometry
{

namespace strategy { namespace uniform_point_distribution {

template
<
    typename Point,
    typename DomainGeometry
>
struct single_point_distribution
{
    single_point_distribution(DomainGeometry const& g) {}
    template<typename Strategy>
    bool equals(DomainGeometry const& l_domain,
                DomainGeometry const& r_domain,
                Strategy const& r_strategy) const
    {
        return boost::geometry::equals(l_domain.domain(), r_domain.domain());
    }
    template<typename Gen>
    Point apply(Gen& g, DomainGeometry const& d)
    {
        Point r;
        boost::geometry::transform(d, r);
        return r;
    }
    void reset(DomainGeometry const&) {};
};

template
<
    typename Point,
    typename DomainGeometry
>
struct multi_point_distribution
{
    multi_point_distribution(DomainGeometry const& p) {}
    bool equals(DomainGeometry const& l_domain,
                DomainGeometry const& r_domain,
                multi_point_distribution const& r_strategy) const
    {
        if(boost::size(l_domain) != boost::size(r_domain)) return false;
        for (std::size_t i = 0; i < boost::size(l_domain); ++i)
            if( !boost::geometry::equals( *(std::begin(l_domain) + i),
                                          *(std::begin(r_domain) + i)) )
            {
                return false;
            }
        return true;
    }
    template<typename Gen>
    Point apply(Gen& g, DomainGeometry const& d)
    {
        Point r;
        if(boost::size(d) == 0) return r;
        std::uniform_int_distribution<std::size_t>
            int_dist(0, boost::size(d) - 1);
        boost::geometry::transform(*(std::begin(d) + int_dist(g)), r);
        return r;
    }
    void reset(DomainGeometry const&) {}
};

namespace services {

template
<
    typename Point,
    typename DomainGeometry,
    int Dim,
    typename CsTag
>
struct default_strategy
<
    Point,
    DomainGeometry,
    pointlike_tag,
    single_tag,
    Dim,
    CsTag
> : public single_point_distribution<Point, DomainGeometry> {
    typedef single_point_distribution<Point, DomainGeometry> base;
    using base::base;
};

template
<
    typename Point,
    typename DomainGeometry,
    int Dim,
    typename CsTag
>
struct default_strategy
<
    Point,
    DomainGeometry,
    pointlike_tag,
    multi_tag,
    Dim,
    CsTag
> : public multi_point_distribution<Point, DomainGeometry> {
    typedef multi_point_distribution<Point, DomainGeometry> base;
    using base::base;
};

} // namespace services

}} // namespace strategy::uniform_point_distribution

}} // namespace boost::geometry

#endif //  BOOST_GEOMETRY_EXTENSIONS_RANDOM_STRATEGIES_AGNOSTIC_UNIFORM_POINT_DISTRIBUTION_DISCRETE_HPP
