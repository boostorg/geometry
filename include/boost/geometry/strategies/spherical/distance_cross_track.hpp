// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_SPHERICAL_DISTANCE_CROSS_TRACK_HPP
#define BOOST_GEOMETRY_STRATEGIES_SPHERICAL_DISTANCE_CROSS_TRACK_HPP


#include <boost/config.hpp>
#include <boost/concept_check.hpp>
#include <boost/mpl/if.hpp>
#include <boost/type_traits.hpp>

#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/radian_access.hpp>

#include <boost/geometry/algorithms/detail/course.hpp>

#include <boost/geometry/strategies/distance.hpp>
#include <boost/geometry/strategies/concepts/distance_concept.hpp>
#include <boost/geometry/strategies/spherical/distance_haversine.hpp>

#include <boost/geometry/util/promote_floating_point.hpp>
#include <boost/geometry/util/math.hpp>

#ifdef BOOST_GEOMETRY_DEBUG_CROSS_TRACK
#  include <boost/geometry/io/dsv/write.hpp>
#endif



namespace boost { namespace geometry
{

namespace strategy { namespace distance
{


namespace comparable
{

/*
  Mathematical analysis
  ---------------------
  Identities used:
  sin(2*x) = 2*sin(x)*cos(x)
  cos(asin(x)) = sqrt(1-x^2)

  Observation:
  The distance d1 need when the projection of the point is is within the
  segment must be the true distance. However, comparable::haversine<>
  returns a comparable distance instead of the one needed.
  To remedy this, we compute implicitly compute what is
  needed. More precisely, we need to compute sin(true_d1):

  sin(true_d1) = sin(2 * asin(sqrt(d1)))
               = 2 * sin(asin(sqrt(d1)) * cos(asin(sqrt(d1)))
               = 2 * sqrt(d1) * sqrt(1-(sqrt(d1))^2)
               = 2 * sqrt(d1 - d1 * d1)
  This relation is used below.

  Goal: find an a such that:
  2 * R * asin(sqrt(a)) == R * asin(2 * sqrt(b-b^2) * sin(c))

  Analysis:
      2 * R * asin(sqrt(a)) == R * asin(2 * sqrt(b-b^2) * sin(c))
  <=> 2 * asin(sqrt(a)) == asin(sqrt(b-b^2) * sin(c))
  <=> sin(2 * asin(sqrt(a))) == 2 * sqrt(b-b^2) * sin(c)
  <=> 2 * sin(asin(sqrt(a))) * cos(asin(sqrt(a))) == 2 * sqrt(b-b^2) * sin(c)
  <=> 2 * sqrt(a) * sqrt(1-a) == 2 * sqrt(b-b^2) * sin(c)
  <=> sqrt(a) * sqrt(1-a) == sqrt(b-b^2) * sin(c)
  <=> sqrt(a-a^2) == sqrt(b-b^2) * sin(c)
  <=> a-a^2 == (b-b^2) * (sin(c))^2

  Consider the quadratic equation: x^2-x+p^2 == 0
  Its discriminant is: d = 1 - 4 * p^2 = 1 - 4 * (b-b^2) * (sin(c))^2

  So solutions are:
  x_{1,2} = (1 + sqrt(d))/2, (1 - sqrt(d))/2

  Which one to choose?
  My understanding is that "a" is the distance of the third
  point to its closest point on the segment. The two
  different values for "a" correspond to the lengths of the two
  arcs delimited by these to points.
  So, my guess is the value we want is the smallest root (unit 
  tests confirm this choice).

  So the answer is:
  a = ( 1 - sqrt(1 - 4 * (b-b^2) * (sin(c))^2) ) / 2;


  Complexity Analysis
  ------------------
  (A) The distance is realized between the point and an
      endpoint of the segment

      Gains:
      Since we are using comparable::haversine<> which is called
      3 times, we gain:
      -> 3 calls to sqrt
      -> 3 calls to asin
      -> 6 multiplications

      Loses: None

      So the net gain is:
      -> 6 function calls (sqrt/asin)
      -> 6 arithmetic operations

      If we use comparable::cross_track<> to compute
      cross_track<> we need to account for a call to sqrt, a call
      to asin and 2 multiplications. In this case the net gain is:
      -> 4 function calls (sqrt/asin)
      -> 4 arithmetic operations


  (B) The distance is realized between the point and an
      interior point of the segment

      Gains:
      Since we are using comparable::haversine<> which is called
      3 times, we gain:
      -> 3 calls to sqrt
      -> 3 calls to asin
      -> 6 multiplications
      Also we gain the operations used to compute XTD:
      -> 2 calls to sin
      -> 1 call to asin
      -> 1 call to abs
      -> 2 multiplications
      -> 1 division
      So the total gains are:
      -> 9 calls to sqrt/sin/asin
      -> 1 call to abs
      -> 8 multiplications
      -> 1 division

      Loses:
      To compute a distance compatible with comparable::haversine<>
      we need to perform a few more operations, namely:
      -> 1 call to sin
      -> 1 call to sqrt
      -> 2 multiplications
      -> 1 addition
      -> 2 subtractions

      So roughly speaking the net gain is:
      -> 8 fewer function calls and 4 fewer arithmetic operations

      If we were to implement cross_track directly from the
      comparable version (much like what haversine<> does using
      comparable::haversine<>) we need additionally
      -> 2 function calls (asin/sqrt)
      -> 2 multiplications

      So it seems like it pays off to re-implement cross_track<> to
      use comparable::cross_track<>; in this case the net gain
      would be:
      -> 6 function calls
      -> 2 arithmetic operations
*/

template
<
    typename CalculationType = void,
    typename Strategy = comparable::haversine<double, CalculationType>
>
class cross_track
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
                (concept::PointDistanceStrategy<Strategy, Point, PointOfSegment>)
            );
#endif

