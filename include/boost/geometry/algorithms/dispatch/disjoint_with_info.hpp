// Boost.Geometry

// Copyright (c) 2020, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ALGORITHMS_DISPATCH_DISJOINT_WITH_INFO_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DISPATCH_DISJOINT_WITH_INFO_HPP

#include <cstddef>

#include <boost/geometry/core/coordinate_dimension.hpp>
#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tag_cast.hpp>
#include <boost/geometry/core/tags.hpp>
#include <boost/geometry/core/reverse_dispatch.hpp>

#include <boost/geometry/algorithms/not_implemented.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


template
<
    typename Geometry1, typename Geometry2,
    std::size_t DimensionCount = dimension<Geometry1>::type::value,
    typename Tag1 = typename tag_cast
        <
            typename tag<Geometry1>::type,
            segment_tag, box_tag, linear_tag, areal_tag
        >::type,
    typename Tag2 = typename tag_cast
        <
            typename tag<Geometry2>::type,
            segment_tag, box_tag, linear_tag, areal_tag
        >::type,
    bool Reverse = reverse_dispatch<Geometry1, Geometry2>::type::value
>
struct disjoint_with_info
    : not_implemented<Geometry1, Geometry2>
{};


// If reversal is needed, perform it
template
<
    typename Geometry1, typename Geometry2,
    std::size_t DimensionCount,
    typename Tag1, typename Tag2
>
struct disjoint_with_info<Geometry1, Geometry2, DimensionCount, Tag1, Tag2, true>
{
    typedef typename point_type<Geometry1>::type point_type;

    typedef segment_intersection_points<point_type> intersection_return_type;

    template <typename Strategy>
    static inline intersection_return_type
    apply(Geometry1 const& g1, Geometry2 const& g2, Strategy const& strategy)
    {
        return disjoint_with_info
            <
                Geometry2, Geometry1,
                DimensionCount,
                Tag2, Tag1
            >::apply(g2, g1, strategy);
    }
};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DISPATCH_DISJOINT_WITH_INFO_HPP
