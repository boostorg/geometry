// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Licensed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_TURNS_MODIFY_TURNS_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_TURNS_MODIFY_TURNS_HPP

#include <algorithm>
#include <type_traits>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>

namespace boost { namespace geometry { namespace detail { namespace turns
{

template <typename Container, typename Less>
inline void sort_turns(Container& turns, Less less)
{
    auto const hook = [&](auto& t) -> decltype(t.sort(less), void())
    { t.sort(less); };
    if constexpr (std::is_invocable_v<decltype(hook), Container&>)
    {
        hook(turns);
    }
    else
    {
        std::sort(boost::begin(turns), boost::end(turns), less);
    }
}

// Grouped relate analysis must reorder only the active subrange, not its records.
template <typename Container, typename Iterator, typename Less>
inline void sort_turns(Container& turns, Iterator first, Iterator last, Less less)
{
    auto const hook = [&](auto& t) -> decltype(t.sort(first, last, less), void())
    { t.sort(first, last, less); };
    if constexpr (std::is_invocable_v<decltype(hook), Container&>)
    {
        hook(turns);
    }
    else
    {
        std::sort(first, last, less);
    }
}

template <typename Container, typename Predicate>
inline void erase_turns_if(Container& turns, Predicate predicate)
{
    auto const hook = [&](auto& t) -> decltype(t.erase_if(predicate), void())
    { t.erase_if(predicate); };
    if constexpr (std::is_invocable_v<decltype(hook), Container&>)
    {
        hook(turns);
    }
    else
    {
        turns.erase(std::remove_if(boost::begin(turns), boost::end(turns), predicate),
                    boost::end(turns));
    }
}

template <typename Container, typename Predicate>
inline void unique_turns(Container& turns, Predicate predicate)
{
    auto const hook = [&](auto& t) -> decltype(t.unique(predicate), void())
    { t.unique(predicate); };
    if constexpr (std::is_invocable_v<decltype(hook), Container&>)
    {
        hook(turns);
    }
    else
    {
        turns.erase(std::unique(boost::begin(turns), boost::end(turns), predicate),
                    boost::end(turns));
    }
}

}}}} // namespace boost::geometry::detail::turns

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_TURNS_MODIFY_TURNS_HPP
