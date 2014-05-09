// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_DISTANCE_SEGMENT_TO_BOX_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_DISTANCE_SEGMENT_TO_BOX_HPP

#include <boost/assert.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/mpl/if.hpp>

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/coordinate_dimension.hpp>
#include <boost/geometry/core/point_type.hpp>
#include <boost/geometry/core/tags.hpp>

#include <boost/geometry/util/calculation_type.hpp>
#include <boost/geometry/util/math.hpp>

#include <boost/geometry/strategies/distance.hpp>
#include <boost/geometry/strategies/distance_comparable_to_regular.hpp>
#include <boost/geometry/strategies/tags.hpp>

#include <boost/geometry/policies/compare.hpp>

#include <boost/geometry/algorithms/assign.hpp>
#include <boost/geometry/algorithms/equals.hpp>
#include <boost/geometry/algorithms/not_implemented.hpp>

#include <boost/geometry/algorithms/detail/distance/default_strategies.hpp>

#include <boost/geometry/algorithms/dispatch/distance.hpp>



namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace distance
{


template
<
    typename ReturnType,
    typename SegmentPoint,
    typename BoxPoint,
    typename PPStrategy,
    typename PSStrategy
>
class segment_to_box_2D
{
private:
    template <typename Result>
    struct cast_to_result
    {
        template <typename T>
        static inline Result apply(T const& t)
        {
            return boost::numeric_cast<Result>(t);
        }
    };

    static inline ReturnType
    non_negative_slope_segment(SegmentPoint const& p0,
                               SegmentPoint const& p1,
                               BoxPoint const& top_left,
                               BoxPoint const& top_right,
                               BoxPoint const& bottom_left,
                               BoxPoint const& bottom_right,
                               PPStrategy const& pp_strategy,
                               PSStrategy const& ps_strategy)
    {
        typedef cast_to_result<ReturnType> cast;

        // assert that the segment has non-negative slope
        BOOST_ASSERT( (math::equals(geometry::get<0>(p0),
                                    geometry::get<0>(p1))
                       && geometry::get<1>(p0) < geometry::get<1>(p1))
                      ||
                      (geometry::get<0>(p0) < geometry::get<0>(p1)
                       && geometry::get<1>(p0) <= geometry::get<1>(p1))
                      );

        // p0 lies to the right of the box
        if ( geometry::get<0>(p0) >= geometry::get<0>(top_right) )
        {
            if ( geometry::get<1>(p0) >= geometry::get<1>(top_right) )
            {
                // closest box point is the top-right corner
                return cast::apply(pp_strategy.apply(p0, top_right));
            }
            else if ( geometry::get<1>(p0) >= geometry::get<1>(bottom_right) )
            {
                // distance is realized between p0 and right-most
                // segment of box
                ReturnType diff = cast::apply(geometry::get<0>(p0))
                    - cast::apply(geometry::get<0>(bottom_right));
                return diff * diff;
            }
            else
            {
                // distance is realized between the bottom-right
                // corner of the box and the segment
                return cast::apply(ps_strategy.apply(bottom_right, p0, p1));
            }
        }

        // p1 lies to the left of the box
        if ( geometry::get<0>(p1) <= geometry::get<1>(bottom_left) )
        {
            if ( geometry::get<1>(p1) <= geometry::get<1>(bottom_left) )
            {
                // closest box point is the bottom-left corner
                return cast::apply(pp_strategy.apply(p1, bottom_left));
            }
            else if ( geometry::get<1>(p1) <= geometry::get<1>(top_left) )
            {
                // distance is realized between p1 and left-most
                // segment of box
                ReturnType diff = cast::apply(geometry::get<0>(p1))
                    - cast::apply(geometry::get<0>(bottom_left));
                return diff * diff;
            }
            else
            {
                // distance is realized between the top-left
                // corner of the box and the segment
                return cast::apply(ps_strategy.apply(top_left, p0, p1));
            }
        }

        // the segment lies below the box
        if ( geometry::get<1>(p1) < geometry::get<1>(bottom_left) )
        {
            // p1 is below the lower segment of the box
            // (and inside its band)
            if ( geometry::get<0>(p1) <= geometry::get<0>(bottom_right) )
            {
                ReturnType diff = cast::apply(geometry::get<1>(bottom_right))
                    - cast::apply(geometry::get<1>(p1));
                return diff * diff;
            }

            // p1 is to the right of the box, but p0 is below the box
            // in this case the distance is realized between the
            // bottom-right corner of the box and the segment
            return cast::apply(ps_strategy.apply(bottom_right, p0, p1));
        }

        // the segment lies above the box
        if ( geometry::get<1>(p0) > geometry::get<1>(top_left) )
        {
            // p0 is above the upper segment of the box
            // (and inside its band)
            if ( geometry::get<0>(p0) >= geometry::get<0>(top_left) )
            {
                ReturnType diff = cast::apply(geometry::get<1>(p0))
                    - cast::apply(geometry::get<1>(top_left));
                return diff * diff;
            }

            // p0 is to the left of the box, but p1 is above the box
            // in this case the distance is realized between the
            // top-left corner of the box and the segment
            return cast::apply(ps_strategy.apply(top_left, p0, p1));
        }

        ReturnType diff0 = cast::apply(geometry::get<0>(p1))
            - cast::apply(geometry::get<0>(p0));
        ReturnType t_min0 = cast::apply(geometry::get<0>(bottom_left))
            - cast::apply(geometry::get<0>(p0));
        ReturnType t_max0 = cast::apply(geometry::get<0>(top_right))
            - cast::apply(geometry::get<0>(p0));

        ReturnType diff1 = cast::apply(geometry::get<1>(p1))
            - cast::apply(geometry::get<1>(p0));
        ReturnType t_min1 = cast::apply(geometry::get<1>(bottom_left))
            - cast::apply(geometry::get<1>(p0));
        ReturnType t_max1 = cast::apply(geometry::get<1>(top_right))
            - cast::apply(geometry::get<1>(p0));

        //  t_min0 > t_max1
        if ( t_min0 * diff1 > t_max1 * diff0 )
        {
            return cast::apply(ps_strategy.apply(top_left, p0, p1));
        }

        //  t_min1 > t_max0
        if ( t_min1 * diff0 > t_max0 * diff1 )
        {
            return cast::apply(ps_strategy.apply(bottom_right, p0, p1));
        }

        // in all other cases the box and segment intersect, so return 0
        return ReturnType(0);
    }


    static inline ReturnType
    negative_slope_segment(SegmentPoint const& p0,
                           SegmentPoint const& p1,
                           BoxPoint const& top_left,
                           BoxPoint const& top_right,
                           BoxPoint const& bottom_left,
                           BoxPoint const& bottom_right,
                           PPStrategy const& pp_strategy,
                           PSStrategy const& ps_strategy)
    {
        typedef cast_to_result<ReturnType> cast;

        // assert that the segment has negative slope
        BOOST_ASSERT
            ( geometry::get<0>(p0) < geometry::get<0>(p1)
              && geometry::get<1>(p0) > geometry::get<1>(p1) );

        // p0 lies to the right of the box
        if ( geometry::get<0>(p0) >= geometry::get<0>(top_right) )
        {
            if ( geometry::get<1>(p0) <= geometry::get<1>(bottom_right) )
            {
                // closest box point is the bottom-right corner
                return cast::apply(pp_strategy.apply(p0, bottom_right));
            }
            else if ( geometry::get<1>(p0) <= geometry::get<1>(top_right) )
            {
                // distance is realized between p0 and right-most
                // segment of box
                ReturnType diff = cast::apply(geometry::get<0>(p0))
                    - cast::apply(geometry::get<0>(bottom_right));
                return diff * diff;
            }
            else
            {
                // distance is realized between the top-right
                // corner of the box and the segment
                return cast::apply(ps_strategy.apply(top_right, p0, p1));
            }
        }

        // p1 lies to the left of the box
        if ( geometry::get<0>(p1) <= geometry::get<1>(bottom_left) )
        {
            if ( geometry::get<1>(p1) >= geometry::get<1>(top_left) )
            {
                // closest box point is the top-left corner
                return cast::apply(pp_strategy.apply(p1, top_left));
            }
            else if ( geometry::get<1>(p1) >= geometry::get<1>(bottom_left) )
            {
                // distance is realized between p1 and left-most
                // segment of box
                ReturnType diff = cast::apply(geometry::get<0>(p1))
                    - cast::apply(geometry::get<0>(bottom_left));
                return diff * diff;
            }
            else
            {
                // distance is realized between the bottom-left
                // corner of the box and the segment
                return cast::apply(ps_strategy.apply(bottom_left, p0, p1));
            }
        }

        // the segment lies below the box
        if ( geometry::get<1>(p0) < geometry::get<1>(bottom_left) )
        {
            // p0 is below the lower segment of the box
            // (and inside its band)
            if ( geometry::get<0>(p0) >= geometry::get<0>(bottom_left) )
            {
                ReturnType diff = cast::apply(geometry::get<1>(bottom_left))
                    - cast::apply(geometry::get<1>(p0));
                return diff * diff;
            }

            // p0 is to the left of the box, but p1 is below the box
            // in this case the distance is realized between the
            // bottom-left corner of the box and the segment
            return cast::apply(ps_strategy.apply(bottom_left, p0, p1));
        }

        // the segment lies above the box
        if ( geometry::get<1>(p1) > geometry::get<1>(top_right) )
        {
            // p1 is above the upper segment of the box
            // (and inside its band)
            if ( geometry::get<0>(p1) <= geometry::get<0>(top_right) )
            {
                ReturnType diff = cast::apply(geometry::get<1>(p1))
                    - cast::apply(geometry::get<1>(top_right));
                return diff * diff;
            }

            // p1 is to the right of the box, but p0 is above the box
            // in this case the distance is realized between the
            // top-right corner of the box and the segment
            return cast::apply(ps_strategy.apply(top_right, p0, p1));
        }

        ReturnType diff0 = cast::apply(geometry::get<0>(p1))
            - cast::apply(geometry::get<0>(p0));
        ReturnType t_min0 = cast::apply(geometry::get<0>(bottom_left))
            - cast::apply(geometry::get<0>(p0));
        ReturnType t_max0 = cast::apply(geometry::get<0>(top_right))
            - cast::apply(geometry::get<0>(p0));

        ReturnType diff1 = cast::apply(geometry::get<1>(p0))
            - cast::apply(geometry::get<1>(p1));
        ReturnType t_min1 = cast::apply(geometry::get<1>(p0))
            - cast::apply(geometry::get<1>(top_right));
        ReturnType t_max1 = cast::apply(geometry::get<1>(p0))
            - cast::apply(geometry::get<1>(bottom_left));

        //  t_max0 < t_max1
        if ( t_max0 * diff1 < t_max1 * diff0 )
        {
            return cast::apply(ps_strategy.apply(top_right, p0, p1));
        }

        //  t_min0 > t_min1
        if ( t_min0 * diff1 > t_min1 * diff0 )
        {
            return cast::apply(ps_strategy.apply(bottom_left, p0, p1));
        }

        // in all other cases the box and segment intersect, so return 0
        return ReturnType(0);
    }

public:
    static inline ReturnType apply(SegmentPoint const& p0,
                                   SegmentPoint const& p1,
                                   BoxPoint const& top_left,
                                   BoxPoint const& top_right,
                                   BoxPoint const& bottom_left,
                                   BoxPoint const& bottom_right,
                                   PPStrategy const& pp_strategy,
                                   PSStrategy const& ps_strategy)
    {
        BOOST_ASSERT( geometry::less<SegmentPoint>()(p0, p1) );

        if ( geometry::get<0>(p0) < geometry::get<0>(p1)
             && geometry::get<1>(p0) > geometry::get<1>(p1) )
        {
            return negative_slope_segment(p0, p1,
                                          top_left, top_right,
                                          bottom_left, bottom_right,
                                          pp_strategy,
                                          ps_strategy);
        }

        return non_negative_slope_segment(p0, p1,
                                          top_left, top_right,
                                          bottom_left, bottom_right,
                                          pp_strategy,
                                          ps_strategy);
    }
};


//=========================================================================
//=========================================================================
//=========================================================================

template
<
    typename Segment,
    typename Box,
    typename std::size_t Dimension,
    typename PPStrategy,
    typename PSStrategy
>
class segment_to_box
    : not_implemented<Segment, Box>
{};


template
<
    typename Segment,
    typename Box,
    typename PPStrategy,
    typename PSStrategy
>
class segment_to_box<Segment, Box, 2, PPStrategy, PSStrategy>
{
private:
    typedef typename point_type<Segment>::type segment_point;
    typedef typename point_type<Box>::type box_point;

    typedef typename strategy::distance::services::comparable_type
        <
            PPStrategy
        >::type pp_comparable_strategy;

    typedef typename strategy::distance::services::comparable_type
        <
            PSStrategy
        >::type ps_comparable_strategy;

public:
    typedef typename strategy::distance::services::return_type
        <
            PSStrategy, segment_point, box_point
        >::type return_type;

    static inline return_type apply(Segment const& segment,
                                    Box const& box,
                                    PPStrategy const& pp_strategy,
                                    PSStrategy const& ps_strategy)
    {
        segment_point p[2];
        detail::assign_point_from_index<0>(segment, p[0]);
        detail::assign_point_from_index<1>(segment, p[1]);

        if ( geometry::equals(p[0], p[1]) )
        {
            typedef typename boost::mpl::if_
                <
                    boost::is_same
                        <
                            ps_comparable_strategy,
                            PSStrategy
                        >,
                    typename strategy::distance::services::comparable_type
                        <
                            typename detail::distance::default_strategy
                                <
                                    segment_point, Box
                                >::type
                        >::type,
                    typename detail::distance::default_strategy
                        <
                            segment_point, Box
                        >::type
                >::type point_box_strategy_type;

            return dispatch::distance
                <
                    segment_point,
                    Box,
                    point_box_strategy_type
                >::apply(p[0], box, point_box_strategy_type());
        }

        box_point top_left, top_right, bottom_left, bottom_right;
        detail::assign_box_corners(box, bottom_left, bottom_right,
                                   top_left, top_right);

        pp_comparable_strategy c_pp_strategy =
            strategy::distance::services::get_comparable
                <
                    PPStrategy
                >::apply(pp_strategy);

        ps_comparable_strategy c_ps_strategy =
            strategy::distance::services::get_comparable
                <
                    PSStrategy
                >::apply(ps_strategy);

        return_type d;

        if ( geometry::less<segment_point>()(p[0], p[1]) )
        {
            d = segment_to_box_2D
                <
                    return_type,
                    segment_point,
                    box_point,
                    pp_comparable_strategy,
                    ps_comparable_strategy
                >::apply(p[0], p[1],
                         top_left, top_right, bottom_left, bottom_right,
                         c_pp_strategy,
                         c_ps_strategy);
        }
        else
        {
            d = segment_to_box_2D
                <
                    return_type,
                    segment_point,
                    box_point,
                    pp_comparable_strategy,
                    ps_comparable_strategy
                >::apply(p[1], p[0],
                         top_left, top_right, bottom_left, bottom_right,
                         c_pp_strategy,
                         c_ps_strategy);
        }

        return strategy::distance::services::comparable_to_regular
            <
                ps_comparable_strategy, PSStrategy, Segment, Box
            >::apply( d );
    }
};


}} // namespace detail::distance
#endif // DOXYGEN_NO_DETAIL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


template <typename Segment, typename Box, typename Strategy>
struct distance
    <
        Segment, Box, Strategy, segment_tag, box_tag,
        strategy_tag_distance_point_segment, false
    >
{
    typedef typename strategy::distance::services::return_type
        <
            Strategy,
            typename point_type<Segment>::type,
            typename point_type<Box>::type
        >::type return_type;


    static inline return_type apply(Segment const& segment,
                                    Box const& box,
                                    Strategy const& strategy)
    {
        assert_dimension_equal<Segment, Box>();

        typedef typename detail::distance::default_strategy
            <
                typename point_type<Segment>::type,
                typename point_type<Box>::type
            >::type pp_strategy_type;

        return detail::distance::segment_to_box
            <
                Segment,
                Box,
                dimension<Segment>::value,
                pp_strategy_type,
                Strategy
            >::apply(segment, box, pp_strategy_type(), strategy);
    }
};



} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_DISTANCE_SEGMENT_TO_BOX_HPP
