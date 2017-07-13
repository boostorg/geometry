// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2016-2017, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_DISTANCE_CROSS_TRACK_HPP
#define BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_DISTANCE_CROSS_TRACK_HPP

#include <algorithm>

#include <boost/config.hpp>
#include <boost/concept_check.hpp>
#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_void.hpp>

#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/radian_access.hpp>
#include <boost/geometry/core/tags.hpp>

#include <boost/geometry/algorithms/detail/course.hpp>

#include <boost/geometry/strategies/distance.hpp>
#include <boost/geometry/strategies/concepts/distance_concept.hpp>
#include <boost/geometry/strategies/spherical/distance_haversine.hpp>
#include <boost/geometry/strategies/geographic/azimuth.hpp>
#include <boost/geometry/strategies/geographic/parameters.hpp>

#include <boost/geometry/formulas/vincenty_direct.hpp>
#include <boost/geometry/formulas/distance_point_segment.hpp>

#include <boost/geometry/util/math.hpp>
#include <boost/geometry/util/promote_floating_point.hpp>
#include <boost/geometry/util/select_calculation_type.hpp>

#ifdef BOOST_GEOMETRY_DEBUG_CROSS_TRACK
#  include <boost/geometry/io/dsv/write.hpp>
#endif


namespace boost { namespace geometry
{

namespace strategy { namespace distance
{


/*!
\brief Strategy functor for distance point to segment calculation
\ingroup strategies
\details Class which calculates the distance of a point to a segment, for points on a sphere or globe
\see http://williams.best.vwh.net/avform.htm
\tparam CalculationType \tparam_calculation
\tparam Strategy underlying point-point distance strategy
\qbk{
[heading See also]
[link geometry.reference.algorithms.distance.distance_3_with_strategy distance (with strategy)]
}
*/
template
<
    typename FormulaPolicy = strategy::andoyer,
    typename Spheroid = srs::spheroid<double>,
    typename CalculationType = void
>
class cross_track_geo
{
public :
    template <typename Point, typename PointOfSegment>
    struct return_type
        : promote_floating_point
          <
              typename select_calculation_type
                  <
                      Point,
                      PointOfSegment,
                      CalculationType
                  >::type
          >
    {};

    inline cross_track_geo()
    {}

    typedef geographic
                    <
                        FormulaPolicy, Spheroid, CalculationType
                    >   DistanceStrategy;

 /*
    template <typename T>
    inline T comparable_to_distance(T& a) const
    {
        return a;
    }
*/
    template <typename Point, typename PointOfSegment>
    inline typename return_type<Point, PointOfSegment>::type
    apply(Point const& p, PointOfSegment const& sp1, PointOfSegment const& sp2) const
    {
        typedef typename coordinate_system<Point>::type::units units_type;

        typedef typename return_type<Point, PointOfSegment>::type CT;

        typedef typename FormulaPolicy::template inverse
                <
                    CT,
                    false,
                    true,
                    false,
                    false,
                    false
                > inverse_type_azimuth;

        typedef typename FormulaPolicy::template inverse
                <
                    CT,
                    true,
                    true,
                    false,
                    true,
                    true
                > inverse_type_distance;

        typedef typename FormulaPolicy::template direct
                <
                    CT,
                    true,
                    false,
                    false,
                    false
                > direct_type;

        return geometry::formula::cross_track_geo_formula
                <
                    CT,
                    units_type,
                    inverse_type_azimuth,
                    inverse_type_distance,
                    direct_type
                >::apply(get<0>(sp1), get<1>(sp1),
                         get<0>(sp2), get<1>(sp2),
                         get<0>(p), get<1>(p));
    }