        typedef typename return_type<Point, PointOfSegment>::type return_type;

        // http://williams.best.vwh.net/avform.htm#XTE
        return_type d1 = m_strategy.apply(sp1, p);
        return_type d3 = m_strategy.apply(sp1, sp2);

        if (geometry::math::equals(d3, 0.0))
        {
            // "Degenerate" segment, return either d1 or d2
            return d1;
        }

        return_type d2 = m_strategy.apply(sp2, p);

        return_type crs_AD = geometry::detail::course<return_type>(sp1, p);
        return_type crs_AB = geometry::detail::course<return_type>(sp1, sp2);
        return_type crs_BA = crs_AB - geometry::math::pi<return_type>();
        return_type crs_BD = geometry::detail::course<return_type>(sp2, p);
        return_type d_crs1 = crs_AD - crs_AB;
        return_type d_crs2 = crs_BD - crs_BA;

        // d1, d2, d3 are in principle not needed, only the sign matters
        return_type projection1 = cos( d_crs1 ) * d1 / d3;
        return_type projection2 = cos( d_crs2 ) * d2 / d3;

        if (projection1 > 0.0 && projection2 > 0.0)
        {
#ifdef BOOST_GEOMETRY_DEBUG_CROSS_TRACK
            std::cout << "Projection ON the segment" << std::endl;
#endif
            return_type const half(0.5);
            return_type const quarter(0.25);

            return_type sin_d_crs1 = sin(d_crs1);
            /*
              This is the straightforward obvious way to continue:
              
              return_type discriminant
                  = 1.0 - 4.0 * (d1 - d1 * d1) * sin_d_crs1 * sin_d_crs1;
              return 0.5 - 0.5 * math::sqrt(discriminant);
            
              Below we optimize the number of arithmetic operations:
            */
            return_type d1_x_sin = d1 * sin_d_crs1;
            return
                half - math::sqrt(quarter + d1_x_sin * (d1_x_sin - sin_d_crs1));
        }
        else
        {
#ifdef BOOST_GEOMETRY_DEBUG_CROSS_TRACK
            std::cout << "Projection OUTSIDE the segment" << std::endl;
#endif

            // Return shortest distance, project either on point sp1 or sp2
            return return_type( (std::min)( d1 , d2 ) );
        }
    }

    inline typename Strategy::radius_type radius() const
    { return m_strategy.radius(); }

private :
    Strategy m_strategy;
};

}// namespace comparable

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
                (concept::PointDistanceStrategy<Strategy, Point, PointOfSegment>)
            );
#endif

        typedef typename return_type<Point, PointOfSegment>::type return_type;

