// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2019, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_CLOSEST_POINTS_HPP
#define BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_CLOSEST_POINTS_HPP

#include <boost/geometry/algorithms/detail/closest_points/result.hpp>

#include <boost/geometry/strategies/closest_points.hpp>
//#include <boost/geometry/formulas/point_segment_distance.hpp>

namespace boost { namespace geometry
{

namespace strategy { namespace closest_points
{

template
<
    typename FormulaPolicy = strategy::andoyer,
    typename Spheroid = srs::spheroid<double>,
    typename CalculationType = void
>
class geographic
{
public :

    template <typename Point1, typename Point2>
    struct calculation_type
        : promote_floating_point
          <
              typename select_calculation_type
                  <
                      Point1,
                      Point2,
                      CalculationType
                  >::type
          >
    {};

    template <typename Point, typename PointOfSegment>
    struct closest_point_result
    {
        typedef geometry::detail::closest_points::result
                <
                    typename calculation_type<Point, PointOfSegment>::type
                > type;
    };

    inline geographic()
        : m_spheroid()
    {}

    explicit inline geographic(Spheroid const& spheroid)
        : m_spheroid(spheroid)
    {}

    Spheroid get_spheroid() const
    {
        return m_spheroid;
    }

    template <typename Point1, typename Point2>
    typename closest_point_result<Point1, Point2>::type
    apply(Point1 const& p1,
          Point2 const& p2) const
    {
        typedef typename calculation_type<Point1, Point2>::type CT;

        typename closest_point_result<Point1, Point2>::type result;

        result.lon1 = get_as_radian<0>(p1);
        result.lat1 = get_as_radian<1>(p1);
        result.lon2 = get_as_radian<0>(p2);
        result.lat2 = get_as_radian<1>(p2);

        result.distance = FormulaPolicy::template inverse
                <
                    CT, true, false, false, false, false
                >::apply(result.lon1, result.lat1,
                         result.lon2, result.lat2, m_spheroid).distance;

        return result;
    }

private :
    Spheroid m_spheroid;
};

#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS
namespace services
{

template <typename Point1, typename Point2>
struct default_strategy
    <
        point_tag, point_tag, Point1, Point2,
        geographic_tag, geographic_tag
    >
{
    typedef geographic<> type;
};

} // namespace services
#endif // DOXYGEN_NO_STRATEGY_SPECIALIZATIONS

} // namespace closest_points

namespace distance { namespace services {

//tags
template <typename FormulaPolicy>
struct tag<closest_points::geographic<FormulaPolicy> >
{
    typedef strategy_tag_distance_point_point type;
};

template
<
        typename FormulaPolicy,
        typename Spheroid
>
struct tag<closest_points::geographic<FormulaPolicy, Spheroid> >
{
    typedef strategy_tag_distance_point_point type;
};

template
<
        typename FormulaPolicy,
        typename Spheroid,
        typename CalculationType
>
struct tag<closest_points::geographic<FormulaPolicy,
                                      Spheroid,
                                      CalculationType> >
{
    typedef strategy_tag_distance_point_point type;
};


//return types
template <typename FormulaPolicy, typename P, typename PS>
struct return_type<closest_points::geographic<FormulaPolicy>, P, PS>
        : closest_points::geographic<FormulaPolicy>
                   ::template closest_point_result<P, PS>
{};

/*
template <typename FormulaPolicy, typename P, typename PS>
struct return_type<closest_points::comparable::geographic<FormulaPolicy>, P, PS>
{
    typedef typename closest_points::geographic_cross_track<FormulaPolicy>
                          ::template closest_point_result<P, PS>
                          ::result_type type;
};

template
<
        typename FormulaPolicy,
        typename Spheroid,
        typename P,
        typename PS
>
struct return_type<closest_points::comparable::geographic_cross_track<FormulaPolicy,
                                                             Spheroid>, P, PS>
{
    typedef typename closest_points::geographic_cross_track<FormulaPolicy, Spheroid>
                     ::template closest_point_result<P, PS>
                     ::result_type type;
};

template
<
        typename FormulaPolicy,
        typename Spheroid,
        typename CalculationType,
        typename P,
        typename PS
>
struct return_type<closest_points::comparable::geographic_cross_track
                   <
                        FormulaPolicy,
                        Spheroid,
                        CalculationType
                   >, P, PS>
{
    typedef typename closest_points::geographic_cross_track
                     <FormulaPolicy, Spheroid, CalculationType>
                    ::template closest_point_result<P, PS>
                    ::result_type type;
};
*/

//comparable types
template
<
        typename FormulaPolicy,
        typename Spheroid,
        typename CalculationType
>
struct comparable_type<closest_points::geographic
                         <FormulaPolicy, Spheroid, CalculationType> >
{
    typedef closest_points::geographic
        <
            FormulaPolicy, Spheroid, CalculationType
        >  type;
};

template
<
        typename FormulaPolicy,
        typename Spheroid,
        typename CalculationType
>
struct get_comparable
        <
            closest_points::geographic
                            <
                                FormulaPolicy,
                                Spheroid,
                                CalculationType
                            >
        >
{
    typedef typename comparable_type
        <
            closest_points::geographic
                    <
                        FormulaPolicy,
                        Spheroid,
                        CalculationType
                    >
        >::type comparable_type;
public :
    static inline comparable_type
    apply(closest_points::geographic
                    <FormulaPolicy, Spheroid, CalculationType> const& strategy)
    {
        return comparable_type(strategy);
    }
};

template
<
    typename CalculationType,
    typename Strategy
>
struct swap<strategy::closest_points::geographic<CalculationType, Strategy> >
{
    template <typename Result>
    static inline void apply(Result& res)
    {
        res.swap_points();
    }
};

template
<
    typename CalculationType,
    typename Strategy
>
struct result_set_unique_point
<
    strategy::closest_points::geographic<CalculationType, Strategy>
>
{
    template <typename T, typename Point>
    static inline void apply(T& result, Point const& point)
    {
        result.set_unique_point(point);
    }
};

}}} // namespace strategy::distance::services

}} // namespace boost::geometry
#endif // BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_CLOSEST_POINTS_HPP
