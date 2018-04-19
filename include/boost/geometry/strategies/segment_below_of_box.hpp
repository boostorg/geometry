// Boost.Geometry

// Copyright (c) 2018, Oracle and/or its affiliates.
// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_SEGMENT_BELOW_OF_BOX_HPP
#define BOOST_GEOMETRY_STRATEGIES_SEGMENT_BELOW_OF_BOX_HPP

namespace boost { namespace geometry
{

namespace strategy { namespace segment_below_of_box
{

namespace services
{

template <typename LessEqual, typename ReturnType, typename PPStrategy, typename CS_Tag>
struct default_strategy
{
    BOOST_MPL_ASSERT_MSG
         (
             false, NOT_IMPLEMENTED_FOR_THIS_COORDINATE_SYSTEM, (types<CS_Tag>)
         );
};

} // namespace services


}} // namespace strategy::side


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_SEGMENT_BELOW_OF_BOX_HPP
