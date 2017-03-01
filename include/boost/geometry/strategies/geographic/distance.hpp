// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2016 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2014-2017.
// Modifications copyright (c) 2014-2017 Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_DISTANCE_HPP
#define BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_DISTANCE_HPP


#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/core/radian_access.hpp>
#include <boost/geometry/core/radius.hpp>
#include <boost/geometry/core/srs.hpp>

#include <boost/geometry/formulas/andoyer_inverse.hpp>
#include <boost/geometry/formulas/flattening.hpp>

#include <boost/geometry/strategies/distance.hpp>

#include <boost/geometry/util/math.hpp>
#include <boost/geometry/util/promote_floating_point.hpp>
#include <boost/geometry/util/select_calculation_type.hpp>


namespace boost { namespace geometry
{

namespace strategy { namespace distance
{

template
<
    template<typename, bool, bool, bool, bool, bool> class InverseFormula = formula::andoyer_inverse,
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

    typedef Spheroid model_type;

    inline geographic()
        : m_spheroid()
    {}

    explicit inline geographic(Spheroid const& spheroid)
        : m_spheroid(spheroid)
    {}

    template <typename Point1, typename Point2>
    inline typename calculation_type<Point1, Point2>::type
    apply(Point1 const& point1, Point2 const& point2) const
    {
        return InverseFormula
            <
                typename calculation_type<Point1, Point2>::type,
                true, false, false, false, false
            >::apply(get_as_radian<0>(point1), get_as_radian<1>(point1),
                     get_as_radian<0>(point2), get_as_radian<1>(point2),
                     m_spheroid).distance;
    }

    inline Spheroid const& model() const
    {
        return m_spheroid;
    }

private :
    Spheroid m_spheroid;
};


#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS
namespace services
{

template
<
    template<typename, bool, bool, bool, bool, bool> class Formula,
    typename Spheroid,
    typename CalculationType
>
struct tag<geographic<Formula, Spheroid, CalculationType> >
{
    typedef strategy_tag_distance_point_point type;
};


template
<
    template<typename, bool, bool, bool, bool, bool> class Formula,
    typename Spheroid,
    typename CalculationType,
    typename P1,
    typename P2
>
struct return_type<geographic<Formula, Spheroid, CalculationType>, P1, P2>
    : geographic<Formula, Spheroid, CalculationType>::template calculation_type<P1, P2>
{};


template
<
    template<typename, bool, bool, bool, bool, bool> class Formula,
    typename Spheroid,
    typename CalculationType
>
struct comparable_type<geographic<Formula, Spheroid, CalculationType> >
{
    typedef geographic<Formula, Spheroid, CalculationType> type;
};


template
<
    template<typename, bool, bool, bool, bool, bool> class Formula,
    typename Spheroid,
    typename CalculationType
>
struct get_comparable<geographic<Formula, Spheroid, CalculationType> >
{
    static inline geographic<Formula, Spheroid, CalculationType>
        apply(geographic<Formula, Spheroid, CalculationType> const& input)
    {
        return input;
    }
};

template
<
    template<typename, bool, bool, bool, bool, bool> class Formula,
    typename Spheroid,
    typename CalculationType,
    typename P1,
    typename P2
>
struct result_from_distance<geographic<Formula, Spheroid, CalculationType>, P1, P2>
{
    template <typename T>
    static inline typename return_type<geographic<Formula, Spheroid, CalculationType>, P1, P2>::type
        apply(geographic<Formula, Spheroid, CalculationType> const& , T const& value)
    {
        return value;
    }
};


template <typename Point1, typename Point2>
struct default_strategy<point_tag, point_tag, Point1, Point2, geographic_tag, geographic_tag>
{
    typedef strategy::distance::geographic
                <
                    formula::andoyer_inverse,
                    srs::spheroid
                        <
                            typename select_coordinate_type<Point1, Point2>::type
                        >
                > type;
};


} // namespace services
#endif // DOXYGEN_NO_STRATEGY_SPECIALIZATIONS


}} // namespace strategy::distance


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_DISTANCE_HPP
