// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2020 Oracle and/or its affiliates.
// Contributed and/or modified by Vissarion Fisikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_CLOSEST_POINTS_SEGMENT_BOX_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_CLOSEST_POINTS_SEGMENT_BOX_HPP

#include <boost/geometry/algorithms/detail/distance/segment_to_box.hpp>
#include <boost/geometry/algorithms/detail/disjoint/linear_linear.hpp>

#include <boost/geometry/strategies/cartesian/distance_projected_point.hpp>
#include <boost/geometry/strategies/cartesian/distance_pythagoras.hpp>
#include <boost/geometry/strategies/cartesian/distance_pythagoras_point_box.hpp>
#include <boost/geometry/strategies/cartesian/point_in_point.hpp>
#include <boost/geometry/strategies/cartesian/side_by_triangle.hpp>

namespace boost { namespace geometry
{


namespace strategy { namespace closest_points
{

template
<
    typename CalculationType = void,
    typename Strategy = distance::pythagoras<CalculationType>
>
struct cartesian_segment_box
{
    template <typename PointOfSegment, typename PointOfBox>
    struct calculation_type
        : promote_floating_point
          <
              typename strategy::distance::services::return_type
                  <
                      Strategy,
                      PointOfSegment,
                      PointOfBox
                  >::type
          >
    {};

    typedef cartesian_tag cs_tag;

    // point-point strategy getters
    struct distance_pp_strategy
    {
        typedef Strategy type;
    };

    inline typename distance_pp_strategy::type get_distance_pp_strategy() const
    {
        return typename distance_pp_strategy::type();
    }
    // point-segment strategy getters
    struct distance_ps_strategy
    {
        typedef projected_point<CalculationType, Strategy> type;
    };

    inline typename distance_ps_strategy::type get_distance_ps_strategy() const
    {
        return typename distance_ps_strategy::type();
    }

    struct distance_pb_strategy
    {
        typedef distance::pythagoras_point_box<CalculationType> type;
    };

    inline typename distance_pb_strategy::type get_distance_pb_strategy() const
    {
        return typename distance_pb_strategy::type();
    }

    typedef side::side_by_triangle<CalculationType> side_strategy_type;

    static inline side_strategy_type get_side_strategy()
    {
        return side_strategy_type();
    }

    typedef within::cartesian_point_point equals_point_point_strategy_type;

    static inline equals_point_point_strategy_type get_equals_point_point_strategy()
    {
        return equals_point_point_strategy_type();
    }

    template <typename Geometry1, typename Geometry2>
    struct point_in_geometry_strategy
    {
        typedef strategy::within::cartesian_winding
            <
                typename point_type<Geometry1>::type,
                typename point_type<Geometry2>::type,
                CalculationType
            > type;
    };

    template <typename Geometry1, typename Geometry2>
    static inline typename point_in_geometry_strategy<Geometry1, Geometry2>::type
        get_point_in_geometry_strategy()
    {
        typedef typename point_in_geometry_strategy
            <
                Geometry1, Geometry2
            >::type strategy_type;
        return strategy_type();
    }

    typedef intersection::cartesian_segments
        <
            CalculationType
        > relate_segment_segment_strategy_type;

    static inline relate_segment_segment_strategy_type
    get_relate_segment_segment_strategy()
    {
        return relate_segment_segment_strategy_type();
    }

    template <typename P1, typename P2>
    struct closest_point_result
    {
        typedef geometry::detail::closest_points::result
                <
                    typename calculation_type<P1, P2>::type
                > type;
    };

    template <typename LessEqual, typename ReturnType,
              typename SegmentPoint, typename BoxPoint>
    inline typename closest_point_result<SegmentPoint, BoxPoint>::type
    segment_below_of_box(SegmentPoint const& p0,
                         SegmentPoint const& p1,
                         BoxPoint const&,
                         BoxPoint const&,
                         BoxPoint const&,
                         BoxPoint const& bottom_right) const
    {

        return geometry::detail::distance::segment_to_box_2D
            <
                ReturnType,
                SegmentPoint,
                BoxPoint,
                cartesian_segment_box<CalculationType, Strategy>
            >::template call_above_of_box
                <
                    typename LessEqual::other
                >(p1, p0, bottom_right, *this);
    }

