// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2016-2020 Oracle and/or its affiliates.
// Contributed and/or modified by Vissarion Fisikopoulos, on behalf of Oracle
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_ENVELOPE_HPP
#define BOOST_GEOMETRY_STRATEGIES_ENVELOPE_HPP

#include <boost/mpl/assert.hpp>

namespace boost { namespace geometry
{


namespace strategy { namespace envelope { namespace services
{

/*!
\brief Traits class binding a default envelope strategy to a coordinate system
\ingroup util
\tparam Tag tag of geometry
\tparam CSTag tag of coordinate system
\tparam CalculationType \tparam_calculation
*/
template <typename Tag, typename CSTag, typename CalculationType = void>
struct default_strategy
{
    BOOST_MPL_ASSERT_MSG
        (
            false, NOT_IMPLEMENTED_FOR_THIS_TYPE
            , (types<Tag, CSTag>)
        );
};

}}} // namespace strategy::envelope::services


namespace strategies { namespace envelope { namespace services
{

template
<
    typename Geometry,
    typename Box,
    typename CSTag = typename geometry::cs_tag<Geometry>::type
>
struct default_strategy
{
    BOOST_MPL_ASSERT_MSG
    (
        false, NOT_IMPLEMENTED_FOR_THIS_COORDINATE_SYSTEM
        , (types<Geometry, Box, CSTag>)
    );
};

template <typename Strategy>
struct strategy_converter
{
    BOOST_MPL_ASSERT_MSG
    (
        false, NOT_IMPLEMENTED_FOR_THIS_STRATEGY
        , (Strategy)
    );
};


}}} // namespace strategies::envelope::services


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_ENVELOPE_HPP

