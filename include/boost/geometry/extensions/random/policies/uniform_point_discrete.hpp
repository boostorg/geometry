// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2020 Tinko Bartels, Berlin, Germany.

// Contributed and/or modified by Tinko Bartels,
//   as part of Google Summer of Code 2019 program.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_RANDOM_POLICIES_UNIFORM_POINT_DISCRETE_HPP
#define BOOST_GEOMETRY_EXTENSIONS_RANDOM_POLICIES_UNIFORM_POINT_DISCRETE_HPP

#include <random>
#include <iterator>

#include <boost/geometry/algorithms/equals.hpp>
#include <boost/geometry/algorithms/transform.hpp>

#include <boost/geometry/extensions/random/policies/uniform_default_policy.hpp>

namespace boost { namespace geometry
{

namespace policy { namespace uniform_point_distribution {

template
<
    typename Point,
    typename DomainGeometry
>
struct single_point
{
    single_point(DomainGeometry const& d) {}
    template<typename Policy>
    bool equals(DomainGeometry const& l_domain,
                DomainGeometry const& r_domain,
                Policy const&) const
    {
        return boost::geometry::equals(l_domain.domain(), r_domain.domain());
    }
    template<typename Generator>
    Point apply(Generator& g, DomainGeometry const& d)
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
struct multi_point
{
    multi_point(DomainGeometry const& p) {}
    bool equals(DomainGeometry const& l_domain,
                DomainGeometry const& r_domain,
                multi_point const&) const
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
    template<typename Generator>
    Point apply(Generator& g, DomainGeometry const& d)
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
struct default_policy
<
    Point,
    DomainGeometry,
    pointlike_tag,
    single_tag,
    Dim,
    CsTag
>
{
    typedef single_point<Point, DomainGeometry> type;
};

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
    pointlike_tag,
    multi_tag,
    Dim,
    CsTag
>
{
    typedef multi_point<Point, DomainGeometry> type;
};

} // namespace services

}} // namespace policy::uniform_point_distribution

}} // namespace boost::geometry

#endif //  BOOST_GEOMETRY_EXTENSIONS_RANDOM_POLICIES_UNIFORM_POINT_DISCRETE_HPP
