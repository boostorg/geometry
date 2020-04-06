// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2019 Tinko Bartels, Berlin, Germany.

// Contributed and/or modified by Tinko Bartels,
//   as part of Google Summer of Code 2019 program.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_RANDOM_STRATEGIES_UNIFORM_POINT_DISTRIBUTION_TRIANGLE_HPP
#define BOOST_GEOMETRY_EXTENSIONS_RANDOM_STRATEGIES_UNIFORM_POINT_DISTRIBUTION_TRIANGLE_HPP

#include <boost/mpl/assert.hpp>

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
    typename DomainPoint,
    typename CsTag = typename tag_cast
                         <
                             typename cs_tag<DomainPoint>::type,
                             spherical_tag
                         >::type
>
struct default_triangle_strategy
{
    BOOST_MPL_ASSERT_MSG
        (
            false, NOT_IMPLEMENTED_FOR_THIS_TYPE
            , (types<CsTag>)
        );
};

}}} // namespace strategy::uniform_point_distribution::services

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_EXTENSIONS_RANDOM_STRATEGIES_UNIFORM_POINT_DISTRIBUTION_TRIANGLE_HPP
