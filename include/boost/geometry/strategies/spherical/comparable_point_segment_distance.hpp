// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2014 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2014-2020.
// Modifications copyright (c) 2014-2020, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle
// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_SPHERICAL_COMPARABLE_POINT_SEGMENT_DISTANCE_HPP
#define BOOST_GEOMETRY_STRATEGIES_SPHERICAL_COMPARABLE_POINT_SEGMENT_DISTANCE_HPP

#include <boost/geometry/formulas/spherical.hpp>

/*
  Given a spherical segment AB and a point D, we are interested in
  computing the distance of D from AB. This is usually known as the
  cross track distance.

  If the projection (along great circles) of the point D lies inside
  the segment AB, then the distance (cross track error) XTD is given
  by the formula (see http://williams.best.vwh.net/avform.htm#XTE):

  XTD = asin( sin(dist_AD) * sin(crs_AD-crs_AB) )

  where dist_AD is the great circle distance between the points A and
  B, and crs_AD, crs_AB is the course (bearing) between the points A,
  D and A, B, respectively.

  If the point D does not project inside the arc AB, then the distance
  of D from AB is the minimum of the two distances dist_AD and dist_BD.

  Our reference implementation for this procedure is listed below
  (this was the old Boost.Geometry implementation of the cross track distance),
  where:
  * The member variable m_strategy is the underlying haversine strategy.
  * p stands for the point D.
  * sp1 stands for the segment endpoint A.
  * sp2 stands for the segment endpoint B.

  ================= reference implementation -- start =================

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
      return_type XTD
          = radius() * math::abs( asin( sin( d1 / radius() ) * sin( d_crs1 ) ));

      // Return shortest distance, projected point on segment sp1-sp2
      return return_type(XTD);
  }
  else
  {
      // Return shortest distance, project either on point sp1 or sp2
      return return_type( (std::min)( d1 , d2 ) );
  }

  ================= reference implementation -- end =================


  Motivation
  ----------
  In what follows we develop a comparable version of the cross track
  distance strategy, that meets the following goals:
  * It is more efficient than the original cross track strategy (less
    operations and less calls to mathematical functions).
  * Distances using the comparable cross track strategy can not only
    be compared with other distances using the same strategy, but also with
    distances computed with the comparable version of the haversine strategy.
  * It can serve as the basis for the computation of the cross track distance,
    as it is more efficient to compute its comparable version and
    transform that to the actual cross track distance, rather than
    follow/use the reference implementation listed above.

  Major idea
  ----------
  The idea here is to use the comparable haversine strategy to compute
  the distances d1, d2 and d3 in the above listing. Once we have done
  that we need also to make sure that instead of returning XTD (as
  computed above) that we return a distance CXTD that is compatible
  with the comparable haversine distance. To achieve this CXTD must satisfy
  the relation:
      XTD = 2 * R * asin( sqrt(XTD) )
  where R is the sphere's radius.

  Below we perform the mathematical analysis that show how to compute CXTD.


  Mathematical analysis
  ---------------------
  Below we use the following trigonometric identities:
      sin(2 * x) = 2 * sin(x) * cos(x)
      cos(asin(x)) = sqrt(1 - x^2)

  Observation:
  The distance d1 needed when the projection of the point D is within the
  segment must be the true distance. However, comparable::haversine<>
  returns a comparable distance instead of the one needed.
  To remedy this, we implicitly compute what is needed.
  More precisely, we need to compute sin(true_d1):

  sin(true_d1) = sin(2 * asin(sqrt(d1)))
               = 2 * sin(asin(sqrt(d1)) * cos(asin(sqrt(d1)))
               = 2 * sqrt(d1) * sqrt(1-(sqrt(d1))^2)
               = 2 * sqrt(d1 - d1 * d1)
  This relation is used below.

  As we mentioned above the goal is to find CXTD (named "a" below for
  brevity) such that ("b" below stands for "d1", and "c" for "d_crs1"):

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

  Consider the quadratic equation: x^2-x+p^2 == 0,
  where p = sqrt(b-b^2) * sin(c); its discriminant is:
      d = 1 - 4 * p^2 = 1 - 4 * (b-b^2) * (sin(c))^2

  The two solutions are:
      a_1 = (1 - sqrt(d)) / 2
      a_2 = (1 + sqrt(d)) / 2

  Which one to choose?
  "a" refers to the distance (on the unit sphere) of D from the
  supporting great circle Circ(A,B) of the segment AB.
  The two different values for "a" correspond to the lengths of the two
  arcs delimited D and the points of intersection of Circ(A,B) and the
  great circle perperdicular to Circ(A,B) passing through D.
  Clearly, the value we want is the smallest among these two distances,
  hence the root we must choose is the smallest root among the two.

  So the answer is:
      CXTD = ( 1 - sqrt(1 - 4 * (b-b^2) * (sin(c))^2) ) / 2

  Therefore, in order to implement the comparable version of the cross
  track strategy we need to:
  (1) Use the comparable version of the haversine strategy instead of
      the non-comparable one.
  (2) Instead of return XTD when D projects inside the segment AB, we
      need to return CXTD, given by the following formula:
          CXTD = ( 1 - sqrt(1 - 4 * (d1-d1^2) * (sin(d_crs1))^2) ) / 2;


  Complexity Analysis
  -------------------
  In the analysis that follows we refer to the actual implementation below.
  In particular, instead of computing CXTD as above, we use the more
  efficient (operation-wise) computation of CXTD shown here:

      return_type sin_d_crs1 = sin(d_crs1);
      return_type d1_x_sin = d1 * sin_d_crs1;
      return_type d = d1_x_sin * (sin_d_crs1 - d1_x_sin);
      return d / (0.5 + math::sqrt(0.25 - d));

  Notice that instead of computing:
      0.5 - 0.5 * sqrt(1 - 4 * d) = 0.5 - sqrt(0.25 - d)
  we use the following formula instead:
      d / (0.5 + sqrt(0.25 - d)).
  This is done for numerical robustness. The expression 0.5 - sqrt(0.25 - x)
  has large numerical errors for values of x close to 0 (if using doubles
  the error start to become large even when d is as large as 0.001).
  To remedy that, we re-write 0.5 - sqrt(0.25 - x) as:
      0.5 - sqrt(0.25 - d)
      = (0.5 - sqrt(0.25 - d) * (0.5 - sqrt(0.25 - d)) / (0.5 + sqrt(0.25 - d)).
  The numerator is the difference of two squares:
      (0.5 - sqrt(0.25 - d) * (0.5 - sqrt(0.25 - d))
      = 0.5^2 - (sqrt(0.25 - d))^ = 0.25 - (0.25 - d) = d,
  which gives the expression we use.

  For the complexity analysis, we distinguish between two cases:
  (A) The distance is realized between the point D and an
      endpoint of the segment AB

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


  (B) The distance is realized between the point D and an
      interior point of the segment AB

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
      -> 1 division
      -> 1 addition
      -> 2 subtractions

      So roughly speaking the net gain is:
      -> 8 fewer function calls and 3 fewer arithmetic operations

      If we were to implement cross_track directly from the
      comparable version (much like what haversine<> does using
      comparable::haversine<>) we need additionally
      -> 2 function calls (asin/sqrt)
      -> 2 multiplications

      So it pays off to re-implement cross_track<> to use
      comparable::cross_track<>; in this case the net gain would be:
      -> 6 function calls
      -> 1 arithmetic operation

   Summary/Conclusion
   ------------------
   Following the mathematical and complexity analysis above, the
   comparable cross track strategy (as implemented below) satisfies
   all the goal mentioned in the beginning:
   * It is more efficient than its non-comparable counter-part.
   * Comparable distances using this new strategy can also be compared
     with comparable distances computed with the comparable haversine
     strategy.
   * It turns out to be more efficient to compute the actual cross
     track distance XTD by first computing CXTD, and then computing
     XTD by means of the formula:
                XTD = 2 * R * asin( sqrt(CXTD) )
*/


