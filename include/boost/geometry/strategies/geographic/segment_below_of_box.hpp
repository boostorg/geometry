// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2018 Oracle and/or its affiliates.
// Contributed and/or modified by Vissarion Fisikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_SEGMENT_BELOW_OF_BOX_HPP
#define BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_SEGMENT_BELOW_OF_BOX_HPP

#include <boost/geometry/strategies/segment_below_of_box.hpp>

namespace boost { namespace geometry
{


namespace strategy { namespace segment_below_of_box
{

template <typename LessEqual, typename ReturnType, typename PPStrategy>
struct geographic
{
    template <typename SegmentPoint, typename BoxPoint, typename PSStrategy>
    static inline ReturnType apply(SegmentPoint const& p0,
                                   SegmentPoint const& p1,
                                   BoxPoint const& top_left,
                                   BoxPoint const& top_right,
                                   BoxPoint const& bottom_left,
                                   BoxPoint const& bottom_right,
                                   PSStrategy const& ps_strategy)
    {
        return spherical<LessEqual, ReturnType, PPStrategy>::
                apply(p0, p1, top_left, top_right, bottom_left, bottom_right, ps_strategy);
    }
};

#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS
namespace services
{

template <typename LessEqual, typename ReturnType, typename PPStrategy>
struct default_strategy<LessEqual, ReturnType, PPStrategy, geographic_tag>
{
    typedef geographic<LessEqual, ReturnType, PPStrategy> type;
};

}
#endif

}} // namespace strategy::segment_below_of_box

}} // namespace boost::geometry
#endif // BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_SEGMENT_BELOW_OF_BOX_HPP
