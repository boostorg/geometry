// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2019 Oracle and/or its affiliates.
// Contributed and/or modified by Vissarion Fisikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_CLOSEST_POINTS_SEGMENT_BOX_HPP
#define BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_CLOSEST_POINTS_SEGMENT_BOX_HPP

#include <boost/geometry/srs/spheroid.hpp>

#include <boost/geometry/algorithms/closest_points.hpp>
#include <boost/geometry/algorithms/detail/closest_points/result.hpp>
#include <boost/geometry/algorithms/detail/distance/segment_to_box.hpp>

#include <boost/geometry/formulas/karney_direct_variant.hpp>

#include <boost/geometry/strategies/distance.hpp>
#include <boost/geometry/strategies/geographic/azimuth.hpp>
#include <boost/geometry/strategies/geographic/distance_cross_track.hpp>
#include <boost/geometry/strategies/geographic/parameters.hpp>
#include <boost/geometry/strategies/geographic/side.hpp>
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
    typename FormulaPolicy = strategy::andoyer,
    typename Spheroid = srs::spheroid<double>,
    typename CalculationType = void
>
struct geographic_segment_box
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

    typedef geographic_tag cs_tag;

    // point-point strategy getters
    struct distance_pp_strategy
    {
        typedef geographic<FormulaPolicy, Spheroid, CalculationType> type;
    };

    inline typename distance_pp_strategy::type get_distance_pp_strategy() const
    {
        typedef typename distance_pp_strategy::type distance_type;
        return distance_type(m_spheroid);
    }
    // point-segment strategy getters
    struct distance_ps_strategy
    {
        typedef geographic_cross_track
                <
                    FormulaPolicy,
                    Spheroid,
                    CalculationType
                > type;
    };

    inline typename distance_ps_strategy::type get_distance_ps_strategy() const
    {
        typedef typename distance_ps_strategy::type distance_type;
        return distance_type(m_spheroid);
    }

    struct distance_pb_strategy
    {
        typedef geographic_cross_track_point_box
                <
                    FormulaPolicy,
                    Spheroid,
                    CalculationType
                > type;
    };

    inline typename distance_pb_strategy::type get_distance_pb_strategy() const
    {
        return typename distance_pb_strategy::type(m_spheroid);
    }

    typedef side::geographic
            <
                FormulaPolicy,
                Spheroid,
                CalculationType
            > side_strategy_type;

    inline side_strategy_type get_side_strategy() const
    {
        return side_strategy_type(m_spheroid);
    }

    typedef within::spherical_point_point equals_point_point_strategy_type;

    static inline equals_point_point_strategy_type get_equals_point_point_strategy()
    {
        return equals_point_point_strategy_type();
    }

    template <typename Geometry1, typename Geometry2>
    struct point_in_geometry_strategy
    {
        typedef strategy::within::geographic_winding
            <
                typename point_type<Geometry1>::type,
                typename point_type<Geometry2>::type,
                FormulaPolicy,
                Spheroid,
                CalculationType
            > type;
    };

    template <typename Geometry1, typename Geometry2>
    inline typename point_in_geometry_strategy<Geometry1, Geometry2>::type
        get_point_in_geometry_strategy() const
    {
        typedef typename point_in_geometry_strategy
            <
                Geometry1, Geometry2
            >::type strategy_type;
        return strategy_type(m_spheroid);
    }

    typedef intersection::geographic_segments
        <
            FormulaPolicy,
            strategy::default_order<FormulaPolicy>::value,
            Spheroid,
            CalculationType
        > relate_segment_segment_strategy_type;

    inline relate_segment_segment_strategy_type
    get_relate_segment_segment_strategy() const
    {
        return relate_segment_segment_strategy_type(m_spheroid);
    }

    typedef geographic_segment_box
        <
            FormulaPolicy, Spheroid, CalculationType
        > closest_points_segment_box_strategy_type;

    inline closest_points_segment_box_strategy_type
    get_closest_points_segment_box_strategy() const
    {
        return closest_points_segment_box_strategy_type(m_spheroid);
    }

    Spheroid get_spheroid() const
    {
        return m_spheroid;
    }

    //constructor

    explicit geographic_segment_box(Spheroid const& spheroid = Spheroid())
             : m_spheroid(spheroid)
    {}

    template <typename SegmentPoint, typename BoxPoint>
    struct closest_point_result
    {
        typedef geometry::detail::closest_points::result
        <
            typename calculation_type<SegmentPoint, BoxPoint>::type
        > type;
    };

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
        typedef typename azimuth::geographic
        <
                FormulaPolicy,
                Spheroid,
                CalculationType
        > azimuth_strategy_type;
        azimuth_strategy_type az_strategy(m_spheroid);

        typedef typename envelope::geographic_segment
        <
                FormulaPolicy,
                Spheroid,
                CalculationType
        > envelope_segment_strategy_type;
        envelope_segment_strategy_type es_strategy(m_spheroid);

        return distance::generic_segment_box::segment_below_of_box
               <
                    LessEqual,
                    ReturnType
               >(p0,p1,top_left,top_right,bottom_left,bottom_right,
                 geographic_segment_box<FormulaPolicy, Spheroid, CalculationType>(),
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
    Spheroid m_spheroid;
};

