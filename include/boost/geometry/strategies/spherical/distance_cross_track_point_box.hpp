// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2008-2014 Bruno Lalande, Paris, France.
// Copyright (c) 2008-2014 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2009-2014 Mateusz Loskot, London, UK.

// This file was modified by Oracle on 2014.
// Modifications copyright (c) 2014, Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_SPHERICAL_DISTANCE_CROSS_TRACK_POINT_BOX_HPP
#define BOOST_GEOMETRY_STRATEGIES_SPHERICAL_DISTANCE_CROSS_TRACK_POINT_BOX_HPP


#include <boost/geometry/core/access.hpp>

#include <boost/geometry/strategies/distance.hpp>
#include <boost/geometry/strategies/spherical/distance_cross_track.hpp>
#include <boost/geometry/strategies/spherical/side_by_cross_track.hpp>

#include <boost/geometry/util/math.hpp>
#include <boost/geometry/util/calculation_type.hpp>



namespace boost { namespace geometry
{

namespace strategy { namespace distance
{

template
<
    typename CalculationType = void,
    typename Strategy = haversine<double, CalculationType>
>
class cross_track_point_box
{
public:
    template <typename Point, typename Box>
    struct return_type
        : promote_floating_point
          <
              typename select_calculation_type
                  <
                      Point,
                      typename point_type<Box>::type,
                      CalculationType
                  >::type
          >
    {};

    inline cross_track_point_box()
    {}

    explicit inline cross_track_point_box(typename Strategy::radius_type const& r)
        : m_pp_strategy(r)
    {}

    inline cross_track_point_box(Strategy const& s)
        : m_pp_strategy(s)
    {}
    
    template <typename Point, typename Box>
    inline typename return_type<Point, Box>::type
    apply(Point const& point, Box const& box) const
    {

#if !defined(BOOST_MSVC)
        BOOST_CONCEPT_ASSERT
            (
                (concept::PointDistanceStrategy
                    <
                        Strategy, Point,
                        typename point_type<Box>::type
                    >)
            );
#endif

        typedef typename return_type<Point, Box>::type return_type;
        
        // Create (counterclockwise) array of points, the fifth one closes it
        // If every point is on the LEFT side (=1) or ON the border (=0)
        // the distance should be equal to 0.

        // TODO: This isn't fully correct!
        // Since if 0 is returned the Point may be in the vicinity of the boundary.
        // So for this we need side strategy not taking EPS into account.

        boost::array<typename point_type<Box>::type, 5> bp;
        geometry::detail::assign_box_corners_oriented<true>(box, bp);
        bp[4] = bp[0];

        typename strategy::side::services::default_strategy
            <
                typename cs_tag<Box>::type
            >::type side_strategy;
        boost::ignore_unused_variable_warning(side_strategy);

        strategy::distance::cross_track
            <
                CalculationType,
                Strategy
            > ps_strategy(m_pp_strategy);

        for (int i = 1; i < 5; i++)
        {
            // TODO: Here basically the same thing is calculated twice
            // side and the distance to segment are both calculated using cross-track formula

            int const side = side_strategy.apply(point, bp[i - 1], bp[i]);
            
            // If the point is on the right side of the edge
            if ( side == -1 )
            {
                return ps_strategy.apply(point, bp[i - 1], bp[i]);
            }
        }

        // Return 0 if the point isn't on the right side of any segment
        return return_type(0);
    }

    inline typename Strategy::radius_type radius() const
    { return m_pp_strategy.radius(); }

private :

    Strategy m_pp_strategy;
};


#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS
namespace services
{

template <typename CalculationType, typename Strategy>
struct tag<cross_track_point_box<CalculationType, Strategy> >
{
    typedef strategy_tag_distance_point_box type;
};


template <typename CalculationType, typename Strategy, typename P, typename Box>
struct return_type<cross_track_point_box<CalculationType, Strategy>, P, Box>
    : cross_track_point_box<CalculationType, Strategy>::template return_type<P, Box>
{};


template <typename CalculationType, typename Strategy>
struct comparable_type<cross_track_point_box<CalculationType, Strategy> >
{
    // There is no shortcut, so the strategy itself is its comparable type
    typedef cross_track_point_box<CalculationType, Strategy>  type;
};


template
<
    typename CalculationType,
    typename Strategy
>
struct get_comparable<cross_track_point_box<CalculationType, Strategy> >
{
    typedef typename comparable_type
        <
            cross_track_point_box<CalculationType, Strategy>
        >::type comparable_type;
public :
    static inline comparable_type apply(cross_track_point_box<CalculationType, Strategy> const& strategy)
    {
        return cross_track_point_box(strategy.radius());
    }
};


template
<
    typename CalculationType,
    typename Strategy,
    typename P, typename Box
>
struct result_from_distance<cross_track_point_box<CalculationType, Strategy>, P, Box>
{
private :
    typedef typename cross_track_point_box<CalculationType, Strategy>::template return_type<P, Box> return_type;
public :
    template <typename T>
    static inline return_type apply(cross_track_point_box<CalculationType, Strategy> const& , T const& distance)
    {
        return distance;
    }
};


template <typename Point, typename Box, typename Strategy>
struct default_strategy
    <
        point_tag, box_tag, Point, Box,
        spherical_equatorial_tag, spherical_equatorial_tag,
        Strategy
    >
{
    typedef cross_track_point_box
        <
            void,
            typename boost::mpl::if_
                <
                    boost::is_void<Strategy>,
                    typename default_strategy
                        <
                            point_tag, point_tag,
                            Point, typename point_type<Box>::type,
                            spherical_equatorial_tag, spherical_equatorial_tag
                        >::type,
                    Strategy
                >::type
        > type;
};


template <typename Box, typename Point, typename Strategy>
struct default_strategy
    <
        box_tag, point_tag, Box, Point,
        spherical_equatorial_tag, spherical_equatorial_tag,
        Strategy
    >
{
    typedef typename default_strategy
        <
            point_tag, box_tag, Point, Box,
            spherical_equatorial_tag, spherical_equatorial_tag,
            Strategy
        >::type type;
};


} // namespace services
#endif // DOXYGEN_NO_STRATEGY_SPECIALIZATIONS


}} // namespace strategy::distance


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_STRATEGIES_SPHERICAL_DISTANCE_CROSS_TRACK_POINT_BOX_HPP
