// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_GIS_GEOGRAPHIC_STRATEGIES_ANDOYER_HPP
#define BOOST_GEOMETRY_EXTENSIONS_GIS_GEOGRAPHIC_STRATEGIES_ANDOYER_HPP


#include <boost/geometry/strategies/distance.hpp>
#include <boost/geometry/core/radian_access.hpp>
#include <boost/geometry/core/coordinate_type.hpp>

#include <boost/geometry/extensions/gis/geographic/detail/ellipsoid.hpp>


namespace boost { namespace geometry
{
namespace strategy
{

    namespace distance
    {


        /*!
            \brief Point-point distance approximation taking flattening into account
            \ingroup distance
            \tparam P1 first point type
            \tparam P2 optional second point type
            \author After Andoyer, 19xx, republished 1950, republished by Meeus, 1999
            \note Although not so well-known, the approximation is very good: in all cases the results
            are about the same as Vincenty. In my (Barend's) testcases the results didn't differ more than 6 m
            \see http://nacc.upc.es/tierra/node16.html
            \see http://sci.tech-archive.net/Archive/sci.geo.satellite-nav/2004-12/2724.html
            \see http://home.att.net/~srschmitt/great_circle_route.html (implementation)
            \see http://www.codeguru.com/Cpp/Cpp/algorithms/article.php/c5115 (implementation)
            \see http://futureboy.homeip.net/frinksamp/navigation.frink (implementation)
            \see http://www.voidware.com/earthdist.htm (implementation)
        */
        template <typename P1, typename P2 = P1>
        class andoyer
        {
            public :
                //typedef spherical_distance return_type;
                typedef P1 first_point_type;
                typedef P2 second_point_type;
                typedef double return_type;

                andoyer()
                    : m_ellipsoid()
                {}
                andoyer(double f)
                    : m_ellipsoid(f)
                {}

                inline return_type apply(const P1& p1, const P2& p2) const
                {
                    return calc(get_as_radian<0>(p1), get_as_radian<1>(p1),
                                    get_as_radian<0>(p2), get_as_radian<1>(p2));
                }


            private :
                typedef typename coordinate_type<P1>::type T1;
                typedef typename coordinate_type<P2>::type T2;
                geometry::detail::ellipsoid m_ellipsoid;

                inline return_type calc(const T1& lon1, const T1& lat1, const T2& lon2, const T2& lat2) const
                {
                    typedef double calculation_type;
                    calculation_type G = (lat1 - lat2) / 2.0;
                    calculation_type lambda = (lon1 - lon2) / 2.0;

                    if (geometry::math::equals(lambda, 0.0)
                        && geometry::math::equals(G, 0.0))
                    {
                        return 0.0;
                    }

                    calculation_type F = (lat1 + lat2) / 2.0;

                    calculation_type sinG2 = math::sqr(sin(G));
                    calculation_type cosG2 = math::sqr(cos(G));
                    calculation_type sinF2 = math::sqr(sin(F));
                    calculation_type cosF2 = math::sqr(cos(F));
                    calculation_type sinL2 = math::sqr(sin(lambda));
                    calculation_type cosL2 = math::sqr(cos(lambda));

                    calculation_type S = sinG2 * cosL2 + cosF2 * sinL2;
                    calculation_type C = cosG2 * cosL2 + sinF2 * sinL2;

                    if (geometry::math::equals(S, 0.0) || geometry::math::equals(C, 0.0))
                    {
                        return 0.0;
                    }

                    calculation_type omega = atan(sqrt(S / C));
                    calculation_type r = sqrt(S * C) / omega; // not sure if this is r or greek nu

                    calculation_type D = 2.0 * omega * m_ellipsoid.a();
                    calculation_type H1 = (3 * r - 1.0) / (2.0 * C);
                    calculation_type H2 = (3 * r + 1.0) / (2.0 * S);

                    return return_type(D
                        * (1.0 + m_ellipsoid.f() * H1 * sinF2 * cosG2
                                    - m_ellipsoid.f() * H2 * cosF2 * sinG2));
                }
        };



    } // namespace distance


} // namespace strategy


#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS
template <typename P1, typename P2>
struct strategy_distance<geographic_tag, geographic_tag, P1, P2>
{
    typedef strategy::distance::andoyer<P1, P2> type;
};


template <typename P1, typename P2>
struct strategy_tag<strategy::distance::andoyer<P1, P2> >
{
    typedef strategy_tag_distance_point_point type;
};

#endif


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_EXTENSIONS_GIS_GEOGRAPHIC_STRATEGIES_ANDOYER_HPP