#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS
namespace services {

// default strategies

template <typename Segment, typename Box>
struct default_strategy
    <
        segment_tag, box_tag, Segment, Box,
        geographic_tag, geographic_tag
    >
{
    typedef geographic_segment_box<> type;
};

template <typename Box, typename Segment>
struct default_strategy
    <
        box_tag, segment_tag, Box, Segment,
        geographic_tag, geographic_tag
    >
{
    typedef typename default_strategy
        <
            segment_tag, box_tag, Segment, Box,
            geographic_tag, geographic_tag
        >::type type;
};

}} // namespace closest_points::services

namespace distance { namespace services
{

//tags

template <typename FormulaPolicy>
struct tag<closest_points::geographic_segment_box<FormulaPolicy> >
{
    typedef strategy_tag_distance_segment_box type;
};

template
<
        typename FormulaPolicy,
        typename Spheroid
>
struct tag<closest_points::geographic_segment_box<FormulaPolicy, Spheroid> >
{
    typedef strategy_tag_distance_segment_box type;
};

template
<
        typename FormulaPolicy,
        typename Spheroid,
        typename CalculationType
>
struct tag<closest_points::geographic_segment_box<FormulaPolicy, Spheroid, CalculationType> >
{
    typedef strategy_tag_distance_segment_box type;
};

// return types

template <typename FormulaPolicy, typename PS, typename PB>
struct return_type<closest_points::geographic_segment_box<FormulaPolicy>, PS, PB>
    :   closest_points::geographic_segment_box<FormulaPolicy>
        ::template closest_point_result<PS,PB>
{};

template
<
        typename FormulaPolicy,
        typename Spheroid,
        typename PS,
        typename PB
>
struct return_type<closest_points::geographic_segment_box<FormulaPolicy, Spheroid>, PS, PB>
        :   closest_points::geographic_segment_box<FormulaPolicy, Spheroid>
            ::template closest_point_result<PS,PB>
{};


template
<
        typename FormulaPolicy,
        typename Spheroid,
        typename CalculationType,
        typename PS,
        typename PB
>
struct return_type<closest_points::geographic_segment_box<FormulaPolicy, Spheroid, CalculationType>, PS, PB>
        :   closest_points::geographic_segment_box<FormulaPolicy, Spheroid, CalculationType>
            ::template closest_point_result<PS,PB>
{};


//comparable types

template
<
        typename FormulaPolicy,
        typename Spheroid,
        typename CalculationType
>
struct comparable_type<closest_points::geographic_segment_box<FormulaPolicy, Spheroid, CalculationType> >
{
    typedef closest_points::geographic_segment_box
        <
            FormulaPolicy, Spheroid, CalculationType
        >  type;
};

template
<
        typename FormulaPolicy,
        typename Spheroid,
        typename CalculationType
>
struct get_comparable<closest_points::geographic_segment_box<FormulaPolicy, Spheroid, CalculationType> >
{
    typedef typename comparable_type
        <
            closest_points::geographic_segment_box<FormulaPolicy, Spheroid, CalculationType>
        >::type comparable_type;
public :
    static inline comparable_type
    apply(closest_points::geographic_segment_box<FormulaPolicy, Spheroid, CalculationType> const& )
    {
        return comparable_type();
    }
};

// result from distance

template
<
    typename FormulaPolicy,
    typename PS,
    typename PB
>
struct result_from_distance<closest_points::geographic_segment_box<FormulaPolicy>, PS, PB>
{
private :
    typedef typename closest_points::geographic_segment_box
        <
            FormulaPolicy
        >::template closest_point_result<PS, PB>::type closest_point_result;
public :
    template <typename T>
    static inline closest_point_result
    apply(closest_points::geographic_segment_box<FormulaPolicy> const&,
          T const& distance)
    {
        return distance;
    }
};

template
<
    typename FormulaPolicy,
    typename Spheroid,
    typename CalculationType,
    typename PS,
    typename PB
>
struct result_from_distance<closest_points::geographic_segment_box<FormulaPolicy, Spheroid, CalculationType>, PS, PB>
{
private :
    typedef typename closest_points::geographic_segment_box
        <
            FormulaPolicy, Spheroid, CalculationType
        >::template closest_point_result<PS, PB>::type return_type;
public :
    template <typename T>
    static inline return_type
    apply(closest_points::geographic_segment_box<FormulaPolicy,
                                                 Spheroid,
                                                 CalculationType> const& ,
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
struct swap
       <
            strategy::closest_points::geographic_segment_box
            <
                CalculationType,
                Strategy
            >
       >
{
    template <typename Result>
    static inline void apply(Result& res)
    {
        res.swap();
    }
};

template
<
    typename FormulaPolicy,
    typename Spheroid,
    typename CalculationType
>
struct mirror
       <
            strategy::closest_points::geographic_segment_box
            <
                FormulaPolicy,
                Spheroid,
                CalculationType
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
    strategy::closest_points::geographic_segment_box<CalculationType, Strategy>
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
    strategy::closest_points::geographic_segment_box<CalculationType, SBStrategy>
>
{
    template
    <
        typename T,
        typename SegmentPoint,
        typename BoxPoint,
        typename Strategy
    >
    static inline bool apply_part(SegmentPoint const& p0,
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
            return true;
        }
        if (geometry::covered_by(p1, b))
        {
            result.set_unique_point(p1);
            return true;
        }

        //check segment intersection with box meridians
        segment s{p0,p1};
        {
            segment sb{bottom_left, top_left};
            auto res =
            geometry::detail::disjoint::disjoint_segment_with_info
            <
                segment,
                segment
            >::apply(s, sb, strategy.get_relate_segment_segment_strategy());
            if (res.count > 0)
            {
                result.set_unique_point(res.intersections[0]);
                return true;
            }
        }
        {
            segment sb{bottom_right, top_right};
            auto res =
            geometry::detail::disjoint::disjoint_segment_with_info
            <
                segment,
                segment
            >::apply(s, sb, strategy.get_relate_segment_segment_strategy());
            if (res.count > 0)
            {
                result.set_unique_point(res.intersections[0]);
                return true;
            }
        }

        // treat the other (parallel) edges of box differently
        // since they are not (geodesic) segments
        typedef typename coordinate_type<SegmentPoint>::type CT;

        CT lon1 = get_as_radian<0>(p0);
        CT lat1 = get_as_radian<1>(p0);
        CT lon2 = get_as_radian<0>(p1);
        CT lat2 = get_as_radian<1>(p1);
        CT lat3 = get_as_radian<1>(bottom_left);

        // ensure that az1 \in [0, pi/2]
        if (lon1 > lon2)
        {
            std::swap(lon1, lon2);
            std::swap(lat1, lat2);
        }
        if (lat1 > lat2)
        {
            lat1 *= -1;
            lat2 *= -1;
            lat3 *= -1;
        }

        CT az1;
        azimuth::geographic<>().apply(lon1, lat1, lon2, lat2, az1);

        CT lon3 = formula::karney_direct_variant<CT>
                ::apply(lon1 * math::r2d<CT>(),
                        lat1 * math::r2d<CT>(),
                        lat3 * math::r2d<CT>(),
                        az1 * math::r2d<CT>(),
                        strategy.get_spheroid());

        CT lon_left = get_as_radian<0>(bottom_left);
        CT lon_right = get_as_radian<0>(bottom_right);

#ifdef BOOST_GEOMETRY_DEBUG_GEOGRAPHIC_SEGMENT_BOX
        std::cout << lon_left * math::r2d<CT>() << " , "
                  << lon_right * math::r2d<CT>() << " , "
                  << lon3 << std::endl;
#endif
        math::normalize_longitude<radian>(lon_left);
        math::normalize_longitude<radian>(lon_right);
        math::normalize_longitude<degree>(lon3);
#ifdef BOOST_GEOMETRY_DEBUG_GEOGRAPHIC_SEGMENT_BOX
        std::cout << lon_left * math::r2d<CT>() << " , "
                  << lon_right * math::r2d<CT>() << " , "
                  << lon3 << std::endl;
#endif
        if ( lon_left * math::r2d<CT>() < lon3
             && lon3 < lon_right * math::r2d<CT>() )
        {
            SegmentPoint p;
            set<0>(p, lon3);
            set_from_radian<1>(p, get_as_radian<1>(bottom_left));
            result.set_unique_point(p);
            return true;
        }
        return false;
    }

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


        if (apply_part(p0, p1,
                       top_left, top_right,
                       bottom_left, bottom_right,
                       strategy, result))
        {
            return;
        }
        else
        {
            typedef typename coordinate_type<SegmentPoint>::type CT;

            CT az1;
            CT lon1 = get_as_radian<0>(p0);
            CT lat1 = get_as_radian<1>(p0);
            CT lon2 = get_as_radian<0>(p1);
            CT lat2 = get_as_radian<1>(p1);

            azimuth::geographic<>().apply(lon1, lat1, lon2, lat2, az1);
            CT vertex_lat = geometry::formula::vertex_latitude<CT, geographic_tag>
                                    ::apply(lon1,
                                            az1,
                                            strategy.get_spheroid());

            CT vertex_lon = geometry::formula::vertex_longitude<CT, geographic_tag>
                                    ::apply(lon1, lat1, lon2, lat2,
                                            vertex_lat,
                                            az1,
                                            azimuth::geographic<>());
            SegmentPoint vertex;
            set_from_radian<0>(vertex, vertex_lon);
            set_from_radian<1>(vertex, vertex_lat);

            apply_part(vertex, p1,
                       top_left, top_right,
                       bottom_left, bottom_right,
                       strategy, result);
        }
    }
};

} // namespace services
#endif // DOXYGEN_NO_STRATEGY_SPECIALIZATIONS

}} // namespace strategy::distance

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_CLOSEST_POINTS_SEGMENT_BOX_HPP
