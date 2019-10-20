// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2019 Tinko Bartels, Berlin, Germany.

// Contributed and/or modified by Tinko Bartels,
//   as part of Google Summer of Code 2019 program.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_RANDOM_STRATEGIES_CARTESIAN_UNIFORM_POINT_DISTRIBUTION_BOX_HPP
#define BOOST_GEOMETRY_EXTENSIONS_RANDOM_STRATEGIES_CARTESIAN_UNIFORM_POINT_DISTRIBUTION_BOX_HPP

#include <random>
#include <iterator>

#include <boost/geometry/algorithms/equals.hpp>
#include <boost/geometry/util/for_each_coordinate.hpp>

#include <boost/geometry/extensions/random/strategies/uniform_point_distribution.hpp>

namespace boost { namespace geometry
{

namespace strategy { namespace uniform_point_distribution {

namespace detail
{

template
<
    typename Point,
    typename Box,
    typename Generator,
    bool isIntegral =
        boost::is_integral<typename coordinate_type<Point>::type>::type::value
>
struct interval_sample {};

template<typename Point, typename Box, typename Generator>
struct interval_sample<Point, Box, Generator, true>
{
    Box const& b;
    Generator& gen;
    inline interval_sample(Box const& b, Generator& gen):b(b),gen(gen) {}
    template <typename PointDst, std::size_t Index>
    inline void apply(PointDst& point_dst) const
    {
        std::uniform_int_distribution<typename coordinate_type<Point>::type>
            dist(get<min_corner,Index>(b),get<max_corner,Index>(b));
        set<Index>(point_dst,dist(gen));
    }
};

template<typename Point, typename Box, typename Generator>
struct interval_sample<Point, Box, Generator, false>
{
    Box const& b;
    Generator& gen;
    inline interval_sample(Box const& b, Generator& gen):b(b),gen(gen) {}

    template <typename PointDst, std::size_t Index>
    inline void apply(PointDst& point_dst) const
    {
        std::uniform_real_distribution<typename coordinate_type<Point>::type>
            dist(get<min_corner,Index>(b),get<max_corner,Index>(b));
        set<Index>(point_dst,dist(gen));
    }
};

}

template
<
    typename Point,
    typename DomainGeometry,
    int Dim
>
struct interval_product_distribution
{
    interval_product_distribution(DomainGeometry const& d) {}
    bool equals(DomainGeometry const& l_domain,
                DomainGeometry const& r_domain,
                interval_product_distribution const& r_strategy) const
    {
        return boost::geometry::equals(l_domain.domain(), r_domain.domain());
    }
    template<typename Generator>
    Point apply(Generator& g, DomainGeometry const& d)
    {
        Point r;
        for_each_coordinate(r,
        	detail::interval_sample<Point, DomainGeometry, Generator>(d, g));
        return r;
    }
    void reset(DomainGeometry const&) {};
};

namespace services {

template
<
    typename Point,
    typename DomainGeometry,
    int Dim
>
struct default_strategy
<
    Point,
    DomainGeometry,
    box_tag,
    single_tag, //There are no MultiBoxes right now
    Dim,
    cartesian_tag
> : public interval_product_distribution<Point, DomainGeometry, Dim> {
    typedef interval_product_distribution<Point, DomainGeometry, Dim> base;
    using base::base;
};

} // namespace services

}} // namespace strategy::uniform_point_distribution

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_EXTENSIONS_RANDOM_STRATEGIES_CARTESIAN_UNIFORM_POINT_DISTRIBUTION_BOX_HPP
