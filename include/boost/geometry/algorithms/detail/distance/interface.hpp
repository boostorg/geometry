// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2014 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2014 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2014 Mateusz Loskot, London, UK.
// Copyright (c) 2013-2014 Adam Wulkiewicz, Lodz, Poland.
// Copyright (c) 2014 Samuel Debionne, Grenoble, France.

// This file was modified by Oracle on 2014.
// Modifications copyright (c) 2014, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_DISTANCE_INTERFACE_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_DISTANCE_INTERFACE_HPP

#include <boost/concept_check.hpp>

#include <boost/geometry/core/point_type.hpp>

#include <boost/geometry/algorithms/num_points.hpp>
#include <boost/geometry/multi/algorithms/num_points.hpp>

#include <boost/geometry/algorithms/detail/throw_on_empty_input.hpp>

#include <boost/geometry/geometries/concepts/check.hpp>

#include <boost/geometry/strategies/distance.hpp>
#include <boost/geometry/strategies/default_distance_result.hpp>

#include <boost/geometry/algorithms/detail/distance/default_strategies.hpp>

#include <boost/geometry/algorithms/dispatch/distance.hpp>

#include <boost/geometry/util/compress_variant.hpp>
#include <boost/geometry/util/transform_variant.hpp>
#include <boost/geometry/util/combine_if.hpp>
//#include <boost/geometry/util/is_implemented.hpp>

#include <boost/mpl/always.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/vector.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


// If reversal is needed, perform it
template
<
    typename Geometry1, typename Geometry2, typename Strategy,
    typename Tag1, typename Tag2, typename StrategyTag
>
struct distance
<
    Geometry1, Geometry2, Strategy,
    Tag1, Tag2, StrategyTag,
    true
>
    : distance<Geometry2, Geometry1, Strategy, Tag2, Tag1, StrategyTag, false>
{
    typedef typename strategy::distance::services::return_type
                     <
                         Strategy,
                         typename point_type<Geometry2>::type,
                         typename point_type<Geometry1>::type
                     >::type return_type;

    static inline return_type apply(
        Geometry1 const& g1,
        Geometry2 const& g2,
        Strategy const& strategy)
    {
        return distance
            <
                Geometry2, Geometry1, Strategy,
                Tag2, Tag1, StrategyTag,
                false
            >::apply(g2, g1, strategy);
    }
};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


namespace resolve_strategy {

namespace result_of
{

template <typename Geometry1, typename Geometry2, typename Strategy>
struct distance
    : strategy::distance::services::return_type
        <
            Strategy,
            typename point_type<Geometry1>::type,
            typename point_type<Geometry2>::type
        >
{};

template <typename Geometry1, typename Geometry2>
struct distance<Geometry1, Geometry2, default_strategy>
    : default_distance_result
        <
            Geometry1, Geometry2
        >
{};

} // namespace result_of


struct distance
{
    template <typename Geometry1, typename Geometry2, typename Strategy>
    static inline typename result_of::distance<Geometry1, Geometry2, Strategy>::type
    apply(Geometry1 const& geometry1, Geometry2 const& geometry2, Strategy const& strategy)
    {
        return dispatch::distance<Geometry1, Geometry2, Strategy>::apply(geometry1, geometry2, strategy);
    }

    template <typename Geometry1, typename Geometry2>
    static inline typename result_of::distance<Geometry1, Geometry2, default_strategy>::type
    apply(Geometry1 const& geometry1, Geometry2 const& geometry2, default_strategy)
    {
        typedef typename detail::distance::default_strategy
            <
                Geometry1, Geometry2
            >::type strategy_type;

        return dispatch::distance<Geometry1, Geometry2>::apply(geometry1, geometry2, strategy_type());
    }
};

} // namespace resolve_strategy


namespace resolve_variant
{

namespace result_of
{

template <typename Geometry1, typename Geometry2, typename Strategy>
struct distance
    : resolve_strategy::result_of::distance
        <
            Geometry1,
            Geometry2,
            Strategy
        >
{};


template <typename Geometry1, BOOST_VARIANT_ENUM_PARAMS(typename T), typename Strategy>
struct distance<Geometry1, variant<BOOST_VARIANT_ENUM_PARAMS(T)>, Strategy>
{
    // A set of all variant type combinations that are compatible and implemented
    typedef typename util::combine_if<
        typename mpl::vector1<Geometry1>,
        typename variant<BOOST_VARIANT_ENUM_PARAMS(T)>::types,
        // Here we want should remove most of the combinations that are not valid
        // mostly to limit the size of the resulting MPL set.
        // But is_implementedn is not ready for prime time
        //
        // util::is_implemented2<mpl::_1, mpl::_2, dispatch::distance<mpl::_1, mpl::_2> >
        mpl::always<mpl::true_>
    >::type possible_input_types;

