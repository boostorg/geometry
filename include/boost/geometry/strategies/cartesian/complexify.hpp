// Boost.Geometry

// Copyright (c) 2017, Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_COMPLEXIFY_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_COMPLEXIFY_HPP


#include <boost/geometry/algorithms/detail/convert_point_to_point.hpp>
#include <boost/geometry/algorithms/detail/signed_size_type.hpp>
#include <boost/geometry/arithmetic/arithmetic.hpp>
#include <boost/geometry/arithmetic/dot_product.hpp>
#include <boost/geometry/core/coordinate_dimension.hpp>
#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/strategies/complexify.hpp>
#include <boost/geometry/util/range.hpp>
#include <boost/geometry/util/select_most_precise.hpp>

#include <boost/range/value_type.hpp>


namespace boost { namespace geometry
{

namespace strategy { namespace complexify
{


template
<
    typename CalculationType = void
>
struct cartesian
{
    template <typename Point, typename RangeOut, typename T>
    static inline void apply(Point const& p0, Point const& p1, RangeOut & rng, T const& length_threshold)
    {
        typedef typename boost::range_value<RangeOut>::type rng_point_t;
        typedef typename select_most_precise
            <
                typename coordinate_type<Point>::type,
                typename coordinate_type<rng_point_t>::type,
                CalculationType
            >::type calc_t;

        typedef model::point<calc_t, 2, cs::cartesian> point2d_t;
        
        point2d_t const xy0(geometry::get<0>(p0), geometry::get<1>(p0));
        point2d_t const xy1(geometry::get<0>(p1), geometry::get<1>(p1));
        // dir01 = xy1 - xy0
        point2d_t dir01 = xy1;
        geometry::subtract_point(dir01, xy0);
        calc_t const dot01 = geometry::dot_product(dir01, dir01);
        calc_t const len2d = math::sqrt(dot01);

        // TODO: For consistency with spherical and geographic 2d length is
        // taken into account. This probably should be changed. Also in the
        // other strategies dimensions > 2 should be taken into account.
        signed_size_type n = signed_size_type(len2d / length_threshold);
        if (n <= 0)
            return;

        // NOTE: Normalization will not work for integral coordinates
        // normalize
        //geometry::divide_value(dir01, len2d);

        calc_t step = len2d / (n + 1);

        calc_t d = step;
        for (signed_size_type i = 0 ; i < n ; ++i, d += step)
        {
            // pd = xy0 + d * dir01
            point2d_t pd = dir01;

            // without normalization
            geometry::multiply_value(pd, calc_t(i + 1));
            geometry::divide_value(pd, calc_t(n + 1));
            // with normalization
            //geometry::multiply_value(pd, d);
            
            geometry::add_point(pd, xy0);

            rng_point_t p;
            geometry::set<0>(p, geometry::get<0>(pd));
            geometry::set<1>(p, geometry::get<1>(pd));
            geometry::detail::conversion::point_to_point
                <
                    Point, rng_point_t,
                    2, dimension<rng_point_t>::value
                >::apply(p0, p);

            range::push_back(rng, p);
        }
    }
};


#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS
namespace services
{

template <>
struct default_strategy<cartesian_tag>
{
    typedef strategy::complexify::cartesian<> type;
};


} // namespace services
#endif // DOXYGEN_NO_STRATEGY_SPECIALIZATIONS


}} // namespace strategy::complexify


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_CARTESIAN_COMPLEXIFY_HPP
