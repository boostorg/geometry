// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2020 Tinko Bartels, Berlin, Germany.

// Contributed and/or modified by Tinko Bartels,
//   as part of Google Summer of Code 2019 program.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_RANDOM_POLICIES_UNIFORM_DEFAULT_POLICY_HPP
#define BOOST_GEOMETRY_EXTENSIONS_RANDOM_POLICIES_UNIFORM_DEFAULT_POLICY_HPP

#include <boost/geometry/algorithms/equals.hpp>
#include <boost/geometry/algorithms/transform.hpp>
#include <boost/geometry/algorithms/within.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/core/point_type.hpp>

#include <boost/geometry/extensions/random/strategies/uniform_point_distribution_box.hpp>

namespace boost { namespace geometry
{

namespace policy { namespace uniform_point_distribution { namespace services
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
    typename SingleOrMulti = typename tag_cast
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
struct default_policy
{
    BOOST_MPL_ASSERT_MSG
        (
            false, NOT_IMPLEMENTED_FOR_THIS_TYPE
            , (types<Tag, SingleOrMulti, boost::mpl::int_<Dim>, CsTag>)
        );
};

}}} // namespace policy::uniform_point_distribution::services

}} // namespace boost::geometry

#endif //  BOOST_GEOMETRY_EXTENSIONS_RANDOM_POLICIES_UNIFORM_DEFAULT_POLICY_HPP