/* Closest point to segment using cartesian geometry
 *
        typedef model::point<CT, 3, cs::cartesian> point3d_t;

        point3d_t vp1 = formula::sph_to_cart3d<point3d_t>(sp1);
        point3d_t vp2 = formula::sph_to_cart3d<point3d_t>(sp2);
        point3d_t vp = formula::sph_to_cart3d<point3d_t>(p);

        point3d_t G = cross_product(vp1, vp2);
        point3d_t F = cross_product(G, vp);
        //consider cases for G e.g.
        set<0>(G, get<0>(G)*(-1));
        set<1>(G, get<1>(G)*(-1));
        set<2>(G, get<2>(G)*(-1));
        point3d_t X = cross_product(G, F);
        geometry::detail::vec_normalize(X);
        Point result_p = formula::cart3d_to_sph<Point>(X);

        closest_point_result.lon1 = get_as_radian<0>(p);
        closest_point_result.lat1 = get_as_radian<1>(p);
        closest_point_result.lon2 = get_as_radian<0>(result_p);
        closest_point_result.lat2 = get_as_radian<1>(result_p);
*/

namespace boost { namespace geometry { namespace strategy
{

template
<
    typename CalculationType,
    bool EnableClosestPoint = false
>
class comparable_spherical_point_segment_distance
{

public :

