// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_SPHERICAL_DISTANCE_CROSS_TRACK_HPP
#define BOOST_GEOMETRY_STRATEGIES_SPHERICAL_DISTANCE_CROSS_TRACK_HPP

#include <boost/concept/requires.hpp>


#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/radian_access.hpp>


#include <boost/geometry/strategies/distance.hpp>
#include <boost/geometry/strategies/concepts/distance_concept.hpp>

#include <boost/geometry/util/promote_floating_point.hpp>
#include <boost/geometry/util/math.hpp>
//#include <boost/geometry/util/write_dsv.hpp>




namespace boost { namespace geometry
{

namespace strategy { namespace distance
{

/*!
    \brief Strategy functor for distance point to segment calculation
    \ingroup distance
    \details Class which calculates the distance of a point to a segment, using latlong points
    \see http://williams.best.vwh.net/avform.htm
    \tparam P point type
    \tparam S segment type
*/
template
<
    typename Point,
    typename PointOfSegment = Point,
    typename CalculationType = void,
    typename Strategy = typename default_distance_strategy<Point>::type
>
class cross_track
{
public :
    typedef typename promote_floating_point
        <
            typename select_calculation_type
                <
                    Point,
                    PointOfSegment,
                    CalculationType
                >::type
        >::type return_type;

    typedef Point point_type;
    typedef PointOfSegment segment_point_type;

    typedef Strategy point_strategy_type;

    BOOST_CONCEPT_ASSERT
        (
            (geometry::concept::PointDistanceStrategy<point_strategy_type>)
        );



    inline cross_track(return_type const& r = 1.0)
        : m_radius(r)
        , m_strategy(1.0) // Keep this 1.0 and not r
    {}


    // It might be useful in the future
    // to overload constructor with strategy info.
    // crosstrack(...) {}


    inline return_type apply(Point const& p,
                PointOfSegment const& sp1, PointOfSegment const& sp2) const
    {
        // http://williams.best.vwh.net/avform.htm#XTE
        return_type d1 = m_strategy.apply(sp1, p);

        // Actually, calculation of d2 not necessary if we know that the projected point is on the great circle...
        return_type d2 = m_strategy.apply(sp2, p);

        return_type crs_AD = course(sp1, p);
        return_type crs_AB = course(sp1, sp2);
        return_type XTD = abs(asin(sin(d1) * sin(crs_AD - crs_AB)));

//std::cout << "Course " << dsv(sp1) << " to " << dsv(p) << " " << crs_AD * geometry::math::r2d << std::endl;
//std::cout << "Course " << dsv(sp1) << " to " << dsv(sp2) << " " << crs_AB * geometry::math::r2d << std::endl;
//std::cout << "XTD: " << (XTD * 6373.0) << " d1: " <<  (d1 * 6373.0)  << " d2: " <<  (d2 * 6373.0)  << std::endl;


        // Return shortest distance, either to projected point on segment sp1-sp2, or to sp1, or to sp2
        return return_type(m_radius * (std::min)((std::min)(d1, d2), XTD));
    }

    inline return_type radius() const { return m_radius; }

private :
    return_type m_radius;

    // Point-point distances are calculated in radians, on the unit sphere
    point_strategy_type m_strategy;

    /// Calculate course (bearing) between two points. Might be moved to a "course formula" ...
    inline return_type course(Point const& p1, Point const& p2) const
    {
        // http://williams.best.vwh.net/avform.htm#Crs
        return_type dlon = get_as_radian<0>(p2) - get_as_radian<0>(p1);
        return_type cos_p2lat = cos(get_as_radian<1>(p2));

        // "An alternative formula, not requiring the pre-computation of d"
        return atan2(sin(dlon) * cos_p2lat,
            cos(get_as_radian<1>(p1)) * sin(get_as_radian<1>(p2))
            - sin(get_as_radian<1>(p1)) * cos_p2lat * cos(dlon));
    }

};



#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS
namespace services
{

template <typename Point, typename PointOfSegment>
struct tag<cross_track<Point, PointOfSegment> >
{
    typedef strategy_tag_distance_point_segment type;
};


template
<
    typename Point,
    typename PointOfSegment,
    typename P,
    typename PS
>
struct similar_type<cross_track<Point, PointOfSegment>, P, PS>
{
    typedef cross_track<Point, PointOfSegment> type;
};


template
<
    typename Point,
    typename PointOfSegment,
    typename P,
    typename PS
>
struct get_similar<cross_track<Point, PointOfSegment>, P, PS>
{
    static inline typename similar_type
        <
            cross_track<Point, PointOfSegment>, P, PS
        >::type apply(cross_track<Point, PointOfSegment> const& strategy)
    {
        return cross_track<P, PS>(strategy.radius());
    }
};


template <typename Point, typename PointOfSegment>
struct comparable_type<cross_track<Point, PointOfSegment> >
{
    // Comparable type is here just the strategy
    typedef typename similar_type<cross_track<Point, PointOfSegment>, Point, PointOfSegment>::type type;
};


template <typename Point, typename PointOfSegment>
struct get_comparable<cross_track<Point, PointOfSegment> >
{
    typedef typename comparable_type
        <
            cross_track<Point, PointOfSegment>
        >::type comparable_type;
public :
    static inline comparable_type apply(cross_track<Point, PointOfSegment> const& strategy)
    {
        return comparable_type(strategy.radius());
    }
};


template <typename Point, typename PointOfSegment>
struct result_from_distance<cross_track<Point, PointOfSegment> >
{
private :
    typedef typename cross_track<Point, PointOfSegment>::return_type return_type;
public :
    template <typename T>
    static inline return_type apply(cross_track<Point, PointOfSegment> const& , T const& distance)
    {
        return distance;
    }
};


} // namespace services
#endif // DOXYGEN_NO_STRATEGY_SPECIALIZATIONS


}} // namespace strategy::distance



#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS


template <typename Point, typename Segment>
struct strategy_distance_segment<spherical_tag, spherical_tag, Point, Segment>
{
    typedef strategy::distance::cross_track<Point, Segment> type;
};


// Use this point-segment for geographic as well. TODO: change this, extension!
template <typename Point, typename PointOfSegment>
struct strategy_distance_segment<geographic_tag, geographic_tag, Point, PointOfSegment>
{
    typedef strategy::distance::cross_track<Point, PointOfSegment> type;
};


template <typename Point, typename Segment>
struct strategy_tag<strategy::distance::cross_track<Point, Segment> >
{
    typedef strategy_tag_distance_point_segment type;
};


#endif


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_STRATEGIES_SPHERICAL_DISTANCE_CROSS_TRACK_HPP
