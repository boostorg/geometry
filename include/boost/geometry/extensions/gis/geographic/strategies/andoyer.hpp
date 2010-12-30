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
#include <boost/geometry/util/select_calculation_type.hpp>
#include <boost/geometry/util/promote_floating_point.hpp>
#include <boost/geometry/util/math.hpp>

#include <boost/geometry/extensions/gis/geographic/detail/ellipsoid.hpp>


namespace boost { namespace geometry
{

namespace strategy { namespace distance
{


/*!
    \brief Point-point distance approximation taking flattening into account
    \ingroup distance
    \tparam Point1 first point type
    \tparam Point2 optional second point type
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
template
<
    typename Point1,
    typename Point2 = Point1,
    typename CalculationType = void
>
class andoyer
{
    public :
    typedef typename promote_floating_point
        <
            typename select_calculation_type
                <
                    Point1,
                    Point2,
                    CalculationType
                >::type
        >::type calculation_type;

        inline andoyer()
            : m_ellipsoid()
        {}

        explicit inline andoyer(calculation_type f)
            : m_ellipsoid(f)
        {}

        explicit inline andoyer(geometry::detail::ellipsoid<calculation_type> const& e)
            : m_ellipsoid(e)
        {}


        inline calculation_type apply(Point1 const& point1, Point2 const& point2) const
        {
            return calc(get_as_radian<0>(point1), get_as_radian<1>(point1),
                            get_as_radian<0>(point2), get_as_radian<1>(point2));
        }

        inline geometry::detail::ellipsoid<calculation_type> ellipsoid() const
        {
            return m_ellipsoid;
        }

        inline calculation_type radius() const
        {
            return m_ellipsoid.a();
        }


    private :
        geometry::detail::ellipsoid<calculation_type> m_ellipsoid;

        inline calculation_type calc(calculation_type const& lon1, 
                    calculation_type const& lat1, 
                    calculation_type const& lon2, 
                    calculation_type const& lat2) const
        {
            calculation_type const G = (lat1 - lat2) / 2.0;
            calculation_type const lambda = (lon1 - lon2) / 2.0;

            if (geometry::math::equals(lambda, 0.0)
                && geometry::math::equals(G, 0.0))
            {
                return 0.0;
            }

            calculation_type const F = (lat1 + lat2) / 2.0;

            calculation_type const sinG2 = math::sqr(sin(G));
            calculation_type const cosG2 = math::sqr(cos(G));
            calculation_type const sinF2 = math::sqr(sin(F));
            calculation_type const cosF2 = math::sqr(cos(F));
            calculation_type const sinL2 = math::sqr(sin(lambda));
            calculation_type const cosL2 = math::sqr(cos(lambda));

            calculation_type const S = sinG2 * cosL2 + cosF2 * sinL2;
            calculation_type const C = cosG2 * cosL2 + sinF2 * sinL2;

            calculation_type const c0 = 0;
            calculation_type const c1 = 1;
            calculation_type const c2 = 2;
            calculation_type const c3 = 3;

            if (geometry::math::equals(S, c0) || geometry::math::equals(C, c0))
            {
                return c0;
            }

            calculation_type const omega = atan(sqrt(S / C));
            calculation_type const r3 = c3 * sqrt(S * C) / omega; // not sure if this is r or greek nu
            calculation_type const D = c2 * omega * m_ellipsoid.a();
            calculation_type const H1 = (r3 - c1) / (c2 * C);
            calculation_type const H2 = (r3 + c1) / (c2 * S);
            calculation_type const f = m_ellipsoid.f();

            return D * (c1 + f * H1 * sinF2 * cosG2 - f * H2 * cosF2 * sinG2);
        }
};


#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS
namespace services
{

template <typename Point1, typename Point2>
struct tag<strategy::distance::andoyer<Point1, Point2> >
{
    typedef strategy_tag_distance_point_point type;
};


template <typename Point1, typename Point2>
struct return_type<strategy::distance::andoyer<Point1, Point2> >
{
    typedef typename strategy::distance::andoyer<Point1, Point2>::calculation_type type;
};


template <typename Point1, typename Point2, typename P1, typename P2>
struct similar_type<andoyer<Point1, Point2>, P1, P2>
{
    typedef andoyer<P1, P2> type;
};


template <typename Point1, typename Point2, typename P1, typename P2>
struct get_similar<andoyer<Point1, Point2>, P1, P2>
{
    static inline andoyer<P1, P2> apply(andoyer<Point1, Point2> const& input)
    {
        return andoyer<P1, P2>(input.ellipsoid());
    }
};

template <typename Point1, typename Point2>
struct comparable_type<andoyer<Point1, Point2> >
{
    typedef andoyer<Point1, Point2> type;
};


template <typename Point1, typename Point2>
struct get_comparable<andoyer<Point1, Point2> >
{
    static inline andoyer<Point1, Point2> apply(andoyer<Point1, Point2> const& input)
    {
        return input;
    }
};

template <typename Point1, typename Point2>
struct result_from_distance<andoyer<Point1, Point2> >
{
    template <typename T>
    static inline typename return_type<andoyer<Point1, Point2> >::type apply(andoyer<Point1, Point2> const& , T const& value)
    {
        return value;
    }
};


template <typename Point1, typename Point2>
struct default_strategy<point_tag, Point1, Point2, geographic_tag, geographic_tag>
{
    typedef strategy::distance::andoyer<Point1, Point2> type;
};


} // namespace services
#endif // DOXYGEN_NO_STRATEGY_SPECIALIZATIONS


}} // namespace strategy::distance


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_EXTENSIONS_GIS_GEOGRAPHIC_STRATEGIES_ANDOYER_HPP
