// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2019, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_CLOSEST_POINTS_CROSS_TRACK_BOX_BOX_HPP
#define BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_CLOSEST_POINTS_CROSS_TRACK_BOX_BOX_HPP

#include <boost/config.hpp>
#include <boost/concept_check.hpp>
#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_void.hpp>

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/assert.hpp>
#include <boost/geometry/core/point_type.hpp>
#include <boost/geometry/core/radian_access.hpp>
#include <boost/geometry/core/tags.hpp>

#include <boost/geometry/strategies/distance.hpp>
#include <boost/geometry/strategies/concepts/distance_concept.hpp>
#include <boost/geometry/strategies/spherical/distance_cross_track.hpp>
#include <boost/geometry/strategies/geographic/distance_cross_track.hpp>
#include <boost/geometry/strategies/spherical/distance_cross_track_box_box.hpp>

#include <boost/geometry/util/math.hpp>
#include <boost/geometry/algorithms/detail/assign_box_corners.hpp>


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
class geographic_cross_track_box_box
{
public:

    // point-point strategy getters
    struct closest_points_pp_strategy
    {
        typedef geographic<FormulaPolicy, Spheroid, CalculationType> type;
    };

    // point-segment strategy getters
    struct closest_points_ps_strategy
    {
        typedef geographic_cross_track
                <
                    FormulaPolicy,
                    Spheroid,
                    CalculationType
                > type;
    };

    template <typename Box1, typename Box2>
    struct return_type
        : promote_floating_point
          <
              typename select_calculation_type
                  <
                      typename point_type<Box1>::type,
                      typename point_type<Box2>::type,
                      CalculationType
                  >::type
          >
    {};

    //constructor

    explicit geographic_cross_track_box_box(Spheroid const& spheroid = Spheroid())
             : m_spheroid(spheroid)
    {}

    template <typename Box1, typename Box2>
    struct point_segment_distance_closest_point
        : formula::point_segment_distance
            <
                typename return_type<Box1, Box2>::type,
                true,
                FormulaPolicy,
                false
            >
    {};

    template <typename Box1, typename Box2>
    typename point_segment_distance_closest_point<Box1, Box2>::result_type
    apply(Box1 const& box1, Box2 const& box2) const
    {
        typedef typename point_segment_distance_closest_point<Box1, Box2>
                            ::result_type return_type;

        return //typename point_segment_distance_closest_point<Point, Box>::result_type();
                distance::details::cross_track_box_box_generic
                <return_type>::apply(box1, box2,
                                     typename closest_points_pp_strategy
                                                ::type(m_spheroid),
                                     typename closest_points_ps_strategy
                                                ::type(m_spheroid));
    }

private :
    Spheroid m_spheroid;
};

namespace services {

template <typename Box1, typename Box2>
struct default_strategy
    <
        box_tag, box_tag, Box1, Box2,
        geographic_tag, geographic_tag
    >
{
    typedef closest_points::geographic_cross_track_box_box<> type;
};

}} // namespace closest_points::services

namespace distance { namespace services {

#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS

template <typename FormulaPolicy>
struct tag<closest_points::geographic_cross_track_box_box<FormulaPolicy> >
{
    typedef strategy_tag_distance_box_box type;
};

template <typename FormulaPolicy, typename Spheroid>
struct tag<closest_points::geographic_cross_track_box_box<FormulaPolicy, Spheroid> >
{
    typedef strategy_tag_distance_box_box type;
};

template <typename FormulaPolicy, typename Spheroid, typename CalculationType>
struct tag<closest_points::geographic_cross_track_box_box<FormulaPolicy, Spheroid, CalculationType> >
{
    typedef strategy_tag_distance_box_box type;
};

//return types
template <typename FormulaPolicy, typename Spheroid, typename CalculationType, typename P, typename Box>
struct return_type<closest_points::geographic_cross_track_box_box<FormulaPolicy, Spheroid, CalculationType>, P, Box>
{
    typedef typename closest_points::geographic_cross_track_box_box<FormulaPolicy, Spheroid, CalculationType>
                          ::template point_segment_distance_closest_point<P, Box>
                          ::result_type type;
};

template <typename FormulaPolicy, typename Spheroid, typename P, typename Box>
struct return_type<closest_points::geographic_cross_track_box_box<FormulaPolicy, Spheroid>, P, Box>
{
    typedef typename closest_points::geographic_cross_track_box_box<FormulaPolicy, Spheroid>
                          ::template point_segment_distance_closest_point<P, Box>
                          ::result_type type;
};

template <typename FormulaPolicy, typename P, typename Box>
struct return_type<closest_points::geographic_cross_track_box_box<FormulaPolicy>, P, Box>
{
    typedef typename closest_points::geographic_cross_track_box_box<FormulaPolicy>
                          ::template point_segment_distance_closest_point<P, Box>
                          ::result_type type;
};


template <typename Strategy, typename Spheroid, typename CalculationType>
struct comparable_type<closest_points::geographic_cross_track_box_box<Strategy, Spheroid, CalculationType> >
{
    typedef closest_points::geographic_cross_track_box_box
        <
            Strategy, Spheroid, CalculationType
        > type;
};


template <typename Strategy, typename Spheroid, typename CalculationType>
struct get_comparable<closest_points::geographic_cross_track_box_box<Strategy, Spheroid, CalculationType> >
{
public:
    static inline closest_points::geographic_cross_track_box_box<Strategy, Spheroid, CalculationType>
    apply(closest_points::geographic_cross_track_box_box<Strategy, Spheroid, CalculationType> const& str)
    {
        return str;
    }
};


template <typename Strategy, typename Spheroid, typename CalculationType, typename Box1, typename Box2>
struct result_from_distance
    <
        closest_points::geographic_cross_track_box_box<Strategy, Spheroid, CalculationType>, Box1, Box2
    >
{
private:
    typedef closest_points::geographic_cross_track_box_box<Strategy, Spheroid, CalculationType> this_strategy;

    typedef typename this_strategy::template return_type
        <
            Box1, Box2
        >::type return_type;

public:
    template <typename T>
    static inline return_type apply(this_strategy const& strategy,
                                    T const& distance)
    {
        result_from_distance
            <
                Strategy, Box1, Box2
            >::apply(strategy, distance);
    }
};

} // namespace services
#endif // DOXYGEN_NO_STRATEGY_SPECIALIZATIONS


}} // namespace strategy::distance


}} // namespace boost::geometry


#endif //BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_CLOSEST_POINTS_CROSS_TRACK_BOX_BOX_HPP
