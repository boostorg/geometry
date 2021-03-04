// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2020 Tinko Bartels, Berlin, Germany.

// Contributed and/or modified by Tinko Bartels,
//   as part of Google Summer of Code 2020 program.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_ALMOST_STATIC_FILTER_HPP
#define BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_ALMOST_STATIC_FILTER_HPP

#include <array>
#include <algorithm>
#include <limits>

#include <boost/geometry/extensions/generic_robust_predicates/strategies/cartesian/detail/expression_tree.hpp>

namespace boost { namespace geometry
{

namespace detail { namespace generic_robust_predicates
{

//The almost static filter holds an instance of a static filter and applies it
//when it is called. Its static filter can be updated and applied like this:
//
//almost_static_filter<...> f;
//
//f.update(max1, max2, ..., min1, min2, ...);
//
//f.apply(arg1, arg2, ...);
//
//Unlike with the static filter, global bounds do not need to be known at
//construction time and with incremental algorithms where inputs with higher
//magnitude are added later, the earlier applications of the filter may benefit
//from more optimistic error bounds.

template
<
    typename Expression,
    typename CalculationType,
    typename StaticFilter
>
class almost_static_filter
{
private:
    using ct = CalculationType;
    using extrema_array = std::array<ct, 2 * max_argn<Expression>>;
    extrema_array m_extrema;
    StaticFilter m_filter;
public:
    static constexpr bool stateful = true;
    static constexpr bool updates = true;

    StaticFilter const& filter() const { return m_filter; }
    inline almost_static_filter()
    {
        std::fill(m_extrema.begin(),
                  m_extrema.begin() + m_extrema.size() / 2,
                  -std::numeric_limits<ct>::infinity());
        std::fill(m_extrema.begin() + m_extrema.size() / 2,
                  m_extrema.end(),
                  std::numeric_limits<ct>::infinity());
    }
    template <typename ...CTs>
    int apply(CTs const&... args) const
    {
        return m_filter.apply(args...);
    }

    template <typename ...CTs>
    inline void update_extrema(CTs const&... args)
    {
        std::array<ct, sizeof...(CTs)> input {{ static_cast<ct>(args)... }};
        for(unsigned int i = 0; i < m_extrema.size() / 2; ++i)
        {
            m_extrema[i] = std::max(m_extrema[i], input[i]);
        }
        for(unsigned int i = m_extrema.size() / 2; i < m_extrema.size(); ++i)
        {
            m_extrema[i] = std::min(m_extrema[i], input[i]);
        }
    }

    template <typename ...CTs>
    inline bool update_extrema_check(CTs const&... args)
    {
        bool changed = false;
        std::array<ct, sizeof...(CTs)> input {{ static_cast<ct>(args)... }};
        for(unsigned int i = 0; i < m_extrema.size() / 2; ++i)
        {
            if (input[i] > m_extrema[i])
            {
                changed = true;
                m_extrema[i] = input[i];
            }
        }
        for(unsigned int i = m_extrema.size() / 2; i < m_extrema.size(); ++i)
        {
            if (input[i] < m_extrema[i])
            {
                changed = true;
                m_extrema[i] = input[i];
            }
        }
        return changed;
    }

    template <typename ...CTs>
    inline void update(CTs const&... args)
    {
        update_extrema(args...);
        update_filter();
    }

    inline void update_filter()
    {
        m_filter = StaticFilter(m_extrema);
    }
};

}} // namespace detail::generic_robust_predicates

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_ALMOST_STATIC_FILTER_HPP
