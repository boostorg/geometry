// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2014 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2014-2019.
// Modifications copyright (c) 2014-2019, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle
// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_SPHERICAL_DISTANCE_CROSS_TRACK_HPP
#define BOOST_GEOMETRY_STRATEGIES_SPHERICAL_DISTANCE_CROSS_TRACK_HPP

#include <algorithm>

#include <boost/config.hpp>
#include <boost/concept_check.hpp>
#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_void.hpp>

#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/radian_access.hpp>
#include <boost/geometry/core/tags.hpp>

#include <boost/geometry/formulas/spherical.hpp>

#include <boost/geometry/strategies/distance.hpp>
#include <boost/geometry/strategies/concepts/distance_concept.hpp>
#include <boost/geometry/strategies/spherical/distance_haversine.hpp>
#include <boost/geometry/strategies/spherical/point_in_point.hpp>
#include <boost/geometry/strategies/spherical/intersection.hpp>

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


namespace comparable
{


template
<
    typename CalculationType = void,
    typename Strategy = comparable::haversine<double, CalculationType>
>
class cross_track
{
public :
    typedef within::spherical_point_point equals_point_point_strategy_type;

    typedef intersection::spherical_segments
        <
            CalculationType
        > relate_segment_segment_strategy_type;

    static inline relate_segment_segment_strategy_type get_relate_segment_segment_strategy()
    {
        return relate_segment_segment_strategy_type();
    }

    typedef within::spherical_winding
        <
            void, void, CalculationType
        > point_in_geometry_strategy_type;

    static inline point_in_geometry_strategy_type get_point_in_geometry_strategy()
    {
        return point_in_geometry_strategy_type();
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

    typedef typename Strategy::radius_type radius_type;

    inline cross_track()
    {}

    explicit inline cross_track(typename Strategy::radius_type const& r)
        : m_strategy(r)
    {}

    inline cross_track(Strategy const& s)
        : m_strategy(s)
    {}

    // It might be useful in the future
    // to overload constructor with strategy info.
    // crosstrack(...) {}


    template <typename Point, typename PointOfSegment>
    inline typename return_type<Point, PointOfSegment>::type
    apply(Point const& p, PointOfSegment const& sp1, PointOfSegment const& sp2) const
    {

#if !defined(BOOST_MSVC)
        BOOST_CONCEPT_ASSERT
            (
                (concepts::PointDistanceStrategy<Strategy, Point, PointOfSegment>)
            );
#endif

        typedef typename return_type<Point, PointOfSegment>::type return_type;

        Strategy distance_strategy = Strategy(m_strategy);
        typedef geometry::formula::comparable_spherical_point_segment_distance
                <return_type, false> spherical_ps_distance;
        typename spherical_ps_distance::result_type res = spherical_ps_distance()
                  .apply(p, sp1, sp2,
                         distance::services::get_comparable<Strategy>
                                 ::apply(distance_strategy));
        return res.distance;
    }

    template <typename ResultType>
    inline ResultType
    apply(ResultType comparable_result) const
    {
        return comparable_result;
    }

    template <typename T1, typename T2, typename T3>
    inline radius_type vertical_or_meridian(T1 const& lat1,
                                            T2 const& lat2,
                                            T3 const& lon) const
    {
        boost::ignore_unused(lon);
        //return m_strategy.radius() * math::abs(lat1 - lat2);
        return math::hav(lat1 - lat2);
    }

    inline typename Strategy::radius_type radius() const
    { return m_strategy.radius(); }

private :
    Strategy m_strategy;
};

} // namespace comparable


/*!
\brief Strategy functor for distance point to segment calculation
\ingroup strategies
\details Class which calculates the distance of a point to a segment, for points on a sphere or globe
\see http://williams.best.vwh.net/avform.htm
\tparam CalculationType \tparam_calculation
\tparam Strategy underlying point-point distance strategy, defaults to haversine

\qbk{
[heading See also]
[link geometry.reference.algorithms.distance.distance_3_with_strategy distance (with strategy)]
}

*/
template
<
    typename CalculationType = void,
    typename Strategy = haversine<double, CalculationType>
>
class cross_track
{
public :
    typedef within::spherical_point_point equals_point_point_strategy_type;

    typedef intersection::spherical_segments
        <
            CalculationType
        > relate_segment_segment_strategy_type;

    static inline relate_segment_segment_strategy_type get_relate_segment_segment_strategy()
    {
        return relate_segment_segment_strategy_type();
    }

    typedef within::spherical_winding
        <
            void, void, CalculationType
        > point_in_geometry_strategy_type;

