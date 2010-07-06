// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_MULTI_ALGORITHMS_DISTANCE_HPP
#define BOOST_GEOMETRY_MULTI_ALGORITHMS_DISTANCE_HPP


#include <boost/numeric/conversion/bounds.hpp>
#include <boost/range.hpp>

#include <boost/geometry/multi/core/tags.hpp>
#include <boost/geometry/multi/core/is_multi.hpp>
#include <boost/geometry/multi/core/geometry_id.hpp>
#include <boost/geometry/multi/core/point_type.hpp>

#include <boost/geometry/algorithms/distance.hpp>
#include <boost/geometry/util/select_coordinate_type.hpp>


namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace distance
{


template<typename Geometry, typename MultiGeometry, typename Strategy>
struct distance_single_to_multi
{
    typedef typename strategy::distance::services::return_type<Strategy>::type return_type;

    static inline return_type apply(Geometry const& geometry,
                MultiGeometry const& multi,
                Strategy const& strategy)
    {
        using namespace boost;

        bool first = true;
        return_type mindist;

        for(typename range_iterator<MultiGeometry const>::type it = begin(multi);
                it != end(multi);
                ++it)
        {
            return_type dist = geometry::distance(geometry, *it);
            if (first || dist < mindist)
            {
                mindist = dist;
            }
            first = false;
        }

        return mindist;
    }
};

template<typename Multi1, typename Multi2, typename Strategy>
struct distance_multi_to_multi
{
    typedef typename strategy::distance::services::return_type<Strategy>::type return_type;

    static inline return_type apply(Multi1 const& multi1,
                Multi2 const& multi2, Strategy const& strategy)
    {
        using namespace boost;

        bool first = true;
        return_type mindist;

        for(typename range_iterator<Multi1 const>::type it = begin(multi1);
                it != end(multi1);
                ++it)
        {
            return_type dist = distance_single_to_multi
                <
                    typename range_value<Multi1>::type,
                    Multi2,
                    Strategy
                >::apply(*it, multi2, strategy);
            if (first || dist < mindist)
            {
                mindist = dist;
            }
            first = false;
        }

        return mindist;
    }
};


}} // namespace detail::distance
#endif


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template <typename GeometryTag1, typename GeometryTag2,
        typename G1, typename G2, typename Strategy>
struct distance<GeometryTag1, GeometryTag2, G1, G2, strategy_tag_distance_point_point, Strategy, false, true>
    : detail::distance::distance_single_to_multi<G1, G2, Strategy>
{};

template <typename GeometryTag1, typename GeometryTag2,
        typename G1, typename G2, typename Strategy>
struct distance<GeometryTag1, GeometryTag2, G1, G2, strategy_tag_distance_point_point, Strategy, true, true>
    : detail::distance::distance_multi_to_multi<G1, G2, Strategy>
{};

} // namespace dispatch
#endif



}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_MULTI_ALGORITHMS_DISTANCE_HPP
