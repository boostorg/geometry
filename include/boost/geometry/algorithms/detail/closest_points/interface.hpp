// Boost.Geometry

// Copyright (c) 2021, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_CLOSEST_POINTS_INTERFACE_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_CLOSEST_POINTS_INTERFACE_HPP

#include <boost/concept_check.hpp>

#include <boost/geometry/algorithms/detail/throw_on_empty_input.hpp>
#include <boost/geometry/algorithms/dispatch/closest_points.hpp>
#include <boost/geometry/algorithms/distance.hpp>

#include <boost/geometry/core/point_type.hpp>

#include <boost/geometry/geometries/adapted/boost_variant.hpp> // For backward compatibility
#include <boost/geometry/geometries/concepts/check.hpp>

#include <boost/geometry/strategies/default_strategy.hpp>
#include <boost/geometry/strategies/detail.hpp>
#include <boost/geometry/strategies/closest_points/services.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


// If reversal is needed, perform it

template
<
    typename Geometry1, 
    typename Geometry2, 
    typename Strategy,
    typename Tag1, 
    typename Tag2, 
    typename StrategyTag
>
struct closest_points
<
    Geometry1, Geometry2, Strategy,
    Tag1, Tag2, StrategyTag,
    true
>
    : closest_points<Geometry2, Geometry1, Strategy, Tag2, Tag1, StrategyTag, false>
{
    template <typename Segment>
    static inline void apply(Geometry1 const& g1, Geometry2 const& g2,
                             Segment& shortest_seg, Strategy const& strategy)
    {
        closest_points
            <
                Geometry2, Geometry1, Strategy,
                Tag2, Tag1, StrategyTag,
                false
            >::apply(g2, g1, shortest_seg, strategy);
        
        Segment temp;
        set<0,0>(temp, get<1,0>(shortest_seg));
        set<0,1>(temp, get<1,1>(shortest_seg));
        set<1,0>(temp, get<0,0>(shortest_seg));
        set<1,1>(temp, get<0,1>(shortest_seg));
        shortest_seg = temp;
    }
};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


namespace resolve_strategy
{

template
<
    typename Strategy,
    bool IsUmbrella = strategies::detail::is_umbrella_strategy<Strategy>::value
>
struct closest_points
{
    template <typename Geometry1, typename Geometry2, typename Segment>
    static inline void apply(Geometry1 const& geometry1,
                             Geometry2 const& geometry2,
                             Segment& shortest_seg,
                             Strategy const& strategy)
    {
        dispatch::closest_points
            <
                Geometry1, Geometry2, Strategy
            >::apply(geometry1, geometry2, shortest_seg, strategy);
    }
};

template <typename Strategy>
struct is_strategy_converter_specialized_cp
{
    typedef strategies::closest_points::services::strategy_converter<Strategy> converter;
    static const bool value = ! std::is_same
        <
            decltype(converter::get(std::declval<Strategy>())),
            strategies::detail::not_implemented
        >::value;
};

template <typename Strategy>
struct closest_points<Strategy, false>
{
    template
    <
        typename Geometry1, typename Geometry2, typename Segment, typename S,
        std::enable_if_t<is_strategy_converter_specialized_cp<S>::value, int> = 0
    >
    static inline void
    apply(Geometry1 const& geometry1,
          Geometry2 const& geometry2,
          Segment& shortest_seg,
          S const& strategy)
    {
        typedef strategies::closest_points::services::strategy_converter<Strategy> converter;
        typedef decltype(converter::get(strategy)) strategy_type;

        dispatch::closest_points
        <
            Geometry1, Geometry2, strategy_type
        >::apply(geometry1, geometry2, shortest_seg, converter::get(strategy));
    }

    template
    <
        typename Geometry1, typename Geometry2, typename Segment, typename S,
        std::enable_if_t<! is_strategy_converter_specialized_cp<S>::value, int> = 0
    >
    static inline void
    apply(Geometry1 const& geometry1,
          Geometry2 const& geometry2,
          Segment& shortest_seg,
          S const& strategy)
    {
        typedef strategies::closest_points::services::custom_strategy_converter
            <
                Geometry1, Geometry2, Strategy
            > converter;
        typedef decltype(converter::get(strategy)) strategy_type;

        dispatch::closest_points
        <
            Geometry1, Geometry2, strategy_type
        >::apply(geometry1, geometry2, shortest_seg, converter::get(strategy));
    }
};

template <>
struct closest_points<default_strategy, false>
{
    template <typename Geometry1, typename Geometry2, typename Segment>
    static inline void
    apply(Geometry1 const& geometry1,
          Geometry2 const& geometry2,
          Segment& shortest_seg,
          default_strategy)
    {
        typedef typename strategies::closest_points::services::default_strategy
            <
                Geometry1, Geometry2
            >::type strategy_type;

        dispatch::closest_points
            <
                Geometry1, Geometry2, strategy_type
            >::apply(geometry1, geometry2, shortest_seg, strategy_type());
    }
};

} // namespace resolve_strategy