    //TODO: apply a more general strategy getter
    inline DistanceStrategy get_distance_strategy() const
    {
        return DistanceStrategy();
    }

};



#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS
namespace services
{

//tags
template <typename FormulaPolicy>
struct tag<cross_track_geo<FormulaPolicy> >
{
    typedef strategy_tag_distance_point_segment type;
};

template
<
        typename FormulaPolicy,
        typename Spheroid
>
struct tag<cross_track_geo<FormulaPolicy, Spheroid> >
{
    typedef strategy_tag_distance_point_segment type;
};

template
<
        typename FormulaPolicy,
        typename Spheroid,
        typename CalculationType
>
struct tag<cross_track_geo<FormulaPolicy, Spheroid, CalculationType> >
{
    typedef strategy_tag_distance_point_segment type;
};


//return types
template <typename FormulaPolicy, typename P, typename PS>
struct return_type<cross_track_geo<FormulaPolicy>, P, PS>
    : cross_track_geo<FormulaPolicy>::template return_type<P, PS>
{};

template
<
        typename FormulaPolicy,
        typename Spheroid,
        typename P,
        typename PS
>
struct return_type<cross_track_geo<FormulaPolicy, Spheroid>, P, PS>
    : cross_track_geo<FormulaPolicy, Spheroid>::template return_type<P, PS>
{};

template
<
        typename FormulaPolicy,
        typename Spheroid,
        typename CalculationType,
        typename P,
        typename PS
>
struct return_type<cross_track_geo<FormulaPolicy, Spheroid, CalculationType>, P, PS>
    : cross_track_geo<FormulaPolicy, Spheroid, CalculationType>::template return_type<P, PS>
{};

//comparable types
template
<
        typename FormulaPolicy,
        typename Spheroid,
        typename CalculationType
>
struct comparable_type<cross_track_geo<FormulaPolicy, Spheroid, CalculationType> >
{
    typedef cross_track_geo
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
struct get_comparable<cross_track_geo<FormulaPolicy, Spheroid, CalculationType> >
{
    typedef typename comparable_type
        <
            cross_track_geo<FormulaPolicy, Spheroid, CalculationType>
        >::type comparable_type;
public :
    static inline comparable_type
    apply(cross_track_geo<FormulaPolicy, Spheroid, CalculationType> const& strategy)
    {
        return comparable_type();
    }
};


template
<
    typename FormulaPolicy,
    typename P,
    typename PS
>
struct result_from_distance<cross_track_geo<FormulaPolicy>, P, PS>
{
private :
    typedef typename cross_track_geo
        <
            FormulaPolicy
        >::template return_type<P, PS>::type return_type;
public :
    template <typename T>
    static inline return_type
    apply(cross_track_geo<FormulaPolicy> const& , T const& distance)
    {
        return distance;
    }
};

/*
template
<
    typename FormulaPolicy,
    typename P,
    typename PS
>
struct result_from_comparable
    <
        cross_track_geo<FormulaPolicy>, P, PS
    >
{
private :
    typedef cross_track_geo<FormulaPolicy> strategy_type;
//    typedef typename return_type<strategy_type, P, PS>::type return_type;
public :
    template <typename T>
    static inline T apply(strategy_type const& strategy,
                          T const& comparable_distance)
    {
        //T c = T(2.0) * asin(math::sqrt(a));
        //return c * strategy.radius();
        return comparable_distance;
    }
};
*/

template <typename Point, typename PointOfSegment>
struct default_strategy
    <
        point_tag, segment_tag, Point, PointOfSegment,
        geographic_tag, geographic_tag
    >
{
    typedef cross_track_geo<> type;
};


template <typename PointOfSegment, typename Point>
struct default_strategy
    <
        segment_tag, point_tag, PointOfSegment, Point,
        geographic_tag, geographic_tag
    >
{
    typedef typename default_strategy
        <
            point_tag, segment_tag, Point, PointOfSegment,
            geographic_tag, geographic_tag
        >::type type;
};

} // namespace services
#endif // DOXYGEN_NO_STRATEGY_SPECIALIZATIONS

}} // namespace strategy::distance

}} // namespace boost::geometry
#endif // BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_DISTANCE_CROSS_TRACK_HPP
