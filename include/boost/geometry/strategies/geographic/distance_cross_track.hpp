// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2016-2019, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_DISTANCE_CROSS_TRACK_HPP
#define BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_DISTANCE_CROSS_TRACK_HPP

#include <algorithm>

#include <boost/tuple/tuple.hpp>
#include <boost/algorithm/minmax.hpp>
#include <boost/config.hpp>
#include <boost/concept_check.hpp>
#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_void.hpp>

#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/radian_access.hpp>
#include <boost/geometry/core/tags.hpp>

#include <boost/geometry/strategies/distance.hpp>
#include <boost/geometry/strategies/concepts/distance_concept.hpp>
#include <boost/geometry/strategies/spherical/distance_cross_track.hpp>
#include <boost/geometry/strategies/spherical/distance_haversine.hpp>
#include <boost/geometry/strategies/spherical/point_in_point.hpp>
#include <boost/geometry/strategies/geographic/azimuth.hpp>
#include <boost/geometry/strategies/geographic/distance.hpp>
#include <boost/geometry/strategies/geographic/parameters.hpp>
#include <boost/geometry/strategies/geographic/intersection.hpp>

#include <boost/geometry/formulas/vincenty_direct.hpp>

#include <boost/geometry/util/math.hpp>
#include <boost/geometry/util/promote_floating_point.hpp>
#include <boost/geometry/util/select_calculation_type.hpp>
#include <boost/geometry/util/normalize_spheroidal_coordinates.hpp>

#include <boost/geometry/formulas/point_segment_distance.hpp>
#include <boost/geometry/formulas/result_direct.hpp>
#include <boost/geometry/formulas/mean_radius.hpp>
#include <boost/geometry/formulas/spherical.hpp>

#ifdef BOOST_GEOMETRY_DEBUG_GEOGRAPHIC_CROSS_TRACK
#include <boost/geometry/io/dsv/write.hpp>
#endif

#ifndef BOOST_GEOMETRY_DETAIL_POINT_SEGMENT_DISTANCE_MAX_STEPS
#define BOOST_GEOMETRY_DETAIL_POINT_SEGMENT_DISTANCE_MAX_STEPS 100
#endif

#ifdef BOOST_GEOMETRY_DEBUG_GEOGRAPHIC_CROSS_TRACK
#include <iostream>
#endif

namespace boost { namespace geometry
{

namespace strategy { namespace distance
{

namespace detail
{
/*!
\brief Strategy functor for distance point to segment calculation on ellipsoid
       Algorithm uses direct and inverse geodesic problems as subroutines.
       The algorithm approximates the distance by an iterative Newton method.
\ingroup strategies
\details Class which calculates the distance of a point to a segment, for points
on the ellipsoid
\see C.F.F.Karney - Geodesics on an ellipsoid of revolution,
      https://arxiv.org/abs/1102.1215
\tparam FormulaPolicy underlying point-point distance strategy
\tparam Spheroid is the spheroidal model used
\tparam CalculationType \tparam_calculation
\tparam EnableClosestPoint computes the closest point on segment if true
*/
template
<
    typename FormulaPolicy = strategy::andoyer,
    typename Spheroid = srs::spheroid<double>,
    typename CalculationType = void,
    bool Bisection = false,
    bool EnableClosestPoint = false
>
class geographic_cross_track
{
public :
    typedef within::spherical_point_point equals_point_point_strategy_type;

    typedef intersection::geographic_segments
        <
            FormulaPolicy,
            strategy::default_order<FormulaPolicy>::value,
            Spheroid,
            CalculationType
        > relate_segment_segment_strategy_type;

    inline relate_segment_segment_strategy_type get_relate_segment_segment_strategy() const
    {
        return relate_segment_segment_strategy_type(m_spheroid);
    }

    typedef within::geographic_winding
        <
            void, void, FormulaPolicy, Spheroid, CalculationType
        > point_in_geometry_strategy_type;

    inline point_in_geometry_strategy_type get_point_in_geometry_strategy() const
    {
        return point_in_geometry_strategy_type(m_spheroid);
    }

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

    explicit geographic_cross_track(Spheroid const& spheroid = Spheroid())
        : m_spheroid(spheroid)
    {}

    template <typename Point, typename PointOfSegment>
    inline typename return_type<Point, PointOfSegment>::type
    apply(Point const& p, PointOfSegment const& sp1, PointOfSegment const& sp2) const
    {
        return (formula::point_segment_distance
                <
                    typename return_type<Point, PointOfSegment>::type,
                    false,
                    FormulaPolicy,
                    Bisection
                >
                       ::apply(get_as_radian<0>(sp1), get_as_radian<1>(sp1),
                               get_as_radian<0>(sp2), get_as_radian<1>(sp2),
                               get_as_radian<0>(p), get_as_radian<1>(p),
                               m_spheroid).distance);
    }

    template <typename ResultType>
    inline ResultType
    apply(ResultType comparable_result) const
    {
        return comparable_result;
    }

    // points on a meridian not crossing poles
    template <typename CT>
    inline CT vertical_or_meridian(CT const& lat1,
                                   CT const& lat2,
                                   CT const& lon) const
    {
        boost::ignore_unused(lon);
        typedef typename formula::meridian_inverse
        <
            CT,
            strategy::default_order<FormulaPolicy>::value
        > meridian_inverse;

        return meridian_inverse::meridian_not_crossing_pole_dist(lat1, lat2,
                                                                 m_spheroid);
    }

    Spheroid const& model() const
    {
        return m_spheroid;
    }

private :

