// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2018 Oracle and/or its affiliates.
// Contributed and/or modified by Vissarion Fisikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_SEGMENT_BELOW_OF_BOX_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_SEGMENT_BELOW_OF_BOX_HPP

#include <boost/geometry/algorithms/detail/distance/segment_to_box.hpp>

namespace boost { namespace geometry
{


namespace strategy { namespace segment_below_of_box
{

template <typename LessEqual, typename ReturnType, typename PPStrategy>
struct cartesian
{
    template <typename SegmentPoint, typename BoxPoint, typename PSStrategy>
    static inline ReturnType apply(SegmentPoint const& p0,
                                   SegmentPoint const& p1,
                                   BoxPoint const&,
                                   BoxPoint const&,
                                   BoxPoint const&,
                                   BoxPoint const& bottom_right,
                                   PSStrategy const& ps_strategy)
    {
        return geometry::detail::distance::segment_to_box_2D
            <
                ReturnType,
                SegmentPoint,
                BoxPoint,
                PPStrategy,
                PSStrategy
            >::template call_above_of_box
                <
                    typename LessEqual::other
                >(p1, p0, bottom_right, ps_strategy);
    }
};

#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS
namespace services
{

template <typename LessEqual, typename ReturnType, typename PPStrategy>
struct default_strategy<LessEqual, ReturnType, PPStrategy, cartesian_tag>
{
    typedef cartesian<LessEqual, ReturnType, PPStrategy> type;
};

}
#endif

}} // namespace strategy::segment_below_of_box

}} // namespace boost::geometry
#endif // BOOST_GEOMETRY_STRATEGIES_CARTESIAN_SEGMENT_BELOW_OF_BOX_HPP
