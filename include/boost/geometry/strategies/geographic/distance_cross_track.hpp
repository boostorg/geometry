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
\brief Strategy functor for distance point to segment calculation on ellipsoid
\ingroup strategies
\details Class which calculates the distance of a point to a segment, for points
on the ellipsoid
\see https://arxiv.org/abs/1102.1215
\tparam FormulaPolicy underlying point-point distance strategy
\tparam Spheroid is the spheroidal model used
\tparam CalculationType \tparam_calculation
*/
template
<
    typename FormulaPolicy = strategy::andoyer,
    typename Spheroid = srs::spheroid<double>,
    typename CalculationType = void
>
class geographic_cross_track
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

    struct distance_strategy
    {
        typedef geographic<FormulaPolicy, Spheroid, CalculationType> type;
    };

    inline typename distance_strategy::type get_distance_strategy() const
    {
        typedef typename distance_strategy::type distance_type;
        return distance_type(m_spheroid);
    }

    explicit geographic_cross_track(Spheroid const& spheroid = Spheroid())
        : m_spheroid(spheroid)
    {}

    template <typename Point, typename PointOfSegment>
    inline typename return_type<Point, PointOfSegment>::type
    apply(Point const& p, PointOfSegment const& sp1, PointOfSegment const& sp2) const
    {
        typedef typename coordinate_system<Point>::type::units units_type;

        typedef typename return_type<Point, PointOfSegment>::type CT;

        return (geometry::formula::distance_point_segment
                <
                    CT,
                    units_type,
                    FormulaPolicy
                >::apply(get<0>(sp1), get<1>(sp1),
                         get<0>(sp2), get<1>(sp2),
                         get<0>(p), get<1>(p),
                         m_spheroid)).distance;
    }

private :
    Spheroid m_spheroid;

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
        typename CalculationType
>
struct get_comparable<geographic_cross_track<FormulaPolicy, Spheroid, CalculationType> >
{
    typedef typename comparable_type
        <
            geographic_cross_track<FormulaPolicy, Spheroid, CalculationType>
        >::type comparable_type;
public :
    static inline comparable_type
    apply(geographic_cross_track<FormulaPolicy, Spheroid, CalculationType> const& strategy)
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