namespace resolve_variant
{


template <typename Geometry1, typename Geometry2>
struct closest_points
{
    template <typename Segment, typename Strategy>
    static inline void apply(Geometry1 const& geometry1,
                             Geometry2 const& geometry2,
                             Segment& shortest_seg,
                             Strategy const& strategy)
    {
        resolve_strategy::closest_points
            <
                Strategy
            >::apply(geometry1, geometry2, shortest_seg, strategy);
    }
};

/*
template <BOOST_VARIANT_ENUM_PARAMS(typename T), typename Geometry2>
struct distance<variant<BOOST_VARIANT_ENUM_PARAMS(T)>, Geometry2>
{
    template <typename Strategy>
    struct visitor: static_visitor
        <
            typename distance_result
                <
                    variant<BOOST_VARIANT_ENUM_PARAMS(T)>,
                    Geometry2,
                    Strategy
                >::type
        >
    {
        Geometry2 const& m_geometry2;
        Strategy const& m_strategy;

        visitor(Geometry2 const& geometry2,
                Strategy const& strategy)
            : m_geometry2(geometry2),
              m_strategy(strategy)
        {}

        template <typename Geometry1>
        typename distance_result<Geometry1, Geometry2, Strategy>::type
        operator()(Geometry1 const& geometry1) const
        {
            return distance
                <
                    Geometry1,
                    Geometry2
                >::template apply
                    <
                        Strategy
                    >(geometry1, m_geometry2, m_strategy);
        }
    };

    template <typename Strategy>
    static inline typename distance_result
        <
            variant<BOOST_VARIANT_ENUM_PARAMS(T)>,
            Geometry2,
            Strategy
        >::type
    apply(variant<BOOST_VARIANT_ENUM_PARAMS(T)> const& geometry1,
          Geometry2 const& geometry2,
          Strategy const& strategy)
    {
        return boost::apply_visitor(visitor<Strategy>(geometry2, strategy), geometry1);
    }
};


template <typename Geometry1, BOOST_VARIANT_ENUM_PARAMS(typename T)>
struct distance<Geometry1, variant<BOOST_VARIANT_ENUM_PARAMS(T)> >
{
    template <typename Strategy>
    struct visitor: static_visitor
        <
            typename distance_result
                <
                    Geometry1,
                    variant<BOOST_VARIANT_ENUM_PARAMS(T)>,
                    Strategy
                >::type
        >
    {
        Geometry1 const& m_geometry1;
        Strategy const& m_strategy;

        visitor(Geometry1 const& geometry1,
                Strategy const& strategy)
            : m_geometry1(geometry1),
              m_strategy(strategy)
        {}

        template <typename Geometry2>
        typename distance_result<Geometry1, Geometry2, Strategy>::type
        operator()(Geometry2 const& geometry2) const
        {
            return distance
                <
                    Geometry1,
                    Geometry2
                >::template apply
                <
                    Strategy
                >(m_geometry1, geometry2, m_strategy);
        }
    };

    template <typename Strategy>
    static inline typename distance_result
        <
            Geometry1,
            variant<BOOST_VARIANT_ENUM_PARAMS(T)>,
            Strategy
        >::type
    apply(
        Geometry1 const& geometry1,
        const variant<BOOST_VARIANT_ENUM_PARAMS(T)>& geometry2,
        Strategy const& strategy)
    {
        return boost::apply_visitor(visitor<Strategy>(geometry1, strategy), geometry2);
    }
};


template
<
    BOOST_VARIANT_ENUM_PARAMS(typename T1),
    BOOST_VARIANT_ENUM_PARAMS(typename T2)
>
struct distance
    <
        boost::variant<BOOST_VARIANT_ENUM_PARAMS(T1)>,
        boost::variant<BOOST_VARIANT_ENUM_PARAMS(T2)>
    >
{
    template <typename Strategy>
    struct visitor: static_visitor
        <
            typename distance_result
                <
                    boost::variant<BOOST_VARIANT_ENUM_PARAMS(T1)>,
                    boost::variant<BOOST_VARIANT_ENUM_PARAMS(T2)>,
                    Strategy
                >::type
        >
    {
        Strategy const& m_strategy;

        visitor(Strategy const& strategy)
            : m_strategy(strategy)
        {}

        template <typename Geometry1, typename Geometry2>
        typename distance_result<Geometry1, Geometry2, Strategy>::type
        operator()(Geometry1 const& geometry1, Geometry2 const& geometry2) const
        {
            return distance
                <
                    Geometry1,
                    Geometry2
                >::template apply
                <
                    Strategy
                >(geometry1, geometry2, m_strategy);
        }
    };

    template <typename Strategy>
    static inline typename distance_result
        <
            boost::variant<BOOST_VARIANT_ENUM_PARAMS(T1)>,
            boost::variant<BOOST_VARIANT_ENUM_PARAMS(T2)>,
            Strategy
        >::type
    apply(boost::variant<BOOST_VARIANT_ENUM_PARAMS(T1)> const& geometry1,
          boost::variant<BOOST_VARIANT_ENUM_PARAMS(T2)> const& geometry2,
          Strategy const& strategy)
    {
        return boost::apply_visitor(visitor<Strategy>(strategy), geometry1, geometry2);
    }
};
*/
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
    concepts::check<Geometry1 const>();
    concepts::check<Geometry2 const>();

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

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_CLOSEST_POINTS_INTERFACE_HPP