    // The (possibly variant) result type resulting from these combinations
    typedef typename compress_variant<
        typename transform_variant<
            possible_input_types,
            resolve_strategy::result_of::distance<
                point_type<mpl::first<mpl::_> >,
                point_type<mpl::second<mpl::_> >,
                Strategy
            >,
            mpl::back_inserter<mpl::vector0<> >
        >::type
    >::type type;
};


// Distance arguments are commutative
template <BOOST_VARIANT_ENUM_PARAMS(typename T), typename Geometry2, typename Strategy>
struct distance<variant<BOOST_VARIANT_ENUM_PARAMS(T)>, Geometry2, Strategy>
    : public distance<Geometry2, variant<BOOST_VARIANT_ENUM_PARAMS(T)>, Strategy>
{};


template <BOOST_VARIANT_ENUM_PARAMS(typename T), typename Strategy>
struct distance<variant<BOOST_VARIANT_ENUM_PARAMS(T)>, variant<BOOST_VARIANT_ENUM_PARAMS(T)>, Strategy>
{
    // A set of all variant type combinations that are compatible and implemented
    typedef typename util::combine_if
        <
            typename variant<BOOST_VARIANT_ENUM_PARAMS(T)>::types,
            typename variant<BOOST_VARIANT_ENUM_PARAMS(T)>::types,
            // Here we want to try to remove most of the combinations that are not valid
            // mostly to limit the size of the resulting MPL vector.
            // But is_implementedn is not ready for prime time
            //
            // util::is_implemented2<mpl::_1, mpl::_2, dispatch::distance<mpl::_1, mpl::_2> >
            mpl::always<mpl::true_>
        >::type possible_input_types;

    // The (possibly variant) result type resulting from these combinations
    typedef typename compress_variant<
        typename transform_variant<
            possible_input_types,
            resolve_strategy::result_of::distance<
                point_type<mpl::first<mpl::_> >,
                point_type<mpl::second<mpl::_> >,
                Strategy
            >,
            mpl::back_inserter<mpl::vector0<> >
        >::type
    >::type type;
};

} // namespace result_of


template <typename Geometry1, typename Geometry2>
struct distance
{
    template <typename Strategy>
    static inline typename result_of::distance<Geometry1, Geometry2, Strategy>::type
    apply(
        const Geometry1& geometry1,
        const Geometry2& geometry2,
        Strategy const& strategy)
    {
        return resolve_strategy::distance::apply(geometry1, geometry2, strategy);
    }
};


template <BOOST_VARIANT_ENUM_PARAMS(typename T), typename Geometry2>
struct distance<variant<BOOST_VARIANT_ENUM_PARAMS(T)>, Geometry2>
{
    template <typename Strategy>
    struct visitor: static_visitor
        <
            typename result_of::distance
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
        typename result_of::distance<Geometry1, Geometry2, Strategy>::type
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
    static inline typename result_of::distance
        <
            variant<BOOST_VARIANT_ENUM_PARAMS(T)>,
            Geometry2,
            Strategy
        >::type
    apply(variant<BOOST_VARIANT_ENUM_PARAMS(T)> const& geometry1,
          Geometry2 const& geometry2,
          Strategy const& strategy)
    {
        return apply_visitor(visitor<Strategy>(geometry2, strategy), geometry1);
    }
};


template <typename Geometry1, BOOST_VARIANT_ENUM_PARAMS(typename T)>
struct distance<Geometry1, variant<BOOST_VARIANT_ENUM_PARAMS(T)> >
{
    template <typename Strategy>
    struct visitor: static_visitor
        <
            typename result_of::distance
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
        typename result_of::distance<Geometry1, Geometry2, Strategy>::type
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
    static inline typename result_of::distance
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
        return apply_visitor(visitor<Strategy>(geometry1, strategy), geometry2);
    }
};


