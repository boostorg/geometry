// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2014 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2014 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2014 Mateusz Loskot, London, UK.

// This file was modified by Oracle on 2014-2020.
// Modifications copyright (c) 2014-2020, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_CONCEPTS_DISTANCE_CONCEPT_HPP
#define BOOST_GEOMETRY_STRATEGIES_CONCEPTS_DISTANCE_CONCEPT_HPP

#include <iterator>
#include <type_traits>
#include <vector>

#include <boost/concept_check.hpp>

#include <boost/geometry/core/static_assert.hpp>

#include <boost/geometry/geometries/concepts/point_concept.hpp>
#include <boost/geometry/geometries/segment.hpp>
#include <boost/geometry/geometries/point.hpp>

#include <boost/geometry/strategies/distance.hpp>
#include <boost/geometry/strategies/tags.hpp>

#include <boost/geometry/util/parameter_type_of.hpp>


namespace boost { namespace geometry { namespace concepts
{


/*!
    \brief Checks strategy for point-point or point-box or box-box distance
    \ingroup distance
*/
template <typename Strategy, typename Point1, typename Point2>
struct PointDistanceStrategy
{
#ifndef DOXYGEN_NO_CONCEPT_MEMBERS
private :

    struct checker
    {
        template <typename ApplyMethod>
        static void apply(ApplyMethod)
        {
            // 1: inspect and define both arguments of apply
            typedef typename parameter_type_of
                <
                    ApplyMethod, 0
                >::type ptype1;

            typedef typename parameter_type_of
                <
                    ApplyMethod, 1
                >::type ptype2;

            // 2) must define meta-function "return_type"
            typedef typename strategy::distance::services::return_type
                <
                    Strategy, ptype1, ptype2
                >::type rtype;

            // 3) must define meta-function "comparable_type"
            using ctype = typename strategy::distance::services::comparable_type
                <
                    Strategy
                >::type;

            // 4) must define meta-function "tag"
            using tag = typename strategy::distance::services::tag
                <
                    Strategy
                >::type;

            constexpr bool is_correct_strategy_tag =
                   std::is_same<tag, strategy_tag_distance_point_point>::value
                || std::is_same<tag, strategy_tag_distance_point_box>::value
                || std::is_same<tag, strategy_tag_distance_box_box>::value;

            BOOST_GEOMETRY_STATIC_ASSERT(
                 is_correct_strategy_tag,
                 "Incorrect Strategy tag.",
                 Strategy, tag);

            // 5) must implement apply with arguments
            Strategy* str = nullptr;
            ptype1 *p1 = nullptr;
            ptype2 *p2 = nullptr;
            static_assert(std::is_convertible<rtype, decltype(str->apply(*p1, *p2))>::value,
                          "rtype must be initialisable from strategy.apply(ptype1&, ptype2&)");

            // 6) must define (meta)struct "get_comparable" with apply
            static_assert(std::is_convertible
                    <
                        ctype,
                        decltype(strategy::distance::services::get_comparable<Strategy>::apply(*str))
                    >::value,
                "comparable_distance<Strategy>::type must be initialisable from get_comparable<Strategy>::apply(Strategy&)");

            // 7) must define (meta)struct "result_from_distance" with apply
            static_assert(std::is_convertible
                    <
                        rtype,
                        decltype(
                            strategy::distance::services::result_from_distance
                                <
                                    Strategy, ptype1, ptype2
                                >::apply(*str, 1.0))
                    >::value,
                "rtype must be initialisable from result_from_distance.apply(Strategy&, double)");
        }
    };



public :
    BOOST_CONCEPT_USAGE(PointDistanceStrategy)
    {
        checker::apply(&Strategy::template apply<Point1, Point2>);
    }
#endif
};


/*!
    \brief Checks strategy for point-segment distance
    \ingroup strategy_concepts
*/
template <typename Strategy, typename Point, typename PointOfSegment>
struct PointSegmentDistanceStrategy
{
#ifndef DOXYGEN_NO_CONCEPT_MEMBERS
private :

    struct checker
    {
        template <typename ApplyMethod>
        static void apply(ApplyMethod)
        {
            // 1) inspect and define both arguments of apply
            typedef typename parameter_type_of
                <
                    ApplyMethod, 0
                >::type ptype;

            typedef typename parameter_type_of
                <
                    ApplyMethod, 1
                >::type sptype;

            namespace services = strategy::distance::services;
            // 2) must define meta-function "tag"
            typedef typename services::tag<Strategy>::type tag;

            BOOST_GEOMETRY_STATIC_ASSERT(
                (std::is_same
                    <
                        tag, strategy_tag_distance_point_segment
                    >::value),
                "Incorrect Strategy tag.",
                Strategy, tag);

            // 3) must define meta-function "return_type"
            using rtype = typename services::return_type
                <
                    Strategy, ptype, sptype
                >::type;

            // 4) must define meta-function "comparable_type"
            using ctype = typename services::comparable_type<Strategy>::type;

            // 5) must implement apply with arguments
            Strategy *str = nullptr;
            ptype *p = nullptr;
            sptype *sp = nullptr;

            static_assert(std::is_convertible<rtype, decltype(str->apply(*p, *sp, *sp))>::value,
                          "return_type<Strategy, arg1_t, arg2_t>::type must be initialisable from strategy.apply(arg1_t&, arg2_t&, arg2_t&)");

            // 6) must define (meta-)struct "get_comparable" with apply
            static_assert(std::is_convertible<ctype, decltype(services::get_comparable<Strategy>::apply(*str))>::value,
                          "comparable_type<Strategy>::type must be initialisable from get_comparable<Strategy>::apply(Strategy&)");

            // 7) must define (meta-)struct "result_from_distance" with apply
            static_assert(std::is_convertible
                    <
                        rtype,
                        decltype(services::result_from_distance<Strategy, ptype, sptype>::apply(*str, rtype(1.0)))
                    >::value,
                "return_type<Strategy, arg1_t, arg2_t>::type must be initialisable from result_from_distance<Strategy, arg1_t, arg2_t>::apply(Strategy&, rtype&)");
        }
    };

public :
    BOOST_CONCEPT_USAGE(PointSegmentDistanceStrategy)
    {
        checker::apply(&Strategy::template apply<Point, PointOfSegment>);
    }
#endif
};


}}} // namespace boost::geometry::concepts


#endif // BOOST_GEOMETRY_STRATEGIES_CONCEPTS_DISTANCE_CONCEPT_HPP
