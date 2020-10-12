// Boost.Geometry

// Copyright (c) 2017 Adam Wulkiewicz, Lodz, Poland.

// This file was modified by Oracle on 2020.
// Modifications copyright (c) 2020 Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGY_AREA_RESULT_HPP
#define BOOST_GEOMETRY_STRATEGY_AREA_RESULT_HPP


#include <type_traits>

#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/core/cs.hpp>

#include <boost/geometry/strategies/area/services.hpp>
#include <boost/geometry/strategies/default_strategy.hpp>
#include <boost/geometry/strategies/detail.hpp>
#include <boost/geometry/strategy/area.hpp>

#include <boost/geometry/util/select_most_precise.hpp>
#include <boost/geometry/util/sequence.hpp>

#include <boost/variant/variant_fwd.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace area
{


template
<
    typename Geometry,
    typename Strategy,
    bool IsUmbrella = strategies::detail::is_umbrella_strategy<Strategy>::value
>
struct area_result
{
    typedef decltype(std::declval<Strategy>().area(std::declval<Geometry>())) strategy_type;
    typedef typename strategy_type::template result_type<Geometry>::type type;
};

template
<
    typename Geometry,
    typename Strategy
>
struct area_result<Geometry, Strategy, false>
    : Strategy::template result_type<Geometry>
{};


template <typename Geometry>
struct default_area_result
    : area_result
        <
            Geometry,
            typename geometry::strategies::area::services::default_strategy
                <
                    Geometry
                >::type
        >
{};

template <typename Curr, typename Next>
struct more_precise_coordinate_type
    : std::is_same
        <
            typename geometry::coordinate_type<Curr>::type,
            typename geometry::select_most_precise
                <
                    typename geometry::coordinate_type<Curr>::type,
                    typename geometry::coordinate_type<Next>::type
                >::type
        >
{};


template <typename Curr, typename Next>
struct more_precise_default_area_result
    : std::is_same
        <
            typename default_area_result<Curr>::type,
            typename geometry::select_most_precise
                <
                    typename default_area_result<Curr>::type,
                    typename default_area_result<Next>::type
                >::type
        >
{};


}} // namespace detail::area
#endif //DOXYGEN_NO_DETAIL


/*!
\brief Meta-function defining return type of area function
\ingroup area
\note The return-type is defined by Geometry and Strategy
 */
template
<
    typename Geometry,
    typename Strategy = default_strategy
>
struct area_result
    : detail::area::area_result<Geometry, Strategy>
{};

template <typename ...Ts, typename Strategy>
struct area_result<boost::variant<Ts...>, Strategy>
    : geometry::area_result
        <
            typename util::select_pack_element
                <
                    detail::area::more_precise_coordinate_type,
                    Ts...
                >::type,
            Strategy
        >
{};

template <typename Geometry>
struct area_result<Geometry, default_strategy>
    : detail::area::default_area_result<Geometry>
{};

template <typename ...Ts>
struct area_result<boost::variant<Ts...>, default_strategy>
    : geometry::area_result
        <
            typename util::select_pack_element
                <
                    detail::area::more_precise_default_area_result,
                    Ts...
                >::type,
            default_strategy
        >
{};


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_STRATEGY_AREA_RESULT_HPP