template <BOOST_VARIANT_ENUM_PARAMS(typename A), BOOST_VARIANT_ENUM_PARAMS(typename B)>
struct distance<variant<BOOST_VARIANT_ENUM_PARAMS(A)>, variant<BOOST_VARIANT_ENUM_PARAMS(B)> >  
{
    template <typename Strategy>
    struct visitor: static_visitor
        <
            typename result_of::distance
                <
                    variant<BOOST_VARIANT_ENUM_PARAMS(A)>,
                    variant<BOOST_VARIANT_ENUM_PARAMS(B)>,
                    Strategy
                >
                ::type
        >
    {
        Strategy const& m_strategy;

        visitor(Strategy const& strategy)
            : m_strategy(strategy)
        {}

        template <typename Geometry1, typename Geometry2>
        typename result_of::distance<Geometry1, Geometry2, Strategy>::type
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
    static inline typename result_of::distance
        <
            variant<BOOST_VARIANT_ENUM_PARAMS(A)>,
            variant<BOOST_VARIANT_ENUM_PARAMS(B)>,
            Strategy
        >
        ::type
    apply(
        const variant<BOOST_VARIANT_ENUM_PARAMS(A)>& geometry1,
        const variant<BOOST_VARIANT_ENUM_PARAMS(B)>& geometry2,
        Strategy const& strategy)
    {
        return apply_visitor(visitor<Strategy>(strategy), geometry1, geometry2);
    }
};

} // namespace resolve_variant


/*!
\brief \brief_calc2{distance} \brief_strategy
\ingroup distance
\details
\details \details_calc{area}. \brief_strategy. \details_strategy_reasons

\tparam Geometry1 \tparam_geometry
\tparam Geometry2 \tparam_geometry
\tparam Strategy \tparam_strategy{Distance}
\param geometry1 \param_geometry
\param geometry2 \param_geometry
\param strategy \param_strategy{distance}
\return \return_calc{distance}
\note The strategy can be a point-point strategy. In case of distance point-line/point-polygon
    it may also be a point-segment strategy.

\qbk{distinguish,with strategy}

\qbk{
[heading Available Strategies]
\* [link geometry.reference.strategies.strategy_distance_pythagoras Pythagoras (cartesian)]
\* [link geometry.reference.strategies.strategy_distance_haversine Haversine (spherical)]
\* [link geometry.reference.strategies.strategy_distance_cross_track Cross track (spherical\, point-to-segment)]
\* [link geometry.reference.strategies.strategy_distance_projected_point Projected point (cartesian\, point-to-segment)]
\* more (currently extensions): Vincenty\, Andoyer (geographic)
}
 */

/*
Note, in case of a Compilation Error:
if you get:
 - "Failed to specialize function template ..."
 - "error: no matching function for call to ..."
for distance, it is probably so that there is no specialization
for return_type<...> for your strategy.
*/
template <typename Geometry1, typename Geometry2, typename Strategy>
inline typename resolve_variant::result_of::distance
    <
        Geometry1,
        Geometry2,
        Strategy
    >::type
distance(Geometry1 const& geometry1,
         Geometry2 const& geometry2,
         Strategy const& strategy)
{
    concept::check<Geometry1 const>();
    concept::check<Geometry2 const>();

    detail::throw_on_empty_input(geometry1);
    detail::throw_on_empty_input(geometry2);

    return resolve_variant::distance
               <
                   Geometry1,
                   Geometry2
               >::apply(geometry1, geometry2, strategy);
}


/*!
\brief \brief_calc2{distance}
\ingroup distance
\details The default strategy is used, corresponding to the coordinate system of the geometries
\tparam Geometry1 \tparam_geometry
\tparam Geometry2 \tparam_geometry
\param geometry1 \param_geometry
\param geometry2 \param_geometry
\return \return_calc{distance}

\qbk{[include reference/algorithms/distance.qbk]}
 */
template <typename Geometry1, typename Geometry2>
inline typename resolve_variant::result_of::distance
    <
        Geometry1,
        Geometry2,
        default_strategy
    >::type
distance(Geometry1 const& geometry1,
         Geometry2 const& geometry2)
{
    concept::check<Geometry1 const>();
    concept::check<Geometry2 const>();

    return distance(geometry1, geometry2, default_strategy());
}

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_DISTANCE_INTERFACE_HPP