#ifdef BOOST_GEOMETRY_USE_OLD_CROSS_TRACK_STRATEGY
        // http://williams.best.vwh.net/avform.htm#XTE
        return_type d1 = m_strategy.apply(sp1, p);
        return_type d3 = m_strategy.apply(sp1, sp2);

        if (geometry::math::equals(d3, 0.0))
        {
            // "Degenerate" segment, return either d1 or d2
            return d1;
        }

        return_type d2 = m_strategy.apply(sp2, p);

        return_type crs_AD = geometry::detail::course<return_type>(sp1, p);
        return_type crs_AB = geometry::detail::course<return_type>(sp1, sp2);
        return_type crs_BA = crs_AB - geometry::math::pi<return_type>();
        return_type crs_BD = geometry::detail::course<return_type>(sp2, p);
        return_type d_crs1 = crs_AD - crs_AB;
        return_type d_crs2 = crs_BD - crs_BA;

        // d1, d2, d3 are in principle not needed, only the sign matters
        return_type projection1 = cos( d_crs1 ) * d1 / d3;
        return_type projection2 = cos( d_crs2 ) * d2 / d3;

#ifdef BOOST_GEOMETRY_DEBUG_CROSS_TRACK
        std::cout << "Course " << dsv(sp1) << " to " << dsv(p) << " " << crs_AD * geometry::math::r2d << std::endl;
        std::cout << "Course " << dsv(sp1) << " to " << dsv(sp2) << " " << crs_AB * geometry::math::r2d << std::endl;
        std::cout << "Course " << dsv(sp2) << " to " << dsv(p) << " " << crs_BD * geometry::math::r2d << std::endl;
        std::cout << "Projection AD-AB " << projection1 << " : " << d_crs1 * geometry::math::r2d << std::endl;
        std::cout << "Projection BD-BA " << projection2 << " : " << d_crs2 * geometry::math::r2d << std::endl;
#endif

        if (projection1 > 0.0 && projection2 > 0.0)
        {
            return_type XTD = radius() * geometry::math::abs( asin( sin( d1 / radius() ) * sin( d_crs1 ) ));

 #ifdef BOOST_GEOMETRY_DEBUG_CROSS_TRACK
            std::cout << "Projection ON the segment" << std::endl;
            std::cout << "XTD: " << XTD << " d1: " <<  d1  << " d2: " <<  d2  << std::endl;
#endif

            // Return shortest distance, projected point on segment sp1-sp2
            return return_type(XTD);
        }
        else
        {
#ifdef BOOST_GEOMETRY_DEBUG_CROSS_TRACK
            std::cout << "Projection OUTSIDE the segment" << std::endl;
#endif

            // Return shortest distance, project either on point sp1 or sp2
            return return_type( (std::min)( d1 , d2 ) );
        }
#else // BOOST_GEOMETRY_USE_OLD_CROSS_TRACK_STRATEGY

        typedef cross_track<CalculationType, Strategy> this_type;

        typedef typename services::comparable_type
            <
                this_type
            >::type comparable_type;

        comparable_type cstrategy
            = services::get_comparable<this_type>::apply(m_strategy);

        return_type const a = cstrategy.apply(p, sp1, sp2);
        return_type const c = return_type(2.0) * asin(math::sqrt(a));
        return c * radius();
#endif // BOOST_GEOMETRY_USE_OLD_CROSS_TRACK_STRATEGY
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
#if 0
    // There is no shortcut, so the strategy itself is its comparable type
    typedef cross_track<CalculationType, Strategy>  type;
#else
    typedef comparable::cross_track
        <
            CalculationType, typename comparable_type<Strategy>::type
        >  type;
#endif
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
    static inline comparable_type apply(cross_track<CalculationType, Strategy> const& strategy)
    {
        return comparable_type(strategy.radius());
    }
};


template
<
    typename CalculationType,
    typename Strategy,
    typename P, typename PS
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
    static inline return_type apply(cross_track<CalculationType, Strategy> const& , T const& distance)
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


template <typename RadiusType, typename CalculationType, typename P1, typename P2>
struct return_type<comparable::cross_track<RadiusType, CalculationType>, P1, P2>
    : comparable::cross_track<RadiusType, CalculationType>::template return_type<P1, P2>
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


template <typename RadiusType, typename CalculationType, typename P1, typename P2>
struct result_from_distance<comparable::cross_track<RadiusType, CalculationType>, P1, P2>
{
private :
    typedef comparable::cross_track<RadiusType, CalculationType> strategy_type;
    typedef typename return_type<strategy_type, P1, P2>::type return_type;
public :
    template <typename T>
    static inline return_type apply(strategy_type const& strategy, T const& distance)
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


#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS


#endif


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_STRATEGIES_SPHERICAL_DISTANCE_CROSS_TRACK_HPP
