// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.

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
\tparam Point1 \tparam_first_point
\tparam Point2 \tparam_second_point
\tparam CalculationType \tparam_calculation
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
                    double // to avoid bad results in float
                >::type
        >::type calculation_type;

    inline vincenty()
    {}

    explicit inline vincenty(geometry::detail::ellipsoid<calculation_type> const& e)
        : m_ellipsoid(e)
    {}

    inline calculation_type apply(Point1 const& p1, Point2 const& p2) const
    {
        return calculate(get_as_radian<0>(p1), get_as_radian<1>(p1),
                        get_as_radian<0>(p2), get_as_radian<1>(p2));
    }

    inline geometry::detail::ellipsoid<calculation_type> ellipsoid() const
    {
        return m_ellipsoid;
    }


private :
    geometry::detail::ellipsoid<calculation_type> m_ellipsoid;

    inline calculation_type calculate(calculation_type const& lon1,
                calculation_type const& lat1,
                calculation_type const& lon2,
                calculation_type const& lat2) const
    {
        calculation_type const c2 = 2;
        calculation_type const pi = geometry::math::pi<calculation_type>();
        calculation_type const two_pi = c2 * pi;

        // lambda: difference in longitude on an auxiliary sphere
        calculation_type L = lon2 - lon1;
        calculation_type lambda = L;

        if (L < -pi) L += two_pi;
        if (L > pi) L -= two_pi;

        if (math::equals(lat1, lat2) && math::equals(lon1, lon2))
        {
            return calculation_type(0);
        }

        // U: reduced latitude, defined by tan U = (1-f) tan phi
        calculation_type const c1 = 1;
        calculation_type const one_min_f = c1 - m_ellipsoid.f();

        calculation_type const U1 = atan(one_min_f * tan(lat1)); // above (1)
        calculation_type const U2 = atan(one_min_f * tan(lat2)); // above (1)

        calculation_type const cos_U1 = cos(U1);
        calculation_type const cos_U2 = cos(U2);
        calculation_type const sin_U1 = sin(U1);
        calculation_type const sin_U2 = sin(U2);

        // alpha: azimuth of the geodesic at the equator
        calculation_type cos2_alpha;
        calculation_type sin_alpha;

        // sigma: angular distance p1,p2 on the sphere
        // sigma1: angular distance on the sphere from the equator to p1
        // sigma_m: angular distance on the sphere from the equator to the midpoint of the line
        calculation_type sigma;
        calculation_type sin_sigma;
        calculation_type cos2_sigma_m;

        calculation_type previous_lambda;

        calculation_type const c3 = 3;
        calculation_type const c4 = 4;
        calculation_type const c6 = 6;
        calculation_type const c16 = 16;

        calculation_type const c_e_12 = 1e-12;

        do
        {
            previous_lambda = lambda; // (13)
            calculation_type sin_lambda = sin(lambda);
            calculation_type cos_lambda = cos(lambda);
            sin_sigma = sqrt(math::sqr(cos_U2 * sin_lambda) + math::sqr(cos_U1 * sin_U2 - sin_U1 * cos_U2 * cos_lambda)); // (14)
            calculation_type cos_sigma = sin_U1 * sin_U2 + cos_U1 * cos_U2 * cos_lambda; // (15)
            sin_alpha = cos_U1 * cos_U2 * sin_lambda / sin_sigma; // (17)
            cos2_alpha = c1 - math::sqr(sin_alpha);
            cos2_sigma_m = math::equals(cos2_alpha, 0) ? 0 : cos_sigma - c2 * sin_U1 * sin_U2 / cos2_alpha; // (18)

            calculation_type C = m_ellipsoid.f()/c16 * cos2_alpha * (c4 + m_ellipsoid.f() * (c4 - c3 * cos2_alpha)); // (10)
            sigma = atan2(sin_sigma, cos_sigma); // (16)
            lambda = L + (c1 - C) * m_ellipsoid.f() * sin_alpha *
                (sigma + C * sin_sigma * ( cos2_sigma_m + C * cos_sigma * (-c1 + c2 * math::sqr(cos2_sigma_m)))); // (11)

        } while (geometry::math::abs(previous_lambda - lambda) > c_e_12
                && geometry::math::abs(lambda) < pi);

        calculation_type sqr_u = cos2_alpha * (math::sqr(m_ellipsoid.a()) - math::sqr(m_ellipsoid.b())) / math::sqr(m_ellipsoid.b()); // above (1)

        // Oops getting hard here
        // (again, problem is that ttmath cannot divide by doubles, which is OK)
        calculation_type const c47 = 47;
        calculation_type const c74 = 74;
        calculation_type const c128 = 128;
        calculation_type const c256 = 256;
        calculation_type const c175 = 175;
        calculation_type const c320 = 320;
        calculation_type const c768 = 768;
        calculation_type const c1024 = 1024;
        calculation_type const c4096 = 4096;
        calculation_type const c16384 = 16384;

        calculation_type A = c1 + sqr_u/c16384 * (c4096 + sqr_u * (-c768 + sqr_u * (c320 - c175 * sqr_u))); // (3)
        calculation_type B = sqr_u/c1024 * (c256 + sqr_u * ( -c128 + sqr_u * (c74 - c47 * sqr_u))); // (4)
        calculation_type delta_sigma = B * sin_sigma * ( cos2_sigma_m + (B/c4) * (cos(sigma)* (-c1 + c2 * cos2_sigma_m)
                - (B/c6) * cos2_sigma_m * (-c3 + c4 * math::sqr(sin_sigma)) * (-c3 + c4 * cos2_sigma_m))); // (6)

        return m_ellipsoid.b() * A * (sigma - delta_sigma); // (19)
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


template <typename Point1, typename Point2>
struct return_type<strategy::distance::vincenty<Point1, Point2> >
{
    typedef typename strategy::distance::vincenty<Point1, Point2>::calculation_type type;
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
    static inline typename return_type<vincenty<Point1, Point2> >::type apply(vincenty<Point1, Point2> const& , T const& value)
    {
        return value;
    }
};


} // namespace services
#endif // DOXYGEN_NO_STRATEGY_SPECIALIZATIONS


// We might add a vincenty-like strategy also for point-segment distance, but to calculate the projected point is not trivial



}} // namespace strategy::distance


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_EXTENSIONS_GIS_GEOGRAPHIC_STRATEGIES_VINCENTY_HPP
