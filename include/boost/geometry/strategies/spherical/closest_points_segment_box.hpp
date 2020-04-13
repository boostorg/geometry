// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2019 Oracle and/or its affiliates.
// Contributed and/or modified by Vissarion Fisikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_SPHERICAL_CLOSEST_POINTS_SEGMENT_BOX_HPP
#define BOOST_GEOMETRY_STRATEGIES_SPHERICAL_CLOSEST_POINTS_SEGMENT_BOX_HPP

#include <boost/geometry/algorithms/detail/closest_points/result.hpp>
#include <boost/geometry/algorithms/detail/distance/segment_to_box.hpp>

#include <boost/geometry/strategies/distance.hpp>
#include <boost/geometry/strategies/normalize.hpp>
#include <boost/geometry/strategies/spherical/disjoint_box_box.hpp>
#include <boost/geometry/strategies/spherical/distance_segment_box.hpp>
#include <boost/geometry/strategies/spherical/point_in_point.hpp>

#include <boost/geometry/util/promote_floating_point.hpp>
#include <boost/geometry/util/select_calculation_type.hpp>

namespace boost { namespace geometry
{


namespace strategy { namespace closest_points
{

template
<
    typename CalculationType = void,
    typename Strategy = distance::haversine<double, CalculationType>
>
struct spherical_segment_box
{
    template <typename PointOfSegment, typename PointOfBox>
    struct calculation_type
        : promote_floating_point
          <
              typename select_calculation_type
                  <
                      PointOfSegment,
                      PointOfBox,
                      CalculationType
                  >::type
          >
    {};

    typedef spherical_tag cs_tag;

    // strategy getters

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
        typedef cross_track<CalculationType, Strategy> type;
    };

    inline typename distance_ps_strategy::type get_distance_ps_strategy() const
    {
        return typename distance_ps_strategy::type();
    }

    struct distance_pb_strategy
    {
        typedef cross_track_point_box<CalculationType, Strategy> type;
    };

    inline typename distance_pb_strategy::type get_distance_pb_strategy() const
    {
        return typename distance_pb_strategy::type();
    }

    template <typename SegmentPoint, typename BoxPoint>
    struct closest_point_result
    {
        typedef geometry::detail::closest_points::result
        <
            typename calculation_type<SegmentPoint, BoxPoint>::type
        > type;
    };

    //constructors

    inline spherical_segment_box()
    {}

    explicit inline spherical_segment_box(typename Strategy::radius_type const& r)
        : m_strategy(r)
    {}

    inline spherical_segment_box(Strategy const& s)
        : m_strategy(s)
    {}

    // TODO: why is the Radius not propagated above?

    typedef side::spherical_side_formula<CalculationType> side_strategy_type;

    static inline side_strategy_type get_side_strategy()
    {
        return side_strategy_type();
    }

    typedef within::spherical_point_point equals_point_point_strategy_type;

    static inline equals_point_point_strategy_type get_equals_point_point_strategy()
    {
        return equals_point_point_strategy_type();
    }

    template <typename Geometry1, typename Geometry2>
    struct point_in_geometry_strategy
    {
        typedef strategy::within::spherical_winding
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

    // methods

    template
    <
        typename LessEqual,
        typename ReturnType,
        typename SegmentPoint,
        typename BoxPoint
    >
    inline typename closest_point_result<SegmentPoint, BoxPoint>::type
    segment_below_of_box(SegmentPoint const& p0,
                         SegmentPoint const& p1,
                         BoxPoint const& top_left,
                         BoxPoint const& top_right,
                         BoxPoint const& bottom_left,
                         BoxPoint const& bottom_right) const
    {
        typedef typename azimuth::spherical<CalculationType> azimuth_strategy_type;
        azimuth_strategy_type az_strategy;

        typedef typename envelope::spherical_segment<CalculationType>
                                             envelope_segment_strategy_type;
        envelope_segment_strategy_type es_strategy;

        return distance::generic_segment_box::segment_below_of_box
               <
                    LessEqual,
                    ReturnType
               >(p0,p1,top_left,top_right,bottom_left,bottom_right,
                 spherical_segment_box<CalculationType>(),
                 az_strategy, es_strategy,
                 normalize::spherical_point(),
                 covered_by::spherical_point_box(),
                 disjoint::spherical_box_box());
    }

