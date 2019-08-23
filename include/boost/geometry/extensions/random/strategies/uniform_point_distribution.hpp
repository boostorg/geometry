// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2019 Tinko Bartels, Berlin, Germany.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_RANDOM_STRATEGIES_UNIFORM_POINT_DISTRIBUTION_HPP
#define BOOST_GEOMETRY_EXTENSIONS_RANDOM_STRATEGIES_UNIFORM_POINT_DISTRIBUTION_HPP

#include <boost/geometry/core/coordinate_dimension.hpp>
#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tags.hpp>
#include <boost/geometry/core/tag_cast.hpp>

namespace boost { namespace geometry
{

namespace strategy { namespace uniform_point_distribution { namespace services
{

template
<
    typename Point,
    typename DomainGeometry,
    typename Tag = typename tag_cast
        <
            typename tag<DomainGeometry>::type,
            segment_tag,
            box_tag,
            linear_tag,
            areal_tag,
            pointlike_tag
        >::type,
    typename MultiOrSingle = typename tag_cast
        <
            typename tag<DomainGeometry>::type,
            single_tag,
            multi_tag
        >::type,
    int Dim = dimension<DomainGeometry>::value,
    typename CsTag = typename tag_cast
                                <
                                    typename cs_tag<DomainGeometry>::type,
                                    spherical_tag
                                >::type
>
struct default_strategy
{};

}}} // namespace strategy::uniform_point_distribution::services

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_EXTENSIONS_RANDOM_STRATEGIES_UNIFORM_POINT_DISTRIBUTION_HPP
