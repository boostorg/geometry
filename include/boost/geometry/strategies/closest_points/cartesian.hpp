// Boost.Geometry

// Copyright (c) 2021, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_STRATEGIES_CLOSEST_POINTS_CARTESIAN_HPP
#define BOOST_GEOMETRY_STRATEGIES_CLOSEST_POINTS_CARTESIAN_HPP


//#include <boost/geometry/strategies/cartesian/azimuth.hpp>

//#include <boost/geometry/strategies/cartesian/distance_projected_point.hpp>
//#include <boost/geometry/strategies/cartesian/distance_pythagoras.hpp>
#include <boost/geometry/strategies/cartesian/distance_pythagoras_box_box.hpp>
#include <boost/geometry/strategies/cartesian/distance_pythagoras_point_box.hpp>
#include <boost/geometry/strategies/cartesian/distance_segment_box.hpp>

#include <boost/geometry/strategies/cartesian/closest_points_pt_pt.hpp>
#include <boost/geometry/strategies/cartesian/closest_points_pt_seg.hpp>

#include <boost/geometry/strategies/detail.hpp>
#include <boost/geometry/strategies/distance/detail.hpp>
#include <boost/geometry/strategies/closest_points/services.hpp>

//#include <boost/geometry/strategies/normalize.hpp>
#include <boost/geometry/strategies/relate/cartesian.hpp>

#include <boost/geometry/util/type_traits.hpp>


namespace boost { namespace geometry
{

namespace strategies { namespace closest_points
{

template <typename CalculationType = void>
struct cartesian
    : public strategies::relate::cartesian<CalculationType>
{
    template <typename Geometry1, typename Geometry2>
    static auto closest_points(Geometry1 const&, Geometry2 const&,
                               distance::detail::enable_if_pp_t<Geometry1, Geometry2> * = nullptr)
    {
        return strategy::closest_points::dummy_pt_pt();
    }

    template <typename Geometry1, typename Geometry2>
    static auto closest_points(Geometry1 const&, Geometry2 const&,
                               distance::detail::enable_if_ps_t<Geometry1, Geometry2> * = nullptr)
    {
        return strategy::closest_points::projected_point
            <
                CalculationType,
                strategy::distance::pythagoras<CalculationType>
            >();
    }

    template <typename Geometry1, typename Geometry2>
    static auto closest_points(Geometry1 const&, Geometry2 const&,
                               distance::detail::enable_if_pb_t<Geometry1, Geometry2> * = nullptr)
    {
        //return strategy::distance::pythagoras_point_box<CalculationType>();
    }

    template <typename Geometry1, typename Geometry2>
    static auto closest_points(Geometry1 const&, Geometry2 const&,
                               distance::detail::enable_if_sb_t<Geometry1, Geometry2> * = nullptr)
    {
        //return strategy::distance::cartesian_segment_box
        //    <
        //        CalculationType,
        //        strategy::distance::pythagoras<CalculationType>
        //    >();
    }

    template <typename Geometry1, typename Geometry2>
    static auto closest_points(Geometry1 const&, Geometry2 const&,
                               distance::detail::enable_if_bb_t<Geometry1, Geometry2> * = nullptr)
    {
        //return strategy::distance::pythagoras_box_box<CalculationType>();
    }

    template <typename Geometry1, typename Geometry2>
    static auto comparable_distance(Geometry1 const&, Geometry2 const&,
                                    distance::detail::enable_if_pp_t<Geometry1, Geometry2> * = nullptr)
    {
        return strategy::distance::comparable::pythagoras<CalculationType>();
    }

    template <typename Geometry1, typename Geometry2>
    static auto comparable_distance(Geometry1 const&, Geometry2 const&,
                                    distance::detail::enable_if_ps_t<Geometry1, Geometry2> * = nullptr)
    {
        //TODO: compute the comparable distance
        return strategy::distance::projected_point<CalculationType>();
    }


    // TODO: used only to get the type in
    // distance::creturn_t<Point, Range, Strategies> cd_min;
    template <typename Geometry1, typename Geometry2>
    static auto distance(Geometry1 const&, Geometry2 const&,
                         distance::detail::enable_if_pp_t<Geometry1, Geometry2> * = nullptr)
    {
        return strategy::distance::comparable::pythagoras<CalculationType>();
    }

    
    template <typename Geometry1, typename Geometry2>
    static auto distance(Geometry1 const&, Geometry2 const&,
                         distance::detail::enable_if_ps_t<Geometry1, Geometry2> * = nullptr)
    {
        return strategy::distance::projected_point<CalculationType>();
    }

};


namespace services
{

template <typename Geometry1, typename Geometry2>
struct default_strategy<Geometry1, Geometry2, cartesian_tag, cartesian_tag>
{
    using type = strategies::closest_points::cartesian<>;
};

/*
template <typename CT>
struct strategy_converter<strategy::closest_points::pythagoras<CT> >
{
    template <typename S>
    static auto get(S const&)
    {
        return strategies::distance::cartesian<CT>();
    }
};

template <typename CT, typename PPS>
struct strategy_converter<strategy::distance::projected_point<CT, PPS> >
    : strategy_converter<PPS>
{};

template <typename CT>
struct strategy_converter<strategy::distance::pythagoras_point_box<CT> >
{
    static auto get(strategy::distance::pythagoras_point_box<CT> const&)
    {
        return strategies::distance::cartesian<CT>();
    }
};

template <typename CT, typename PPS>
struct strategy_converter<strategy::distance::cartesian_segment_box<CT, PPS> >
    : strategy_converter<PPS>
{};

template <typename CT>
struct strategy_converter<strategy::distance::pythagoras_box_box<CT> >
{
    static auto get(strategy::distance::pythagoras_box_box<CT> const&)
    {
        return strategies::distance::cartesian<CT>();
    }
};


template <typename CT>
struct strategy_converter<strategy::distance::comparable::pythagoras<CT> >
{
    template <typename S>
    static auto get(S const&)
    {
        return strategies::distance::detail::make_comparable(
                    strategies::distance::cartesian<CT>());
    }
};

template <typename CT>
struct strategy_converter<strategy::distance::comparable::pythagoras_point_box<CT> >
{
    static auto get(strategy::distance::comparable::pythagoras_point_box<CT> const&)
    {
        return strategies::distance::detail::make_comparable(
                    strategies::distance::cartesian<CT>());
    }
};

template <typename CT>
struct strategy_converter<strategy::distance::comparable::pythagoras_box_box<CT> >
{
    static auto get(strategy::distance::comparable::pythagoras_box_box<CT> const&)
    {
        return strategies::distance::detail::make_comparable(
                    strategies::distance::cartesian<CT>());
    }
};
*/

} // namespace services

}} // namespace strategies::closest_points

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_CLOSEST_POINTS_CARTESIAN_HPP