    Spheroid m_spheroid;
};

} // namespace detail

template
<
    typename FormulaPolicy = strategy::andoyer,
    typename Spheroid = srs::spheroid<double>,
    typename CalculationType = void
>
class geographic_cross_track
    : public detail::geographic_cross_track
        <
            FormulaPolicy,
            Spheroid,
            CalculationType,
            false,
            false
        >
{
public :
    explicit geographic_cross_track(Spheroid const& spheroid = Spheroid())
        :
        detail::geographic_cross_track<
                FormulaPolicy,
                Spheroid,
                CalculationType,
                false,
                false
            >(spheroid)
        {}
};

#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS
namespace services
{

//tags
template <typename FormulaPolicy>
struct tag<geographic_cross_track<FormulaPolicy> >
{
    typedef strategy_tag_distance_point_segment type;
};

template
<
        typename FormulaPolicy,
        typename Spheroid
>
struct tag<geographic_cross_track<FormulaPolicy, Spheroid> >
{
    typedef strategy_tag_distance_point_segment type;
};

template
<
        typename FormulaPolicy,
        typename Spheroid,
        typename CalculationType
>
struct tag<geographic_cross_track<FormulaPolicy, Spheroid, CalculationType> >
{
    typedef strategy_tag_distance_point_segment type;
};

template
<
        typename FormulaPolicy,
        typename Spheroid,
        typename CalculationType,
        bool Bisection
>
struct tag<detail::geographic_cross_track<FormulaPolicy, Spheroid, CalculationType, Bisection> >
{
    typedef strategy_tag_distance_point_segment type;
};

//return types
template <typename FormulaPolicy, typename P, typename PS>
struct return_type<geographic_cross_track<FormulaPolicy>, P, PS>
    : geographic_cross_track<FormulaPolicy>::template return_type<P, PS>
{};

template
<
        typename FormulaPolicy,
        typename Spheroid,
        typename P,
        typename PS
>
struct return_type<geographic_cross_track<FormulaPolicy, Spheroid>, P, PS>
    : geographic_cross_track<FormulaPolicy, Spheroid>::template return_type<P, PS>
{};

template
<
        typename FormulaPolicy,
        typename Spheroid,
        typename CalculationType,
        typename P,
        typename PS
>
struct return_type<geographic_cross_track<FormulaPolicy, Spheroid, CalculationType>, P, PS>
    : geographic_cross_track<FormulaPolicy, Spheroid, CalculationType>::template return_type<P, PS>
{};

template
<
        typename FormulaPolicy,
        typename Spheroid,
        typename CalculationType,
        bool Bisection,
        typename P,
        typename PS
>
struct return_type<detail::geographic_cross_track<FormulaPolicy, Spheroid, CalculationType, Bisection>, P, PS>
    : detail::geographic_cross_track<FormulaPolicy, Spheroid, CalculationType, Bisection>::template return_type<P, PS>
{};

//comparable types
template
<
        typename FormulaPolicy,
        typename Spheroid,
        typename CalculationType
>
struct comparable_type<geographic_cross_track<FormulaPolicy, Spheroid, CalculationType> >
{
    typedef geographic_cross_track
        <
            FormulaPolicy, Spheroid, CalculationType
        >  type;
};


template
<
        typename FormulaPolicy,
        typename Spheroid,
        typename CalculationType,
        bool Bisection
>
struct comparable_type<detail::geographic_cross_track<FormulaPolicy, Spheroid, CalculationType, Bisection> >
{
    typedef detail::geographic_cross_track
        <
            FormulaPolicy, Spheroid, CalculationType, Bisection
        >  type;
};

template
<
        typename FormulaPolicy,
        typename Spheroid,
        typename CalculationType
>
struct get_comparable<geographic_cross_track<FormulaPolicy, Spheroid, CalculationType> >
{
public :
    static inline geographic_cross_track<FormulaPolicy, Spheroid, CalculationType>
    apply(geographic_cross_track<FormulaPolicy, Spheroid, CalculationType> const& strategy)
    {
        return strategy;
    }
};

template
<
        typename FormulaPolicy,
        typename Spheroid,
        typename CalculationType,
        bool Bisection
>
struct get_comparable<detail::geographic_cross_track<FormulaPolicy, Spheroid, CalculationType, Bisection> >
{
public :
    static inline detail::geographic_cross_track<FormulaPolicy, Spheroid, CalculationType, Bisection>
    apply(detail::geographic_cross_track<FormulaPolicy, Spheroid, CalculationType, Bisection> const& strategy)
    {
        return strategy;
    }
};


template
<
    typename FormulaPolicy,
    typename P,
    typename PS
>
struct result_from_distance<geographic_cross_track<FormulaPolicy>, P, PS>
{
private :
    typedef typename geographic_cross_track
        <
            FormulaPolicy
        >::template return_type<P, PS>::type return_type;
public :
    template <typename T>
    static inline return_type
    apply(geographic_cross_track<FormulaPolicy> const& , T const& distance)
    {
        return distance;
    }
};

template
<
    typename FormulaPolicy,
    typename Spheroid,
    typename CalculationType,
    typename P,
    typename PS
>
struct result_from_distance<geographic_cross_track<FormulaPolicy, Spheroid, CalculationType>, P, PS>
{
private :
    typedef typename geographic_cross_track
        <
            FormulaPolicy, Spheroid, CalculationType
        >::template return_type<P, PS>::type return_type;
public :
    template <typename T>
    static inline return_type
    apply(geographic_cross_track<FormulaPolicy, Spheroid, CalculationType> const& , T const& distance)
    {
        return distance;
    }
};


template <typename Point, typename PointOfSegment>
struct default_strategy
    <
        point_tag, segment_tag, Point, PointOfSegment,
        geographic_tag, geographic_tag
    >
{
    typedef geographic_cross_track<> type;
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
