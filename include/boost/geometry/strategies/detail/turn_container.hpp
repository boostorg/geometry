// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Licensed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_GEOMETRY_STRATEGIES_DETAIL_TURN_CONTAINER_HPP
#define BOOST_GEOMETRY_STRATEGIES_DETAIL_TURN_CONTAINER_HPP

#include <algorithm>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{

namespace turn_container_detail
{

template <typename DefaultContainer, typename Strategy, typename Geometry1, typename Geometry2>
inline auto make(Strategy const& strategy, Geometry1 const& geometry1,
                 Geometry2 const& geometry2, int)
    -> decltype(strategy.template make_turn_container<DefaultContainer>(geometry1, geometry2))
{
    return strategy.template make_turn_container<DefaultContainer>(geometry1, geometry2);
}

template <typename DefaultContainer, typename Strategy, typename Geometry1, typename Geometry2>
inline DefaultContainer make(Strategy const&, Geometry1 const&, Geometry2 const&, long)
{
    return DefaultContainer{};
}

template <typename Container, typename Less>
inline auto sort(Container& turns, Less less, int) -> decltype(turns.sort(less), void())
{
    turns.sort(less);
}

template <typename Container, typename Less>
inline void sort(Container& turns, Less less, long)
{
    std::sort(boost::begin(turns), boost::end(turns), less);
}

template <typename Container, typename Iterator, typename Less>
inline auto sort(Container& turns, Iterator first, Iterator last, Less less, int)
    -> decltype(turns.sort(first, last, less), void())
{
    turns.sort(first, last, less);
}

template <typename Container, typename Iterator, typename Less>
inline void sort(Container&, Iterator first, Iterator last, Less less, long)
{
    std::sort(first, last, less);
}

template <typename Container, typename Predicate>
inline auto erase_if(Container& turns, Predicate predicate, int)
    -> decltype(turns.erase_if(predicate), void())
{
    turns.erase_if(predicate);
}

template <typename Container, typename Predicate>
inline void erase_if(Container& turns, Predicate predicate, long)
{
    turns.erase(std::remove_if(boost::begin(turns), boost::end(turns), predicate),
                boost::end(turns));
}

template <typename Container, typename Predicate>
inline auto unique(Container& turns, Predicate predicate, int)
    -> decltype(turns.unique(predicate), void())
{
    turns.unique(predicate);
}

template <typename Container, typename Predicate>
inline void unique(Container& turns, Predicate predicate, long)
{
    turns.erase(std::unique(boost::begin(turns), boost::end(turns), predicate),
                boost::end(turns));
}

struct empty_model_turn_scope {};

template <bool Reverse1, bool Reverse2, typename Strategy, typename Turn>
inline auto model_scope(Strategy const& strategy, Turn const& turn, int)
    -> decltype(strategy.template model_turn_scope<Reverse1, Reverse2>(turn))
{
    return strategy.template model_turn_scope<Reverse1, Reverse2>(turn);
}

template <bool Reverse1, bool Reverse2, typename Strategy, typename Turn>
inline empty_model_turn_scope model_scope(Strategy const&, Turn const&, long)
{
    return {};
}

} // namespace turn_container_detail

template <typename DefaultContainer, typename Strategy, typename Geometry1, typename Geometry2>
inline decltype(auto) make_turn_container(Strategy const& strategy,
                                        Geometry1 const& geometry1, Geometry2 const& geometry2)
{
    return turn_container_detail::make<DefaultContainer>(strategy, geometry1, geometry2, 0);
}

template <typename Container, typename Less>
inline void sort_turns(Container& turns, Less less)
{
    turn_container_detail::sort(turns, less, 0);
}

// Grouped relate analysis must reorder only the active subrange, not its records.
template <typename Container, typename Iterator, typename Less>
inline void sort_turns(Container& turns, Iterator first, Iterator last, Less less)
{
    turn_container_detail::sort(turns, first, last, less, 0);
}

template <typename Container, typename Predicate>
inline void erase_turns_if(Container& turns, Predicate predicate)
{
    turn_container_detail::erase_if(turns, predicate, 0);
}

template <typename Container, typename Predicate>
inline void unique_turns(Container& turns, Predicate predicate)
{
    turn_container_detail::unique(turns, predicate, 0);
}

template <typename Strategy, typename Apply>
inline auto with_turn_strategy(Strategy const& s, Apply const& apply, int)
    -> decltype(s.with_turn_strategy(apply))
{ return s.with_turn_strategy(apply); }

template <typename Strategy, typename Apply>
inline void with_turn_strategy(Strategy const& s, Apply const& apply, long)
{ apply(s); }

template <bool Reverse1 = false, bool Reverse2 = false, typename Strategy, typename Turn>
inline decltype(auto) model_turn_scope(Strategy const& strategy, Turn const& turn)
{
    return turn_container_detail::model_scope<Reverse1, Reverse2>(strategy, turn, 0);
}

}}}} // namespace boost::geometry::strategies::detail

#endif // BOOST_GEOMETRY_STRATEGIES_DETAIL_TURN_CONTAINER_HPP
