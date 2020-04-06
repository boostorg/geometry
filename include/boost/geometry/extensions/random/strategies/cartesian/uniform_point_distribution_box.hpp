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

#include <boost/geometry/extensions/random/strategies/uniform_point_distribution_box.hpp>

namespace boost { namespace geometry
{

namespace strategy { namespace uniform_point_distribution
{

namespace detail
{

template<typename Point, typename Box, std::size_t Dim, typename CalculationType>
struct interval_product
{
    Box const& m_b;
    std::array<CalculationType, Dim> const& m_fractions;
    inline interval_product(Box const& b, std::array<CalculationType, Dim> const& f) : 
        m_b(b), m_fractions(f) {}

    template <typename PointDst, std::size_t Index>
    inline void apply(PointDst& point_dst) const
    {
        set<Index>(point_dst, 
                   get<min_corner, Index>(m_b) + 
                         m_fractions[Index]
                       * (get<max_corner, Index>(m_b) - get<min_corner, Index>(m_b)) );
    }
};

} // namespace detail

template
<
    typename Point,
    typename DomainGeometry,
    int Dim
>
struct cartesian_box
{
    template<typename CalculationType>
    static Point apply(DomainGeometry const& d, std::array<CalculationType, Dim> const& f)
    {
        Point r;
        for_each_coordinate(r,
            detail::interval_product<Point, DomainGeometry, Dim, CalculationType>(d, f));
        return r;
    }
};

namespace services {

template
<
    typename Point,
    typename DomainGeometry,
    int Dim
>
struct default_box_strategy
<
    Point,
    DomainGeometry,
    Dim,
    cartesian_tag
>
{
    typedef cartesian_box<Point, DomainGeometry, Dim> type;
};

} // namespace services

}} // namespace strategy::uniform_point_distribution

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_EXTENSIONS_RANDOM_STRATEGIES_CARTESIAN_UNIFORM_POINT_DISTRIBUTION_BOX_HPP
