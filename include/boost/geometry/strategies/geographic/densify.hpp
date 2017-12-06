// Boost.Geometry

// Copyright (c) 2017, Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_DENSIFY_HPP
#define BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_DENSIFY_HPP


#include <boost/geometry/algorithms/detail/convert_point_to_point.hpp>
#include <boost/geometry/algorithms/detail/signed_size_type.hpp>
#include <boost/geometry/core/coordinate_dimension.hpp>
#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/core/radian_access.hpp>
#include <boost/geometry/core/srs.hpp>
#include <boost/geometry/strategies/densify.hpp>
#include <boost/geometry/strategies/geographic/parameters.hpp>
#include <boost/geometry/util/range.hpp>
#include <boost/geometry/util/select_most_precise.hpp>

#include <boost/range/value_type.hpp>


namespace boost { namespace geometry
{

namespace strategy { namespace densify
{


template
<
    typename FormulaPolicy = strategy::andoyer,
    typename Spheroid = srs::spheroid<double>,
    typename CalculationType = void
>
struct geographic
{
    geographic()
        : m_spheroid()
    {}

    explicit geographic(Spheroid const& spheroid)
        : m_spheroid(spheroid)
    {}

    template <typename Point, typename RangeOut, typename T>
    inline void apply(Point const& p0, Point const& p1, RangeOut & rng, T const& length_threshold) const
    {
        typedef typename boost::range_value<RangeOut>::type rng_point_t;
        typedef typename select_most_precise
            <
                typename coordinate_type<Point>::type,
                typename coordinate_type<rng_point_t>::type,
                CalculationType
            >::type calc_t;

        typedef typename FormulaPolicy::template direct<calc_t, true, false, false, false> direct_t;
        typedef typename FormulaPolicy::template inverse<calc_t, true, true, false, false, false> inverse_t;

        typename inverse_t::result_type
            inv_r = inverse_t::apply(get_as_radian<0>(p0), get_as_radian<1>(p0),
                                     get_as_radian<0>(p1), get_as_radian<1>(p1),
                                     m_spheroid);

        signed_size_type n = signed_size_type(inv_r.distance / length_threshold);
        if (n <= 0)
            return;

        calc_t step = inv_r.distance / (n + 1);

        calc_t current = step;
        for (signed_size_type i = 0 ; i < n ; ++i, current += step)
        {
            typename direct_t::result_type
                dir_r = direct_t::apply(get_as_radian<0>(p0), get_as_radian<1>(p0),
                                        current, inv_r.azimuth,
                                        m_spheroid);

            rng_point_t p;
            set_from_radian<0>(p, dir_r.lon2);
            set_from_radian<1>(p, dir_r.lat2);
            geometry::detail::conversion::point_to_point
                <
                    Point, rng_point_t,
                    2, dimension<rng_point_t>::value
                >::apply(p0, p);

            range::push_back(rng, p);
        }
    }

private:
    Spheroid m_spheroid;
};


#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS
namespace services
{

template <>
struct default_strategy<geographic_tag>
{
    typedef strategy::densify::geographic<> type;
};


} // namespace services
#endif // DOXYGEN_NO_STRATEGY_SPECIALIZATIONS


}} // namespace strategy::densify


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DENSIFY_HPP
