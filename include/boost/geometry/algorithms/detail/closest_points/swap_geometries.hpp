// Boost.Geometry

// Copyright (c) 2019, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_CLOSEST_POINTS_SWAP_GEOMETRIES_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_CLOSEST_POINTS_SWAP_GEOMETRIES_HPP

namespace boost { namespace geometry
{

namespace dispatch
{

template <typename StrategyType>
struct swap
{
    template <typename T>
    static inline T apply(T& t)
    {
        return t;
    }
};

}}}

#include <boost/geometry/strategies/strategies.hpp>

namespace boost { namespace geometry
{

namespace dispatch
{

template
<
    typename FormulaPolicy,
    typename Spheroid,
    typename CalculationType
>
struct swap<strategy::closest_points::geographic_cross_track<
                                            FormulaPolicy,
                                            Spheroid,
                                            CalculationType> >
{
    template <typename Result>
    static inline void apply(Result& res)
    {
        res.swap();
    }
};

template
<
    typename FormulaPolicy,
    typename Spheroid,
    typename CalculationType
>
struct swap<strategy::closest_points::comparable::geographic_cross_track<
                                            FormulaPolicy,
                                            Spheroid,
                                            CalculationType> >
{
    template <typename Result>
    static inline void apply(Result& res)
    {
        res.swap();
    }
};


template
<
    typename FormulaPolicy,
    typename Spheroid,
    typename CalculationType
>
struct swap<strategy::closest_points::geographic<
                                            FormulaPolicy,
                                            Spheroid,
                                            CalculationType> >
{
    template <typename Result>
    static inline void apply(Result& res)
    {
        res.swap();
    }
};

template
<
    typename FormulaPolicy,
    typename Spheroid,
    typename CalculationType
>
struct swap<strategy::closest_points::geographic_cross_track_point_box<
                                            FormulaPolicy,
                                            Spheroid,
                                            CalculationType> >
{
    template <typename Result>
    static inline void apply(Result& res)
    {
        res.swap();
    }
};

template
<
    typename FormulaPolicy,
    typename Spheroid,
    typename CalculationType
>
struct swap<strategy::closest_points::geographic_segment_box<
                                            FormulaPolicy,
                                            Spheroid,
                                            CalculationType> >
{
    template <typename Result>
    static inline void apply(Result& res)
    {
        res.swap();
    }
};

}}} //namespace boost::geometry::dispatch

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_CLOSEST_POINTS_SWAP_GEOMETRIES_HPP
