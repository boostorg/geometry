// Boost.Geometry

// Copyright (c) 2018, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ALGORITHMS_CLOSEST_POINTS_HPP
#define BOOST_GEOMETRY_ALGORITHMS_CLOSEST_POINTS_HPP


#include <boost/geometry/algorithms/clear.hpp>
#include <boost/geometry/algorithms/detail/convert_point_to_point.hpp>
#include <boost/geometry/algorithms/not_implemented.hpp>
#include <boost/geometry/core/closure.hpp>
#include <boost/geometry/core/exception.hpp>
#include <boost/geometry/core/point_type.hpp>
#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tags.hpp>
#include <boost/geometry/strategies/default_strategy.hpp>
#include <boost/geometry/strategies/densify.hpp>
#include <boost/geometry/util/condition.hpp>
#include <boost/geometry/util/range.hpp>

#include <boost/range/size.hpp>
#include <boost/range/value_type.hpp>

#include <boost/throw_exception.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template
<
    typename Geometry1, typename Geometry2, typename Strategy
>
struct closest_points
{
    template <typename Segment>
    static inline void apply(
        Geometry1 const& g1,
        Geometry2 const& g2,
        Segment& shortest_seg,
        Strategy const& strategy)
    {

        typename strategy::distance::services::return_type
                <
                    Strategy, Geometry1, typename point_type<Geometry2>::type
                >::type
                seg = distance <
                                Geometry1, Geometry2, Strategy,
                                point_tag, segment_tag, strategy_tag_distance_point_segment,
                                false
                            >::apply(g1, g2, strategy);

        //set_from_radian<0,0>(shortest_seg, get_as_radian<0>(p));
        //set_from_radian<0,1>(shortest_seg, get_as_radian<1>(p));
        set_from_radian<1,0>(shortest_seg, seg.lon);
        set_from_radian<1,1>(shortest_seg, seg.lat);
    }
};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


namespace resolve_strategy
{

struct closest_points
{
    template
    <
            typename Geometry1,
            typename Geometry2,
            typename Segment,
            typename Strategy
    >
    static inline void apply(Geometry1 const& geometry1,
                             Geometry2 const& geometry2,
                             Segment& shortest_seg,
                             Strategy const& strategy)
    {
        dispatch::closest_points<Geometry1, Geometry2, Strategy>
            ::apply(geometry1, geometry2, shortest_seg, strategy);
    }

    template <typename Geometry1, typename Geometry2, typename Segment>
    static inline void apply(Geometry1 const& geometry1,
                             Geometry2 const& geometry2,
                             Segment& shortest_seg,
                             default_strategy)
    {
        typedef typename detail::distance::default_strategy
            <
                Geometry1, Geometry2
            >::type strategy_type;

        /*BOOST_CONCEPT_ASSERT(
            (concepts::ClosestPointsStrategy<strategy_type>)
        );*/

        apply(geometry1, geometry2, shortest_seg, strategy_type());
    }
};

} // namespace resolve_strategy


namespace resolve_variant {

template <typename Geometry1, typename Geometry2>
struct closest_points
{
    template <typename Segment, typename Strategy>
    static inline void apply(Geometry1 const& geometry1,
                             Geometry2 const& geometry2,
                             Segment& shortest_seg,
                             Strategy const& strategy)
    {
        resolve_strategy::closest_points::apply(geometry1, geometry2,
                                                shortest_seg, strategy);
    }
};

/*
template <BOOST_VARIANT_ENUM_PARAMS(typename T)>
struct closest_points<boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)> >
{
    template <typename Segment, typename Strategy>
    struct visitor: boost::static_visitor<void>
    {
        Segment& m_shortest_seg;
        Strategy const& m_strategy;
        visitor(Segment const& shortest_seg, Strategy const& strategy)
            : m_shortest_seg(shortest_seg)
            , m_strategy(strategy)
        {}
        template <typename Geometry>
        void operator()(Geometry const& geometry, Geometry& out) const
        {
            closest_points<Geometry>::apply(geometry1, geometry2,
                                            m_shortest_seg, m_strategy);
        }
    };
    template <typename Segment, typename Strategy>
    static inline void
    apply(boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)> const& geometry1,
          boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)> const& geometry2,
          Segment& shortest_seg,
          Strategy const& strategy)
    {
        boost::apply_visitor(
            visitor<Segment, Strategy>(shortest_seg, strategy),
            geometry1,
            geometry2
        );
    }
};
*/

} // namespace resolve_variant


/*!
\brief Closest points
\ingroup closest_points
\tparam Geometry \tparam_geometry
\tparam Segment A segment type
\tparam Strategy A type fulfilling a ClosestPointsStrategy concept
\param geometry1 Input geometry1
\param geometry2 Input geometry2
\param shortest_seg Output segment
\param strategy ClosestPoints strategy
\qbk{distinguish,with strategy}
\qbk{[include reference/algorithms/closest_points.qbk]}
\qbk{
[heading Available Strategies]
[heading Example]
[closest_points]
[closest_points_output]
}
*/
template <typename Geometry1, typename Geometry2, typename Segment, typename Strategy>
inline void closest_points(Geometry1 const& geometry1,
                           Geometry2 const& geometry2,
                           Segment& shortest_seg,
                           Strategy const& strategy)
{
    concepts::check<Geometry1>();
    concepts::check<Geometry2>();

    geometry::clear(shortest_seg);

    resolve_variant::closest_points
        <
            Geometry1,
            Geometry2
        >::apply(geometry1, geometry2, shortest_seg, strategy);
}


/*!
\brief Closest points
\ingroup closest_points
\tparam Geometry \tparam_geometry
\tparam Segment A segment type
\param geometry1 Input geometry1
\param geometry2 Input geometry2
\param shortest_seg Output segment
\qbk{[include reference/algorithms/closest_points.qbk]}
\qbk{
[heading Example]
[closest_points]
[closest_points_output]
}
*/
template <typename Geometry1, typename Geometry2, typename Segment>
inline void closest_points(Geometry1 const& geometry1,
                           Geometry2 const& geometry2,
                           Segment& shortest_seg)
{
    closest_points(geometry1, geometry2, shortest_seg, default_strategy());
}


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_CLOSEST_POINTS_HPP