    template <typename SPoint, typename BPoint>
    static bool mirror(SPoint&,
                       SPoint&,
                       BPoint&,
                       BPoint&,
                       BPoint&,
                       BPoint&)
    {
        return false;
    }
};

#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS
namespace services
{

// default strategies

template <typename Segment, typename Box>
struct default_strategy
    <
        segment_tag, box_tag, Segment, Box,
        cartesian_tag, cartesian_tag
    >
{
    typedef cartesian_segment_box<> type;
};

template <typename Box, typename Segment>
struct default_strategy
    <
        box_tag, segment_tag, Box, Segment,
        cartesian_tag, cartesian_tag
    >
{
    typedef typename default_strategy
        <
            segment_tag, box_tag, Segment, Box,
            cartesian_tag, cartesian_tag
        >::type type;
};

} // namespace services
#endif // DOXYGEN_NO_STRATEGY_SPECIALIZATIONS

} // namespace closest_points

namespace distance {

#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS
namespace services
{

template <typename CalculationType, typename Strategy>
struct tag<closest_points::cartesian_segment_box<CalculationType, Strategy> >
{
    typedef strategy_tag_distance_segment_box type;
};

template <typename CalculationType, typename Strategy, typename PS, typename PB>
struct return_type<closest_points::cartesian_segment_box<CalculationType, Strategy>, PS, PB>
    : closest_points::cartesian_segment_box<CalculationType, Strategy>
        ::template closest_point_result<PS, PB>
{};

template <typename CalculationType, typename Strategy>
struct comparable_type<closest_points::cartesian_segment_box<CalculationType, Strategy> >
{
    // Define a cartesian_segment_box strategy with its underlying point-point
    // strategy being comparable
    typedef closest_points::cartesian_segment_box
        <
            CalculationType,
            typename comparable_type<Strategy>::type
        > type;
};


template <typename CalculationType, typename Strategy>
struct get_comparable<closest_points::cartesian_segment_box<CalculationType, Strategy> >
{
    typedef typename comparable_type
        <
            closest_points::cartesian_segment_box<CalculationType, Strategy>
        >::type comparable_type;
public :
    static inline comparable_type
    apply(closest_points::cartesian_segment_box<CalculationType, Strategy> const& )
    {
        return comparable_type();
    }
};

template <typename CalculationType, typename Strategy, typename PS, typename PB>
struct result_from_distance
       <
            closest_points::cartesian_segment_box
            <
                CalculationType,
                Strategy
            >,
            PS,
            PB
       >
{
private :
    typedef typename return_type<
                                    closest_points::cartesian_segment_box
                                    <
                                        CalculationType,
                                        Strategy
                                    >,
                                    PS,
                                    PB
                                 >::type return_type;
public :
    template <typename T>
    static inline return_type
    apply(closest_points::cartesian_segment_box<CalculationType, Strategy> const& ,
          T const& value)
    {
        return value;
    }
};

template
<
    typename CalculationType,
    typename Strategy
>
struct swap<strategy::closest_points::cartesian_segment_box<CalculationType, Strategy> >
{
    template <typename Result>
    static inline void apply(Result& res)
    {
        res.swap();
    }
};

template
<
    typename CalculationType,
    typename Strategy
>
struct result_set_unique_point
<
    strategy::closest_points::cartesian_segment_box<CalculationType, Strategy>
>
{
    template <typename T, typename Point>
    static inline void apply(T& result, Point const& point)
    {
        result.set_unique_point(point);
    }
};

template
<
    typename CalculationType,
    typename SBStrategy
>
struct closest_points_seg_box
<
    strategy::closest_points::cartesian_segment_box<CalculationType, SBStrategy>
>
{
    template
    <
        typename T,
        typename SegmentPoint,
        typename BoxPoint,
        typename Strategy
    >
    static inline void apply(SegmentPoint const& p0,
                             SegmentPoint const& p1,
                             BoxPoint const& top_left,
                             BoxPoint const& top_right,
                             BoxPoint const& bottom_left,
                             BoxPoint const& bottom_right,
                             Strategy const& strategy,
                             T & result)
    {
        typedef geometry::model::segment<SegmentPoint> segment;
        typedef geometry::model::box<BoxPoint> box;

        //TODO pass strategy
        box b(bottom_left, top_right);
        if (geometry::covered_by(p0, b))
        {
            result.set_unique_point(p0);
            return;
        }
        if (geometry::covered_by(p1, b))
        {
            result.set_unique_point(p1);
            return;
        }

        segment s{p0,p1};
        {
            segment sb{top_left, bottom_left};
            auto res =
            geometry::detail::disjoint::disjoint_segment_with_info
            <
                segment,
                segment
            >::apply(s, sb, strategy.get_relate_segment_segment_strategy());
            if (res.count > 0)
            {
                result.set_unique_point(res.intersections[0]);
                return;
            }
        }
        {
            segment sb{top_right, bottom_right};
            auto res =
            geometry::detail::disjoint::disjoint_segment_with_info
            <
                segment,
                segment
            >::apply(s, sb, strategy.get_relate_segment_segment_strategy());
            if (res.count > 0)
            {
                result.set_unique_point(res.intersections[0]);
                return;
            }
        }
        {
            segment sb{bottom_left, bottom_right};
            auto res =
            geometry::detail::disjoint::disjoint_segment_with_info
            <
                segment,
                segment
            >::apply(s, sb, strategy.get_relate_segment_segment_strategy());
            if (res.count > 0)
            {
                result.set_unique_point(res.intersections[0]);
                return;
            }
        }
        {
            segment sb{top_left, top_right};
            auto res =
            geometry::detail::disjoint::disjoint_segment_with_info
            <
                segment,
                segment
            >::apply(s, sb, strategy.get_relate_segment_segment_strategy());
            if (res.count > 0)
            {
                result.set_unique_point(res.intersections[0]);
                return;
            }
        }
    }
};


}
#endif

}} // namespace strategy::distance

}} // namespace boost::geometry
#endif // BOOST_GEOMETRY_STRATEGIES_CARTESIAN_CLOSEST_POINTS_SEGMENT_BOX_HPP
