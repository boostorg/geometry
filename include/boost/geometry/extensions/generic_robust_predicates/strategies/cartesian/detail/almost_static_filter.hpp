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

template <typename Filter, std::size_t N, std::size_t End>
struct make_filter_impl
{
    template <typename ExtremaArray, typename ...Reals>
    static Filter apply(const ExtremaArray& extrema, const Reals&... args)
    {
        return make_filter_impl<Filter, N + 1, End>
            ::apply(extrema, args..., extrema[N]);
    }
};

template <typename Filter, std::size_t End>
struct make_filter_impl<Filter, End, End>
{
    template <typename ExtremaArray, typename ...Reals>
    static Filter apply(const ExtremaArray& extrema, const Reals&... args)
    {
        Filter f(args...);
        return f;
    }
};

template
<
    typename Expression,
    typename CalculationType,
    typename StaticFilter
>
class almost_static_filter
{
private:
    using expression_max_argn = max_argn<Expression>;
    using ct = CalculationType;
    using extrema_array = std::array<ct, 2 * expression_max_argn::value>;
    extrema_array m_extrema;
    StaticFilter m_filter;
public:
    const StaticFilter& filter() const { return m_filter; }
    inline almost_static_filter()
    {
        std::fill(m_extrema.begin(),
                  m_extrema.begin() + m_extrema.size() / 2,
                  -std::numeric_limits<ct>::infinity());
        std::fill(m_extrema.begin() + m_extrema.size() / 2,
                  m_extrema.end(),
                  std::numeric_limits<ct>::infinity());
    }
    template <typename ...Reals>
    int apply(const Reals&... args) const
    {
        return m_filter.apply(args...);
    }

    template <typename ...Reals>
    inline void update_extrema(const Reals&... args)
    {
        std::array<ct, sizeof...(Reals)> input {{ static_cast<ct>(args)... }};
        for(int i = 0; i < m_extrema.size() / 2; ++i)
        {
            m_extrema[i] = std::max(m_extrema[i], input[i]);
        }
        for(int i = m_extrema.size() / 2; i < m_extrema.size(); ++i)
        {
            m_extrema[i] = std::min(m_extrema[i], input[i]);
        }
    }

    template <typename ...Reals>
    inline bool update_extrema_check(const Reals&... args)
    {
        bool changed = false;
        std::array<ct, sizeof...(Reals)> input {{ static_cast<ct>(args)... }};
        for(int i = 0; i < m_extrema.size() / 2; ++i)
        {
            if(input[i] > m_extrema[i])
            {
                changed = true;
                m_extrema[i] = input[i];
            }
        }
        for(int i = m_extrema.size() / 2; i < m_extrema.size(); ++i)
        {
            if(input[i] < m_extrema[i])
            {
                changed = true;
                m_extrema[i] = input[i];
            }
        }
        return changed;
    }

    inline void update_filter()
    {
        m_filter = make_filter_impl
                <
                    StaticFilter,
                    0,
                    2 * expression_max_argn::value
                >::apply(m_extrema);
    }
};

}} // namespace detail::generic_robust_predicates

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_ALMOST_STATIC_FILTER_HPP
