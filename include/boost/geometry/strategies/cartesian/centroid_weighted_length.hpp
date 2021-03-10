// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2009-2015 Mateusz Loskot, London, UK.
// Copyright (c) 2009-2015 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2015-2021.
// Modifications copyright (c) 2015-2021, Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_CENTROID_WEIGHTED_LENGTH_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_CENTROID_WEIGHTED_LENGTH_HPP

#include <boost/math/special_functions/fpclassify.hpp>
#include <boost/numeric/conversion/cast.hpp>

#include <boost/geometry/arithmetic/arithmetic.hpp>

// Helper geometry
#include <boost/geometry/geometries/point.hpp>

#include <boost/geometry/strategies/cartesian/distance_pythagoras.hpp>
#include <boost/geometry/strategies/centroid.hpp>

#include <boost/geometry/util/for_each_coordinate.hpp>
#include <boost/geometry/util/select_most_precise.hpp>


namespace boost { namespace geometry
{

namespace strategy { namespace centroid
{

template
<
    typename Ignored1 = void,
    typename Ignored2 = void,
    typename CalculationType = void
>
class weighted_length
{
private :
    typedef geometry::strategy::distance::pythagoras<CalculationType> pythagoras_strategy;

    template <typename GeometryPoint, typename ResultPoint>
    struct calculation_type
    {
        // Below the distance between two GeometryPoints is calculated.
        // ResultPoint is taken into account by passing them together here.
        typedef typename pythagoras_strategy::template calculation_type
            <
                GeometryPoint, ResultPoint
            >::type type;
    };

    template <typename GeometryPoint, typename ResultPoint>
    class sums
    {
        friend class weighted_length;
        template <typename, typename> friend struct set_sum_div_length;

        typedef typename calculation_type<GeometryPoint, ResultPoint>::type calc_type;
        typedef typename geometry::model::point
            <
                calc_type,
                geometry::dimension<ResultPoint>::value,
                cs::cartesian
            > work_point;

        calc_type length;
        work_point average_sum;

    public:
        inline sums()
            : length(calc_type())
        {
            geometry::assign_zero(average_sum);
        }
    };

public :
    template <typename GeometryPoint, typename ResultPoint>
    struct state_type
    {
        typedef sums<GeometryPoint, ResultPoint> type;
    };

    template <typename GeometryPoint, typename ResultPoint>
    static inline void apply(GeometryPoint const& p1, GeometryPoint const& p2,
                             sums<GeometryPoint, ResultPoint>& state)
    {
        typedef typename calculation_type<GeometryPoint, ResultPoint>::type distance_type;

        distance_type const d = pythagoras_strategy::apply(p1, p2);
        state.length += d;

        typename sums<GeometryPoint, ResultPoint>::work_point weighted_median;
        geometry::assign_zero(weighted_median);
        geometry::add_point(weighted_median, p1);
        geometry::add_point(weighted_median, p2);
        geometry::multiply_value(weighted_median, d/2);
        geometry::add_point(state.average_sum, weighted_median);
    }

    template <typename GeometryPoint, typename ResultPoint>
    static inline bool result(sums<GeometryPoint, ResultPoint> const& state,
                              ResultPoint& centroid)
    {
        typedef typename calculation_type<GeometryPoint, ResultPoint>::type distance_type;

        distance_type const zero = distance_type();
        if (! geometry::math::equals(state.length, zero)
            && boost::math::isfinite(state.length)) // Prevent NaN centroid coordinates
        {
            // NOTE: above distance_type is checked, not the centroid coordinate_type
            // which means that the centroid can still be filled with INF
            // if e.g. distance_type is double and centroid contains floats
            geometry::for_each_coordinate(centroid,
                set_sum_div_length<GeometryPoint, ResultPoint>(state));
            return true;
        }

        return false;
    }

private:
    template <typename GeometryPoint, typename ResultPoint>
    struct set_sum_div_length
    {
        typedef typename state_type<GeometryPoint, ResultPoint>::type state_t;
        state_t const& m_state;
        set_sum_div_length(state_t const& state)
            : m_state(state)
        {}
        template <typename Pt, std::size_t Dimension>
        void apply(Pt & centroid) const
        {
            typedef typename geometry::coordinate_type<Pt>::type coordinate_type;
            geometry::set<Dimension>(
                centroid,
                boost::numeric_cast<coordinate_type>(
                    geometry::get<Dimension>(m_state.average_sum) / m_state.length
                )
            );
        }
    };
};

#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS

namespace services
{


// Register this strategy for linear geometries, in all dimensions

template <std::size_t N, typename Point, typename Geometry>
struct default_strategy
<
    cartesian_tag,
    linear_tag,
    N,
    Point,
    Geometry
>
{
    typedef weighted_length
        <
            Point,
            typename point_type<Geometry>::type
        > type;
};


} // namespace services


#endif // DOXYGEN_NO_STRATEGY_SPECIALIZATIONS


}} // namespace strategy::centroid


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_STRATEGIES_CARTESIAN_CENTROID_WEIGHTED_LENGTH_HPP
