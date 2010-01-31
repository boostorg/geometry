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

#include <boost/geometry/util/math.hpp>
//#include <boost/geometry/util/write_dsv.hpp>



namespace boost { namespace geometry
{

namespace strategy { namespace distance {

/*!
    \brief Strategy functor for distance point to segment calculation
    \ingroup distance
    \details Class which calculates the distance of a point to a segment, using latlong points
    \see http://williams.best.vwh.net/avform.htm
    \tparam P point type
    \tparam S segment type
*/
template <typename Point, typename PointOfSegment>
class cross_track
{
public :
    typedef double return_type;
    typedef Point point_type;
    typedef PointOfSegment segment_point_type;

    typedef typename strategy_distance
        <
            typename geometry::cs_tag<Point>::type,
            typename geometry::cs_tag<Point>::type,
            Point, Point
        >::type point_strategy_type;

    BOOST_CONCEPT_ASSERT
        (
            (geometry::concept::PointDistanceStrategy<point_strategy_type>)
        );



    inline cross_track(double r = 1.0)
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
        double d1 = m_strategy.apply(sp1, p);

        // Actually, calculation of d2 not necessary if we know that the projected point is on the great circle...
        double d2 = m_strategy.apply(sp2, p);

        double crs_AD = course(sp1, p);
        double crs_AB = course(sp1, sp2);
        double XTD = std::abs(asin(sin(d1) * sin(crs_AD - crs_AB)));

//std::cout << "Course " << dsv(sp1) << " to " << dsv(p) << " " << crs_AD * geometry::math::r2d << std::endl;
//std::cout << "Course " << dsv(sp1) << " to " << dsv(sp2) << " " << crs_AB * geometry::math::r2d << std::endl;
//std::cout << "XTD: " << (XTD * 6373.0) << " d1: " <<  (d1 * 6373.0)  << " d2: " <<  (d2 * 6373.0)  << std::endl;


        // Return shortest distance, either to projected point on segment sp1-sp2, or to sp1, or to sp2
        return return_type(m_radius * (std::min)((std::min)(d1, d2), XTD));
    }

private :
    double m_radius;

    // Point-point distances are calculated in radians, on the unit sphere
    point_strategy_type m_strategy;

    /// Calculate course (bearing) between two points. Might be moved to a "course formula" ...
    inline double course(Point const& p1, Point const& p2) const
    {
        // http://williams.best.vwh.net/avform.htm#Crs
        double dlon = get_as_radian<0>(p2) - get_as_radian<0>(p1);
        double cos_p2lat = cos(get_as_radian<1>(p2));

        // "An alternative formula, not requiring the pre-computation of d"
        return atan2(sin(dlon) * cos_p2lat,
            cos(get_as_radian<1>(p1)) * sin(get_as_radian<1>(p2))
            - sin(get_as_radian<1>(p1)) * cos_p2lat * cos(dlon));
    }

};


}} // namespace strategy::distance



#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS


template <typename Point, typename Segment>
struct strategy_distance_segment<spherical_tag, spherical_tag, Point, Segment>
{
    typedef strategy::distance::cross_track<Point, Segment> type;
};


// Use this point-segment for geographic as well. TODO: change this, extension!
template <typename Point, typename Segment>
struct strategy_distance_segment<geographic_tag, geographic_tag, Point, Segment>
{
    typedef strategy::distance::cross_track<Point, Segment> type;
};


template <typename Point, typename Segment>
struct strategy_tag<strategy::distance::cross_track<Point, Segment> >
{
    typedef strategy_tag_distance_point_segment type;
};



#endif






}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_STRATEGIES_SPHERICAL_DISTANCE_CROSS_TRACK_HPP
