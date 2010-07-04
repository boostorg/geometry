// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_GIS_GEOGRAPHIC_STRATEGIES_VINCENTY_HPP
#define BOOST_GEOMETRY_EXTENSIONS_GIS_GEOGRAPHIC_STRATEGIES_VINCENTY_HPP

#include <boost/math/constants/constants.hpp>


#include <boost/geometry/strategies/distance.hpp>
#include <boost/geometry/core/radian_access.hpp>
#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/util/select_calculation_type.hpp>
#include <boost/geometry/util/promote_floating_point.hpp>
#include <boost/geometry/util/math.hpp>

#include <boost/geometry/extensions/gis/geographic/detail/ellipsoid.hpp>




namespace boost { namespace geometry
{

namespace strategy { namespace distance
{

/*!
    \brief Distance calculation formulae on latlong coordinates, after Vincenty, 1975
    \ingroup distance
    \tparam Point1 first point type
    \tparam Point2 optional second point type
    \author See http://www.ngs.noaa.gov/PUBS_LIB/inverse.pdf
    \author Adapted from various implementations to get it close to the original document
        - http://www.movable-type.co.uk/scripts/LatLongVincenty.html
        - http://exogen.case.edu/projects/geopy/source/geopy.distance.html
        - http://futureboy.homeip.net/fsp/colorize.fsp?fileName=navigation.frink

*/
template 
<
    typename Point1, 
    typename Point2 = Point1,
    typename CalculationType = void
>
class vincenty
{
public :
    typedef typename promote_floating_point
        <
            typename select_most_precise
                <
                    typename select_calculation_type
                        <
                            Point1,
                            Point2,
                            CalculationType
                        >::type,
                    double // it should be at least double otherwise Vincenty does not run
                >::type
        >::type return_type;

    typedef Point1 first_point_type;
    typedef Point2 second_point_type;

    inline vincenty()
    {}

    explicit inline vincenty(geometry::detail::ellipsoid const& e)
        : m_ellipsoid(e)
    {}

    inline return_type apply(Point1 const& p1, Point2 const& p2) const
    {
        return calculate(get_as_radian<0>(p1), get_as_radian<1>(p1),
                        get_as_radian<0>(p2), get_as_radian<1>(p2));
    }

    inline geometry::detail::ellipsoid ellipsoid() const
    {
        return m_ellipsoid;
    }


private :
    typedef return_type promoted_type;
    geometry::detail::ellipsoid m_ellipsoid;

