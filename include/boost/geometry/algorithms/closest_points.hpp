// Boost.Geometry

// Copyright (c) 2019, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ALGORITHMS_CLOSEST_POINTS_HPP
#define BOOST_GEOMETRY_ALGORITHMS_CLOSEST_POINTS_HPP

#include <boost/geometry/algorithms/clear.hpp>
#include <boost/geometry/algorithms/distance.hpp>
#include <boost/geometry/algorithms/detail/closest_points/default_strategies.hpp>
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
            Strategy,
            Geometry1,
            typename point_type<Geometry2>::type
        >::type result = distance <
                                      Geometry1, Geometry2, Strategy
                                  >::apply(g1, g2, strategy);

        set_from_radian<0,0>(shortest_seg, result.lon1);
        set_from_radian<0,1>(shortest_seg, result.lat1);
        set_from_radian<1,0>(shortest_seg, result.lon2);
        set_from_radian<1,1>(shortest_seg, result.lat2);
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

        typedef typename detail::closest_points::default_strategy
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


template <BOOST_VARIANT_ENUM_PARAMS(typename T), typename Geometry2>
struct closest_points<variant<BOOST_VARIANT_ENUM_PARAMS(T)>, Geometry2 >
{
    template <typename Segment, typename Strategy>
    struct visitor: static_visitor<void>
    {
        Geometry2 const& m_geometry2;
        Segment& m_shortest_segment;
        Strategy const& m_strategy;

        visitor(Geometry2 const& geometry2,
                Segment& shortest_seg,
                Strategy const& strategy)
            :  m_geometry2(geometry2)
            ,  m_shortest_segment(shortest_seg)
            ,  m_strategy(strategy)
        {}

        template <typename Geometry1>
        void operator()(Geometry1 const& geometry1) const
        {
            closest_points
                <
                    Geometry1,
                    Geometry2
                >::apply(geometry1, m_geometry2, m_shortest_segment, m_strategy);
        }
    };

    template <typename Segment, typename Strategy>
    static inline void
    apply(boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)> const& geometry1,
          Geometry2 const& geometry2,
          Segment& shortest_seg,
          Strategy const& strategy)
    {
        return boost::apply_visitor(
                    visitor<Segment, Strategy>(geometry2,
                                               shortest_seg,
                                               strategy),
                    geometry1);
    }
};


template <typename Geometry1, BOOST_VARIANT_ENUM_PARAMS(typename T)>
struct closest_points<Geometry1, variant<BOOST_VARIANT_ENUM_PARAMS(T)> >
{
    template <typename Segment, typename Strategy>
    struct visitor: static_visitor<void>
    {
        Geometry1 const& m_geometry1;
        Segment& m_shortest_segment;
        Strategy const& m_strategy;

        visitor(Geometry1 const& geometry1,
                Segment& shortest_seg,
                Strategy const& strategy)
            :  m_geometry1(geometry1)
            ,  m_shortest_segment(shortest_seg)
            ,  m_strategy(strategy)
        {}

        template <typename Geometry2>
        void operator()(Geometry2 const& geometry2) const
        {
            closest_points
                <
                    Geometry1,
                    Geometry2
                >::apply(m_geometry1, geometry2, m_shortest_segment, m_strategy);
        }
    };

    template <typename Segment, typename Strategy>
    static inline void
    apply(Geometry1 const& geometry1,
          boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)> const& geometry2,
          Segment& shortest_seg,
          Strategy const& strategy)
    {
        return boost::apply_visitor(
                    visitor<Segment, Strategy>(geometry1,
                                               shortest_seg,
                                               strategy),
                    geometry2);
    }
};


template
<
    BOOST_VARIANT_ENUM_PARAMS(typename T1),
    BOOST_VARIANT_ENUM_PARAMS(typename T2)
>
struct closest_points
    <
        boost::variant<BOOST_VARIANT_ENUM_PARAMS(T1)>,
        boost::variant<BOOST_VARIANT_ENUM_PARAMS(T2)>
    >
{
    template <typename Segment, typename Strategy>
    struct visitor: static_visitor<void>
    {
        Segment& m_shortest_segment;
        Strategy const& m_strategy;

        visitor(Segment& shortest_seg, Strategy const& strategy)
            :  m_shortest_segment(shortest_seg)
            ,  m_strategy(strategy)
        {}

        template <typename Geometry1, typename Geometry2>
        void operator()(Geometry1 const& geometry1,
                        Geometry2 const& geometry2) const
        {
            closest_points
                <
                    Geometry1,
                    Geometry2
                >::apply(geometry1, geometry2, m_shortest_segment, m_strategy);
        }
    };

    template <typename Segment, typename Strategy>
    static inline void
    apply(boost::variant<BOOST_VARIANT_ENUM_PARAMS(T1)> const& geometry1,
          boost::variant<BOOST_VARIANT_ENUM_PARAMS(T2)> const& geometry2,
          Segment& shortest_seg,
          Strategy const& strategy)
    {
        return boost::apply_visitor(
                    visitor<Segment, Strategy>(shortest_seg, strategy),
                    geometry1, geometry2);
    }
};


} // namespace resolve_variant

/*!
\brief Calculate the closest points between two geometries \brief_strategy
\ingroup closest_points
\details
\details The free function closest_points calculates the distance between two geometries \brief_strategy. \details_strategy_reasons

\tparam Geometry1 \tparam_geometry
\tparam Geometry2 \tparam_geometry
\tparam Segment Any type fulfilling a Segment Concept
\tparam Strategy \tparam_strategy{Closest Points}
\param geometry1 \param_geometry
\param geometry2 \param_geometry
\param shortest_seg Output segment containing the closest points
\param strategy \param_strategy{closest_points}
\note The strategy can be a point-point strategy. In case of distance point-line/point-polygon
    it may also be a point-segment strategy.

\qbk{distinguish,with strategy}

\qbk{

[heading Example]
[closest_points_strategy]
[closest_points_strategy_output]

[heading See also]
\* [link geometry.reference.algorithms.distance distance]
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

    detail::throw_on_empty_input(geometry1);
    detail::throw_on_empty_input(geometry2);

    resolve_variant::closest_points
        <
            Geometry1,
            Geometry2
        >::apply(geometry1, geometry2, shortest_seg, strategy);
}


/*!
\brief Compute the closest points between two geometries.
\ingroup closest_points
\details The free function closest_points calculates the closest points between two geometries. \details_default_strategy

\tparam Geometry1 \tparam_geometry
\tparam Geometry2 \tparam_geometry
\tparam Segment Any type fulfilling a Segment Concept
\param geometry1 \param_geometry
\param geometry2 \param_geometry
\param shortest_seg Output segment containing the closest points

\qbk{

[heading Example]
[closest_points]
[closest_points_output]

[heading See also]
\* [link geometry.reference.algorithms.distance distance]
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
