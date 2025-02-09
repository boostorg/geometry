// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2020 Tinko Bartels, Berlin, Germany.

// Contributed and/or modified by Tinko Bartels,
//   as part of Google Summer of Code 2020 program.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_STAGED_PREDICATE_HPP
#define BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_STAGED_PREDICATE_HPP

#include <type_traits>
#include <array>
#include <tuple>

#include <boost/mp11/list.hpp>
#include <boost/mp11/algorithm.hpp>
#include <boost/mp11/set.hpp>

#include <boost/geometry/extensions/generic_robust_predicates/strategies/cartesian/detail/expression_tree.hpp>

namespace boost { namespace geometry
{

namespace detail { namespace generic_robust_predicates
{


template <typename RemainingStages>
constexpr bool has_next_and_is_stateful =
    boost::mp11::mp_front<RemainingStages>::stateful;

template <>
constexpr bool has_next_and_is_stateful<boost::mp11::mp_list<>> = false;

template
<
    typename StatefulStages,
    typename RemainingStages,
    bool next_is_stateful = has_next_and_is_stateful<RemainingStages>
>
struct next_stage
{
    template <typename ...CTs>
    static inline int apply(StatefulStages const& stages,
                            CTs const&... args)
    {
        using stage = boost::mp11::mp_front<RemainingStages>;
        int sign = stage::template apply<>(args...);
        if (sign == sign_uncertain)
        {
            return next_stage
                <
                    StatefulStages,
                    boost::mp11::mp_pop_front<RemainingStages>
                >::apply(stages, args...);
        }
        else
        {
            return sign;
        }
    }
};

template
<
    typename StatefulStages,
    typename RemainingStages
>
struct next_stage
    <
        StatefulStages,
        RemainingStages,
        true
    >
{
    template <typename ...CTs>
    static inline int apply(StatefulStages const& stages,
                            CTs const&... args)
    {
        using stage = boost::mp11::mp_front<RemainingStages>;
        int sign = std::get<stage>(stages).apply(args...);
        if (sign == sign_uncertain)
        {
            return next_stage
                <
                    StatefulStages,
                    boost::mp11::mp_pop_front<RemainingStages>
                >::apply(stages, args...);
        }
        else
        {
            return sign;
        }
    }
};

template
<
    typename StatefulStages
>
struct next_stage
    <
        StatefulStages,
        boost::mp11::mp_list<>,
        false
    >
{
    template <typename ...CTs>
    static inline int apply(StatefulStages const&,
                            CTs const&...)
    {
        return sign_uncertain;
    }
};

template <typename Stage> using is_stateful = boost::mp11::mp_bool<Stage::stateful>;

template <typename Stage, bool updates = Stage::updates>
struct construct_stage_impl
{
    template <typename Array> static inline Stage apply(Array const& a)
    {
        Stage out(a);
        return out;
    }
};

template <typename Stage>
struct construct_stage_impl<Stage, true>
{
    template <typename Array> static inline Stage apply(Array const&)
    {
        Stage out;
        return out;
    }
};

template <typename Stages>
struct construct_stages_impl
{
    template <typename Array>
    static inline boost::mp11::mp_rename<Stages, std::tuple>
    apply(Array const& a)
    {
        using stage = boost::mp11::mp_front<Stages>;
        std::tuple<stage> first(construct_stage_impl<stage>::apply(a));
        return std::tuple_cat(first,
                              construct_stages_impl
                                    <
                                        boost::mp11::mp_pop_front<Stages>
                                    >::apply(a));
    }
};

template <>
struct construct_stages_impl<std::tuple<>>
{
    template <typename Array>
    static inline std::tuple<> apply(Array const&)
    {
        return std::tuple<>{};
    }
};

template <typename Stage>
using is_updatable = boost::mp11::mp_bool<Stage::updates>;

template <typename UpdatableStages>
struct update_all_impl
{
    template <typename Stages, typename ...CTs>
    static void apply(Stages& stages, CTs const&... args)
    {
        using current_stage = boost::mp11::mp_front<UpdatableStages>;
        std::get<current_stage>(stages).update(args...);
        update_all_impl<boost::mp11::mp_pop_front<UpdatableStages>>
            ::apply(stages, args...);
    }
};

template <>
struct update_all_impl<std::tuple<>>
{
    template <typename Stages, typename ...CTs>
    static void apply(Stages&, CTs const&...) {}
};

template
<
    typename CalculationType,
    typename ...Stages
>
struct staged_predicate
{
private:
    using ct = CalculationType;
    using stages = std::tuple<Stages...>;
    using stateful_stages = boost::mp11::mp_copy_if<stages, is_stateful>;
    using updatable_stages = boost::mp11::mp_copy_if<stages, is_updatable>;
    stateful_stages m_stages;
    using all_stages = boost::mp11::mp_list<Stages...>;
public:
    static constexpr bool stateful =
        boost::mp11::mp_any_of<stages, is_stateful>::value;
    static constexpr bool updates =
        boost::mp11::mp_any_of<stages, is_updatable>::value;
    template <typename ...CTs>
    inline staged_predicate(CTs const&... args) : m_stages(
            construct_stages_impl<stateful_stages>::apply(
                std::array<ct, sizeof...(args)>{ static_cast<ct>(args)... }
            )) {}

    template <typename ...CTs>
    inline void update(CTs const&... args)
    {
        update_all_impl<updatable_stages>::apply(m_stages, args...);
    }

    template <typename ...CTs>
    inline int apply(CTs const&... args) const
    {
        return next_stage
            <
                stateful_stages,
                all_stages
            >::apply(m_stages, args...);
    }
};

}} // namespace detail::generic_robust_predicates

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_STAGED_PREDICATE_HPP