    inline return_type calculate(promoted_type const& lon1, 
                promoted_type const& lat1, 
                promoted_type const& lon2, 
                promoted_type const& lat2) const
    {
        namespace mc = boost::math::constants;

        promoted_type const c2 = 2;
        promoted_type const two_pi = c2 * mc::pi<promoted_type>();

        // lambda: difference in longitude on an auxiliary sphere
        promoted_type L = lon2 - lon1;
        promoted_type lambda = L;

        if (L < -mc::pi<promoted_type>()) L += two_pi;
        if (L > mc::pi<promoted_type>()) L -= two_pi;

        if (math::equals(lat1, lat2) && math::equals(lon1, lon2))
        {
            return return_type(0);
        }

        // TODO: give ellipsoid a template-parameter
        promoted_type const ellipsoid_f = m_ellipsoid.f();
        promoted_type const ellipsoid_a = m_ellipsoid.a();
        promoted_type const ellipsoid_b = m_ellipsoid.b();

        // U: reduced latitude, defined by tan U = (1-f) tan phi
        promoted_type const c1 = 1;
        promoted_type const one_min_f = c1 - ellipsoid_f;

        promoted_type const U1 = atan(one_min_f * tan(lat1)); // above (1)
        promoted_type const U2 = atan(one_min_f * tan(lat2)); // above (1)

        promoted_type const cos_U1 = cos(U1);
        promoted_type const cos_U2 = cos(U2);
        promoted_type const sin_U1 = sin(U1);
        promoted_type const sin_U2 = sin(U2);

        // alpha: azimuth of the geodesic at the equator
        promoted_type cos2_alpha;
        promoted_type sin_alpha;

        // sigma: angular distance p1,p2 on the sphere
        // sigma1: angular distance on the sphere from the equator to p1
        // sigma_m: angular distance on the sphere from the equator to the midpoint of the line
        promoted_type sigma;
        promoted_type sin_sigma;
        promoted_type cos2_sigma_m;

        promoted_type previous_lambda;

        promoted_type const c3 = 3;
        promoted_type const c4 = 4;
        promoted_type const c6 = 6;
        promoted_type const c16 = 16;

        promoted_type const c_e_12 = 1e-12;

        do
        {
            previous_lambda = lambda; // (13)
            promoted_type sin_lambda = sin(lambda);
            promoted_type cos_lambda = cos(lambda);
            sin_sigma = sqrt(math::sqr(cos_U2 * sin_lambda) + math::sqr(cos_U1 * sin_U2 - sin_U1 * cos_U2 * cos_lambda)); // (14)
            promoted_type cos_sigma = sin_U1 * sin_U2 + cos_U1 * cos_U2 * cos_lambda; // (15)
            sin_alpha = cos_U1 * cos_U2 * sin_lambda / sin_sigma; // (17)
            cos2_alpha = c1 - math::sqr(sin_alpha);
            cos2_sigma_m = cos2_alpha == 0 ? 0 : cos_sigma - c2 * sin_U1 * sin_U2 / cos2_alpha; // (18)


            promoted_type C = ellipsoid_f/c16 * cos2_alpha * (c4 + ellipsoid_f * (c4 - c3 * cos2_alpha)); // (10)
            sigma = atan2(sin_sigma, cos_sigma); // (16)
            lambda = L + (c1 - C) * ellipsoid_f * sin_alpha *
                (sigma + C * sin_sigma * ( cos2_sigma_m + C * cos_sigma * (-c1 + c2 * math::sqr(cos2_sigma_m)))); // (11)

        } while (geometry::math::abs(previous_lambda - lambda) > c_e_12
                && geometry::math::abs(lambda) < mc::pi<promoted_type>());

        promoted_type sqr_u = cos2_alpha * (math::sqr(ellipsoid_a) - math::sqr(ellipsoid_b)) / math::sqr(ellipsoid_b); // above (1)

        // Oops getting hard here
        // (again, problem is that ttmath cannot divide by doubles, which is OK)
        promoted_type const c47 = 47;
        promoted_type const c74 = 74;
        promoted_type const c128 = 128;
        promoted_type const c256 = 256;
        promoted_type const c175 = 175;
        promoted_type const c320 = 320;
        promoted_type const c768 = 768;
        promoted_type const c1024 = 1024;
        promoted_type const c4096 = 4096;
        promoted_type const c16384 = 16384;

        promoted_type A = c1 + sqr_u/c16384 * (c4096 + sqr_u * (-c768 + sqr_u * (c320 - c175 * sqr_u))); // (3)
        promoted_type B = sqr_u/c1024 * (c256 + sqr_u * ( -c128 + sqr_u * (c74 - c47 * sqr_u))); // (4)
        promoted_type delta_sigma = B * sin_sigma * ( cos2_sigma_m + (B/c4) * (cos(sigma)* (-c1 + c2 * cos2_sigma_m)
                - (B/c6) * cos2_sigma_m * (-c3 + c4 * math::sqr(sin_sigma)) * (-c3 + c4 * cos2_sigma_m))); // (6)

        promoted_type dist = ellipsoid_b * A * (sigma - delta_sigma); // (19)

        return return_type(dist);
    }
};

#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS
namespace services
{

template <typename Point1, typename Point2>
struct tag<strategy::distance::vincenty<Point1, Point2> >
{
    typedef strategy_tag_distance_point_point type;
};


template <typename Point1, typename Point2, typename P1, typename P2>
struct similar_type<vincenty<Point1, Point2>, P1, P2>
{
    typedef vincenty<P1, P2> type;
};


template <typename Point1, typename Point2, typename P1, typename P2>
struct get_similar<vincenty<Point1, Point2>, P1, P2>
{
    static inline vincenty<P1, P2> apply(vincenty<Point1, Point2> const& input)
    {
        return vincenty<P1, P2>(input.ellipsoid());
    }
};

template <typename Point1, typename Point2>
struct comparable_type<vincenty<Point1, Point2> >
{
    typedef vincenty<Point1, Point2> type;
};


template <typename Point1, typename Point2>
struct get_comparable<vincenty<Point1, Point2> >
{
    static inline vincenty<Point1, Point2> apply(vincenty<Point1, Point2> const& input)
    {
        return input;
    }
};

template <typename Point1, typename Point2>
struct result_from_distance<vincenty<Point1, Point2> >
{
    template <typename T>
    static inline typename vincenty<Point1, Point2>::return_type apply(vincenty<Point1, Point2> const& , T const& value)
    {
        return value;
    }
};


} // namespace services
#endif // DOXYGEN_NO_STRATEGY_SPECIALIZATIONS



// We might add a vincenty-like strategy also for point-segment distance, but to calculate the projected point is not trivial



}} // namespace strategy::distance


#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS

template <typename Point1, typename Point2>
struct strategy_tag<strategy::distance::vincenty<Point1, Point2> >
{
    typedef strategy_tag_distance_point_point type;
};


#endif


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_EXTENSIONS_GIS_GEOGRAPHIC_STRATEGIES_VINCENTY_HPP
