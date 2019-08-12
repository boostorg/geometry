// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2019 Tinko Bartels, Berlin, Germany.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_RANDOM_UNIFORM_POINT_DISTRIBUTION_HPP
#define BOOST_GEOMETRY_EXTENSIONS_RANDOM_UNIFORM_POINT_DISTRIBUTION_HPP

#include <boost/geometry/extensions/random/dispatch/uniform_point_distribution.hpp>

namespace boost { namespace geometry { namespace random
{

template
<
    typename DomainGeometry,
    typename Point = typename geometry::point_type<DomainGeometry>::type
>
inline dispatch::uniform_point_distribution<DomainGeometry, Point>
    uniform_point_distribution(DomainGeometry const& domain)
{
    return dispatch::uniform_point_distribution<DomainGeometry, Point>(domain);
}

}}} // namespace boost::geometry::random

template
<
    typename Char,
    typename Traits,
    typename Point,
    typename DomainGeometry
>
inline std::basic_ostream<Char, Traits>& operator<<
    (
        std::basic_ostream<Char, Traits> &os,
        boost::geometry::random::dispatch::uniform_point_distribution<
            DomainGeometry, Point> const& dist
    )
{
    os << boost::geometry::wkt<DomainGeometry>(dist.domain());
    return os;
}

template<typename Char, typename Traits, typename Point, typename DomainGeometry>
inline std::basic_istream<Char, Traits>& operator>>
    (
        std::basic_istream<Char, Traits> &is,
        boost::geometry::random::dispatch::uniform_point_distribution<
            DomainGeometry, Point> & dist
    )
{
    std::basic_string<Char, Traits> line;
    std::getline(is, line);
    DomainGeometry g;
    namespace bg = boost::geometry;
    typedef typename bg::random::dispatch::uniform_point_distribution<
        DomainGeometry, Point>::param_type param_type;
    bg::read_wkt<DomainGeometry>(line, g);
    dist.param( param_type(g) );
    return is;
}

#endif // BOOST_GEOMETRY_EXTENSIONS_RANDOM_UNIFORM_POINT_DISTRIBUTION_HPP