    struct result_type
    {
        result_type()
            : distance(0)
            , lon1(0)
            , lat1(0)
            , lon2(0)
            , lat2(0)
        {}

        CalculationType distance;
        CalculationType lon1;
        CalculationType lat1;
        CalculationType lon2;
        CalculationType lat2;
    };

    template <typename Point, typename PointOfSegment, typename Strategy>
    static inline result_type
    apply(Point const& p,
          PointOfSegment const& sp1,
          PointOfSegment const& sp2,
          Strategy const& comparable_distance_strategy)
    {
        result_type res;
        if (EnableClosestPoint)
        {
            res.lon1 = get_as_radian<0>(p);
            res.lat1 = get_as_radian<1>(p);
        }

        typedef CalculationType CT;

        // http://williams.best.vwh.net/avform.htm#XTE
        CT d1 = comparable_distance_strategy.apply(sp1, p);
        CT d3 = comparable_distance_strategy.apply(sp1, sp2);

        if (geometry::math::equals(d3, 0.0))
        {
            // "Degenerate" segment, return either d1 or d2
            res.distance = d1;
            if (EnableClosestPoint)
            {
                res.lon2 = get_as_radian<0>(sp1);
                res.lat2 = get_as_radian<1>(sp1);
            }
            return res;
        }

        CT d2 = comparable_distance_strategy.apply(sp2, p);

        CT lon1 = geometry::get_as_radian<0>(sp1);
        CT lat1 = geometry::get_as_radian<1>(sp1);
        CT lon2 = geometry::get_as_radian<0>(sp2);
        CT lat2 = geometry::get_as_radian<1>(sp2);
        CT lon = geometry::get_as_radian<0>(p);
        CT lat = geometry::get_as_radian<1>(p);

        CT crs_AD = geometry::formula::spherical_azimuth
            <
                CT,
                false
            >(lon1, lat1, lon, lat).azimuth;

        geometry::formula::result_spherical<CT> result =
            geometry::formula::spherical_azimuth
                <
                    CT,
                    true
                >(lon1, lat1, lon2, lat2);

        CT crs_AB = result.azimuth;
        CT crs_BA = result.reverse_azimuth - geometry::math::pi<CT>();

        CT crs_BD = geometry::formula::spherical_azimuth<CT, false>
                             (lon2, lat2, lon, lat).azimuth;

        CT d_crs1 = crs_AD - crs_AB;
        CT d_crs2 = crs_BD - crs_BA;

        // d1, d2, d3 are in principle not needed, only the sign matters
        CT projection1 = cos( d_crs1 ) * d1 / d3;
        CT projection2 = cos( d_crs2 ) * d2 / d3;

#ifdef BOOST_GEOMETRY_DEBUG_PT_SEG
        /*
        std::cout << "Course " << dsv(sp1) << " to " << dsv(p) << " "
                  << crs_AD * geometry::math::r2d<CT>() << std::endl;
        std::cout << "Course " << dsv(sp1) << " to " << dsv(sp2) << " "
                  << crs_AB * geometry::math::r2d<CT>() << std::endl;
        std::cout << "Course " << dsv(sp2) << " to " << dsv(sp1) << " "
                  << crs_BA * geometry::math::r2d<CT>() << std::endl;
        std::cout << "Course " << dsv(sp2) << " to " << dsv(p) << " "
                  << crs_BD * geometry::math::r2d<CT>() << std::endl;*/
        std::cout << "Projection AD-AB " << projection1 << " : "
                  << d_crs1 * geometry::math::r2d<CT>() << std::endl;
        std::cout << "Projection BD-BA " << projection2 << " : "
                  << d_crs2 * geometry::math::r2d<CT>() << std::endl;
        std::cout << " d1: " << (d1 )
                  << " d2: " << (d2 )
                  << std::endl;
#endif

        if (projection1 > 0.0 && projection2 > 0.0)
        {
#ifdef BOOST_GEOMETRY_DEBUG_PT_SEG
            CT XTD = comparable_distance_strategy.radius()
                    * geometry::math::abs( asin( sin( d1 ) * sin( d_crs1 ) ));

            std::cout << "Projection ON the segment" << std::endl;
            std::cout << "XTD: " << XTD
                      << " d1: " << (d1 * comparable_distance_strategy.radius())
                      << " d2: " << (d2 * comparable_distance_strategy.radius())
                      << std::endl;
#endif
            CT const half(0.5);
            CT const quarter(0.25);

            CT sin_d_crs1 = sin(d_crs1);
            /*
              This is the straightforward obvious way to continue:

              return_type discriminant
                  = 1.0 - 4.0 * (d1 - d1 * d1) * sin_d_crs1 * sin_d_crs1;
              return 0.5 - 0.5 * math::sqrt(discriminant);

              Below we optimize the number of arithmetic operations
              and account for numerical robustness:
            */
            CT d1_x_sin = d1 * sin_d_crs1;
            CT d = d1_x_sin * (sin_d_crs1 - d1_x_sin);
            res.distance = d / (half + math::sqrt(quarter - d));
#ifdef BOOST_GEOMETRY_DEBUG_PT_SEG
            //boost::geometry::strategy::distance::cross_track<return_type,Strategy> str;
            //std::cout << str.apply(p, sp1, sp2)
            //        << std::endl;
            std::cout << "dist=" << res.distance << std::endl;
            std::cout << "dist_cross_track radius=" << comparable_distance_strategy.radius() << std::endl;
            std::cout << "dist_cross_track asin=" <<
                         asin(math::sqrt(res.distance)) << std::endl;


#endif
            if (EnableClosestPoint)
            {
                CT dist = CT(2)
                        * asin(math::sqrt(res.distance))
                        * comparable_distance_strategy.radius();
                CT dist_d1 = CT(2)
                        * asin(math::sqrt(d1))
                        * comparable_distance_strategy.radius();
                // this is similar to spherical computation in geographic
                // point_segment_distance formula
                CT earth_radius = comparable_distance_strategy.radius();
                CT cos_frac = cos(dist_d1 / earth_radius)
                        / cos(dist / earth_radius);
                CT s14_sph = cos_frac >= 1 ? CT(0)
                                           : cos_frac <= -1 ? math::pi<CT>()
                                                              * earth_radius
                                                            : acos(cos_frac)
                                                              * earth_radius;

                CT a12 = geometry::formula::spherical_azimuth<>(lon1, lat1, lon2, lat2);
                geometry::formula::result_direct<CT> res_direct =
                    geometry::formula::spherical_direct
                        <
                            true,
                            false
                        >(lon1, lat1, s14_sph, a12,
                          srs::sphere<CalculationType>(earth_radius));

                res.lon2 = res_direct.lon2;
                res.lat2 = res_direct.lat2;
            }
            return res;
        }
        else
        {
#ifdef BOOST_GEOMETRY_DEBUG_PT_SEG
            std::cout << "Projection OUTSIDE the segment" << std::endl;
#endif

            // Return shortest distance, project either on point sp1 or sp2
            if (d1 < d2)
            {
                res.distance = CT(d1);
                if (EnableClosestPoint)
                {
                    res.lon2 = get_as_radian<0>(sp1);
                    res.lat2 = get_as_radian<1>(sp1);
                }
            }
            else
            {
                res.distance = CT(d2);
                if (EnableClosestPoint)
                {
                    res.lon2 = get_as_radian<0>(sp2);
                    res.lat2 = get_as_radian<1>(sp2);
                }
            }
            return res;
        }
    }
};

}}} // namespace boost::geometry::strategy

#endif // BOOST_GEOMETRY_STRATEGIES_SPHERICAL_COMPARABLE_POINT_SEGMENT_DISTANCE_HPP