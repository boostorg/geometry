// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_STRATEGIES_GEOGRAPHIC_DISTANCE_CROSS_TRACK_HPP
#define BOOST_GEOMETRY_EXTENSIONS_STRATEGIES_GEOGRAPHIC_DISTANCE_CROSS_TRACK_HPP


#include <boost/concept_check.hpp>
#include <boost/mpl/if.hpp>
#include <boost/type_traits.hpp>


#include <boost/geometry/strategies/spherical/distance_cross_track.hpp>



#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS
namespace boost { namespace geometry
{

namespace strategy { namespace distance
{

namespace services
{


template <typename Point, typename PointOfSegment, typename Strategy>
struct default_strategy<segment_tag, Point, PointOfSegment, geographic_tag, geographic_tag, Strategy>
{
    typedef cross_track
        <
            Point,
            PointOfSegment,
            void,
            typename boost::mpl::if_
                <
                    boost::is_void<Strategy>,
                    typename default_strategy
                        <
                            point_tag, Point, PointOfSegment,
                            geographic_tag, geographic_tag
                        >::type,
                    Strategy
                >::type
        > type;
};



} // namespace services
#endif // DOXYGEN_NO_STRATEGY_SPECIALIZATIONS


}} // namespace strategy::distance


#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS


#endif


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_EXTENSIONS_STRATEGIES_GEOGRAPHIC_DISTANCE_CROSS_TRACK_HPP