    static inline point_in_geometry_strategy_type get_point_in_geometry_strategy()
    {
        return point_in_geometry_strategy_type();
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

    typedef typename Strategy::radius_type radius_type;

    inline cross_track()
    {}

    explicit inline cross_track(typename Strategy::radius_type const& r)
        : m_strategy(r)
    {}

    inline cross_track(Strategy const& s)
        : m_strategy(s)
    {}

    // It might be useful in the future
    // to overload constructor with strategy info.
    // crosstrack(...) {}


    template <typename Point, typename PointOfSegment>
    inline typename return_type<Point, PointOfSegment>::type
    apply(Point const& p, PointOfSegment const& sp1, PointOfSegment const& sp2) const
    {

#if !defined(BOOST_MSVC)
        BOOST_CONCEPT_ASSERT
            (
                (concepts::PointDistanceStrategy<Strategy, Point, PointOfSegment>)
            );
#endif
        typedef cross_track<CalculationType, Strategy> this_type;

        typedef typename services::comparable_type
            <
                this_type
            >::type comparable_type;

        comparable_type cstrategy
            = services::get_comparable<this_type>::apply(m_strategy);

        return apply(cstrategy.apply(p, sp1, sp2));
    }

    template <typename ResultType>
    inline ResultType
    apply(ResultType comparable_result) const
    {
        return ResultType(2.0) * asin(math::sqrt(comparable_result)) * radius();
    }

    template <typename T1, typename T2, typename T3>
    inline radius_type vertical_or_meridian(T1 const& lat1,
                                            T2 const& lat2,
                                            T3 const& lon) const
    {
        boost::ignore_unused(lon);
        return m_strategy.radius() * math::abs(lat1 - lat2);
    }

    inline typename Strategy::radius_type radius() const
    { return m_strategy.radius(); }

private :

    Strategy m_strategy;
};



#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS
namespace services
{

template <typename CalculationType, typename Strategy>
struct tag<cross_track<CalculationType, Strategy> >
{
    typedef strategy_tag_distance_point_segment type;
};


template <typename CalculationType, typename Strategy, typename P, typename PS>
struct return_type<cross_track<CalculationType, Strategy>, P, PS>
    : cross_track<CalculationType, Strategy>::template return_type<P, PS>
{};


template <typename CalculationType, typename Strategy>
struct comparable_type<cross_track<CalculationType, Strategy> >
{
    typedef comparable::cross_track
        <
            CalculationType, typename comparable_type<Strategy>::type
        >  type;
};


template
<
    typename CalculationType,
    typename Strategy
>
struct get_comparable<cross_track<CalculationType, Strategy> >
{
    typedef typename comparable_type
        <
            cross_track<CalculationType, Strategy>
        >::type comparable_type;
public :
    static inline comparable_type
    apply(cross_track<CalculationType, Strategy> const& strategy)
    {
        return comparable_type(strategy.radius());
    }
};


template
<
    typename CalculationType,
    typename Strategy,
    typename P,
    typename PS
>
struct result_from_distance<cross_track<CalculationType, Strategy>, P, PS>
{
private :
    typedef typename cross_track
        <
            CalculationType, Strategy
        >::template return_type<P, PS>::type return_type;
public :
    template <typename T>
    static inline return_type
    apply(cross_track<CalculationType, Strategy> const& , T const& distance)
    {
        return distance;
    }
};


// Specializations for comparable::cross_track
template <typename RadiusType, typename CalculationType>
struct tag<comparable::cross_track<RadiusType, CalculationType> >
{
    typedef strategy_tag_distance_point_segment type;
};


template
<
    typename RadiusType,
    typename CalculationType,
    typename P,
    typename PS
>
struct return_type<comparable::cross_track<RadiusType, CalculationType>, P, PS>
    : comparable::cross_track
        <
            RadiusType, CalculationType
        >::template return_type<P, PS>
{};


template <typename RadiusType, typename CalculationType>
struct comparable_type<comparable::cross_track<RadiusType, CalculationType> >
{
    typedef comparable::cross_track<RadiusType, CalculationType> type;
};


template <typename RadiusType, typename CalculationType>
struct get_comparable<comparable::cross_track<RadiusType, CalculationType> >
{
private :
    typedef comparable::cross_track<RadiusType, CalculationType> this_type;
public :
    static inline this_type apply(this_type const& input)
    {
        return input;
    }
};


template
<
    typename RadiusType,
    typename CalculationType,
    typename P,
    typename PS
>
struct result_from_distance
    <
        comparable::cross_track<RadiusType, CalculationType>, P, PS
    >
{
private :
    typedef comparable::cross_track<RadiusType, CalculationType> strategy_type;
    typedef typename return_type<strategy_type, P, PS>::type return_type;
public :
    template <typename T>
    static inline return_type apply(strategy_type const& strategy,
                                    T const& distance)
    {
        return_type const s
            = sin( (distance / strategy.radius()) / return_type(2.0) );
        return s * s;
    }
};



/*

TODO:  spherical polar coordinate system requires "get_as_radian_equatorial<>"

template <typename Point, typename PointOfSegment, typename Strategy>
struct default_strategy
    <
        segment_tag, Point, PointOfSegment,
        spherical_polar_tag, spherical_polar_tag,
        Strategy
    >
{
    typedef cross_track
        <
            void,
            typename boost::mpl::if_
                <
                    boost::is_void<Strategy>,
                    typename default_strategy
                        <
                            point_tag, Point, PointOfSegment,
                            spherical_polar_tag, spherical_polar_tag
                        >::type,
                    Strategy
                >::type
        > type;
};
*/

template <typename Point, typename PointOfSegment, typename Strategy>
struct default_strategy
    <
        point_tag, segment_tag, Point, PointOfSegment,
        spherical_equatorial_tag, spherical_equatorial_tag,
        Strategy
    >
{
    typedef cross_track
        <
            void,
            typename boost::mpl::if_
                <
                    boost::is_void<Strategy>,
                    typename default_strategy
                        <
                            point_tag, point_tag, Point, PointOfSegment,
                            spherical_equatorial_tag, spherical_equatorial_tag
                        >::type,
                    Strategy
                >::type
        > type;
};


template <typename PointOfSegment, typename Point, typename Strategy>
struct default_strategy
    <
        segment_tag, point_tag, PointOfSegment, Point,
        spherical_equatorial_tag, spherical_equatorial_tag,
        Strategy
    >
{
    typedef typename default_strategy
        <
            point_tag, segment_tag, Point, PointOfSegment,
            spherical_equatorial_tag, spherical_equatorial_tag,
            Strategy
        >::type type;
};


} // namespace services
#endif // DOXYGEN_NO_STRATEGY_SPECIALIZATIONS

}} // namespace strategy::distance

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_SPHERICAL_DISTANCE_CROSS_TRACK_HPP