    template <typename SPoint, typename BPoint>
    static bool mirror(SPoint& p0,
                       SPoint& p1,
                       BPoint& bottom_left,
                       BPoint& bottom_right,
                       BPoint& top_left,
                       BPoint& top_right)
    {

        return distance::generic_segment_box::mirror(p0, p1,
                                                     bottom_left, bottom_right,
                                                     top_left, top_right);
    }

private :
    Strategy m_strategy;

};

#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS
namespace services {

// default strategies

template <typename Segment, typename Box>
struct default_strategy
    <
        segment_tag, box_tag, Segment, Box,
        spherical_equatorial_tag, spherical_equatorial_tag
    >
{
    typedef spherical_segment_box<> type;
};

template <typename Box, typename Segment>
struct default_strategy
    <
        box_tag, segment_tag, Box, Segment,
        spherical_equatorial_tag, spherical_equatorial_tag
    >
{
    typedef typename default_strategy
        <
            segment_tag, box_tag, Segment, Box,
            spherical_equatorial_tag, spherical_equatorial_tag
        >::type type;
};

}} // namespace closest_points::services

namespace distance { namespace services
{

//tags

template
<
        typename CalculationType,
        typename Strategy
>
struct tag<closest_points::spherical_segment_box<CalculationType, Strategy> >
{
    typedef strategy_tag_distance_segment_box type;
};

template
<
        typename CalculationType
>
struct tag<closest_points::spherical_segment_box<CalculationType> >
{
    typedef strategy_tag_distance_segment_box type;
};

// return types

template <typename CalculationType, typename PS, typename PB>
struct return_type<closest_points::spherical_segment_box<CalculationType>, PS, PB>
    :   closest_points::spherical_segment_box<CalculationType>
        ::template closest_point_result<PS,PB>
{};

template
<
        typename CalculationType,
        typename Strategy,
        typename PS,
        typename PB
>
struct return_type<closest_points::spherical_segment_box<CalculationType, Strategy>, PS, PB>
        :   closest_points::spherical_segment_box<CalculationType, Strategy>
            ::template closest_point_result<PS,PB>
{};

//comparable types

template
<
        typename CalculationType,
        typename Strategy
>
struct comparable_type<closest_points::spherical_segment_box<CalculationType, Strategy> >
{
    typedef closest_points::spherical_segment_box<
            CalculationType, Strategy
        >  type;
};

template
<
        typename CalculationType,
        typename Strategy
>
struct get_comparable<closest_points::spherical_segment_box<CalculationType, Strategy> >
{
    typedef typename comparable_type
        <
            closest_points::spherical_segment_box<CalculationType, Strategy>
        >::type comparable_type;
public :
    static inline comparable_type
    apply(closest_points::spherical_segment_box<CalculationType, Strategy> const& )
    {
        return comparable_type();
    }
};

// result from distance

template
<
    typename CalculationType,
    typename Strategy,
    typename PS,
    typename PB
>
struct result_from_distance<closest_points::spherical_segment_box<CalculationType, Strategy>, PS, PB>
{
private :
    typedef typename closest_points::spherical_segment_box
        <
            CalculationType, Strategy
        >::template closest_point_result<PS, PB>::type return_type;
public :
    template <typename T>
    static inline return_type
    apply(closest_points::spherical_segment_box<CalculationType, Strategy> const& ,
          T const& distance)
    {
        return distance;
    }
};

template
<
    typename CalculationType,
    typename Strategy
>
struct swap<strategy::closest_points::spherical_segment_box<CalculationType, Strategy> >
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
struct mirror
       <
            strategy::closest_points::spherical_segment_box
            <
                CalculationType,
                Strategy
            >
       >
{
    template <typename Result>
    static inline void apply(Result& res)
    {
        res.mirror();
    }
};

template
<
    typename CalculationType,
    typename Strategy
>
struct result_set_unique_point
<
    strategy::closest_points::spherical_segment_box<CalculationType, Strategy>
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
    typename Strategy
>
struct closest_points_seg_box
<
    strategy::closest_points::spherical_segment_box<CalculationType, Strategy>
>
{
    template
    <
        typename T,
        typename SegmentPoint,
        typename BoxPoint,
        typename DistancePointSegmentStrategy
    >
    static inline void apply(SegmentPoint const& p0,
                             SegmentPoint const& p1,
                             BoxPoint const& top_left,
                             BoxPoint const& top_right,
                             BoxPoint const& bottom_left,
                             BoxPoint const& bottom_right,
                             DistancePointSegmentStrategy const& ps_strategy,
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

        segment s{p0,p1}, sout;
        {
            geometry::closest_points(top_left, s, sout);
            if (geometry::covered_by(sout.second, b))
            {
                result.set_unique_point(sout.second);
                return;
            }
        }
        {
            geometry::closest_points(top_right, s, sout);
            if (geometry::covered_by(sout.second, b))
            {
                result.set_unique_point(sout.second);
                return;
            }
        }
        {
            geometry::closest_points(bottom_left, s, sout);
            if (geometry::covered_by(sout.second, b))
            {
                result.set_unique_point(sout.second);
                return;
            }
        }
        {
            geometry::closest_points(bottom_right, s, sout);
            if (geometry::covered_by(sout.second, b))
            {
                result.set_unique_point(sout.second);
                return;
            }
        }

    }
};

} // namespace services
#endif // DOXYGEN_NO_STRATEGY_SPECIALIZATIONS

}} // namespace strategy::distance

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_SPHERICAL_CLOSEST_POINTS_SEGMENT_BOX_HPP
