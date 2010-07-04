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

#include <boost/geometry/extensions/gis/geographic/detail/ellipsoid.hpp>


namespace boost { namespace geometry
{

namespace strategy { namespace distance
{

/*!
    \brief Distance calculation formulae on latlong coordinates, after Vincenty, 1975
    \ingroup distance
    \tparam P1 first point type
    \tparam P2 optional second point type
    \author See http://www.ngs.noaa.gov/PUBS_LIB/inverse.pdf
    \author Adapted from various implementations to get it close to the original document
        - http://www.movable-type.co.uk/scripts/LatLongVincenty.html
        - http://exogen.case.edu/projects/geopy/source/geopy.distance.html
        - http://futureboy.homeip.net/fsp/colorize.fsp?fileName=navigation.frink

*/
template <typename P1, typename P2 = P1>
class vincenty
{
    public :
        //typedef spherical_distance return_type;
        typedef P1 first_point_type;
        typedef P2 second_point_type;
        typedef double return_type;

        inline vincenty()
        {}

        explicit inline vincenty(geometry::detail::ellipsoid const& e)
            : m_ellipsoid(e)
        {}

        inline return_type apply(P1 const& p1, P2 const& p2) const
        {
            return calculate(get_as_radian<0>(p1), get_as_radian<1>(p1),
                            get_as_radian<0>(p2), get_as_radian<1>(p2));
        }

        inline geometry::detail::ellipsoid ellipsoid() const
        {
            return m_ellipsoid;
        }


    private :
        typedef typename coordinate_type<P1>::type T1;
        typedef typename coordinate_type<P2>::type T2;
        geometry::detail::ellipsoid m_ellipsoid;

        inline return_type calculate(T1 const& lon1, T1 const& lat1, T2 const& lon2, T2 const& lat2) const
        {
            namespace mc = boost::math::constants;

            double const two_pi = 2.0 * mc::pi<double>();

            // lambda: difference in longitude on an auxiliary sphere
            double L = lon2 - lon1;
            double lambda = L;

            if (L < -mc::pi<double>()) L += two_pi;
            if (L > mc::pi<double>()) L -= two_pi;

            if (lat1 == lat2 && lon1 == lon2)
            {
              return return_type(0);
            }

            // U: reduced latitude, defined by tan U = (1-f) tan phi
            double U1 = atan((1-m_ellipsoid.f()) * tan(lat1)); // above (1)
            double U2 = atan((1-m_ellipsoid.f()) * tan(lat2)); // above (1)

            double cos_U1 = cos(U1);
            double cos_U2 = cos(U2);
            double sin_U1 = sin(U1);
            double sin_U2 = sin(U2);

            // alpha: azimuth of the geodesic at the equator
            double cos2_alpha;
            double sin_alpha;

            // sigma: angular distance p1,p2 on the sphere
            // sigma1: angular distance on the sphere from the equator to p1
            // sigma_m: angular distance on the sphere from the equator to the midpoint of the line
            double sigma;
            double sin_sigma;
            double cos2_sigma_m;

            double previous_lambda;

            do
            {
                previous_lambda = lambda; // (13)
                double sin_lambda = sin(lambda);
                double cos_lambda = cos(lambda);
                sin_sigma = sqrt(math::sqr(cos_U2 * sin_lambda) + math::sqr(cos_U1 * sin_U2 - sin_U1 * cos_U2 * cos_lambda)); // (14)
                double cos_sigma = sin_U1 * sin_U2 + cos_U1 * cos_U2 * cos_lambda; // (15)
                sin_alpha = cos_U1 * cos_U2 * sin_lambda / sin_sigma; // (17)
                cos2_alpha = 1.0 - math::sqr(sin_alpha);
                cos2_sigma_m = cos2_alpha == 0 ? 0 : cos_sigma - 2.0 * sin_U1 * sin_U2 / cos2_alpha; // (18)

                double C = m_ellipsoid.f()/16.0 * cos2_alpha * (4.0 + m_ellipsoid.f() * (4.0 - 3.0 * cos2_alpha)); // (10)
                sigma = atan2(sin_sigma, cos_sigma); // (16)
                lambda = L + (1.0 - C) * m_ellipsoid.f() * sin_alpha *
                    (sigma + C * sin_sigma * ( cos2_sigma_m + C * cos_sigma * (-1.0 + 2.0 * math::sqr(cos2_sigma_m)))); // (11)

            } while (fabs(previous_lambda - lambda) > 1e-12 && fabs(lambda) < mc::pi<double>());

            double sqr_u = cos2_alpha * (math::sqr(m_ellipsoid.a()) - math::sqr(m_ellipsoid.b())) / math::sqr(m_ellipsoid.b()); // above (1)

            double A = 1.0 + sqr_u/16384.0 * (4096 + sqr_u * (-768.0 + sqr_u * (320.0 - 175.0 * sqr_u))); // (3)
            double B = sqr_u/1024.0 * (256.0 + sqr_u * ( -128.0 + sqr_u * (74.0 - 47.0 * sqr_u))); // (4)
            double delta_sigma = B * sin_sigma * ( cos2_sigma_m + (B/4.0) * (cos(sigma)* (-1.0 + 2.0 * cos2_sigma_m)
                    - (B/6.0) * cos2_sigma_m * (-3.0 + 4.0 * math::sqr(sin_sigma)) * (-3.0 + 4.0 * cos2_sigma_m))); // (6)

            double dist = m_ellipsoid.b() * A * (sigma - delta_sigma); // (19)

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

template <typename P1, typename P2>
struct strategy_tag<strategy::distance::vincenty<P1, P2> >
{
    typedef strategy_tag_distance_point_point type;
};


#endif


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_EXTENSIONS_GIS_GEOGRAPHIC_STRATEGIES_VINCENTY_HPP
