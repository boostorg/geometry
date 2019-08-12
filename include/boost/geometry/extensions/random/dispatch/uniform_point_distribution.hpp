// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2019 Tinko Bartels, Berlin, Germany.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_RANDOM_DISPATCH_UNIFORM_POINT_DISTRIBUTION_HPP
#define BOOST_GEOMETRY_EXTENSIONS_RANDOM_DISPATCH_UNIFORM_POINT_DISTRIBUTION_HPP

#include <boost/geometry/core/coordinate_dimension.hpp>
#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tag_cast.hpp>
#include <boost/geometry/core/cs.hpp>

#include <boost/geometry/extensions/random/detail/uniform_point_distribution.hpp>

namespace boost { namespace geometry { namespace random { namespace dispatch
{

template
<
    typename DomainGeometry,
    typename Point,
    typename DomainTag = typename tag_cast
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
    typename CSTag = typename tag_cast
        <
            typename cs_tag<Point>::type,
            spherical_tag
        >::type,
    std::size_t dimension = dimension<Point>::type::value
>
class uniform_point_distribution : public detail::uniform_point_distribution
        <
            Point, DomainGeometry, DomainTag,
            MultiOrSingle,
            CSTag,
            dimension
        >
{
public:
    typedef detail::uniform_point_distribution
        <
            Point,
            DomainGeometry,
            DomainTag,
            MultiOrSingle,
            CSTag,
            dimension
        > base;
    using base::base;
};

}}}} // namespace boost::geometry::random::dispatch

#endif // BOOST_GEOMETRY_EXTENSIONS_RANDOM_DISPATCH_UNIFORM_POINT_DISTRIBUTION_HPP
