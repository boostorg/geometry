// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Licensed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_WORKING_OUTPUT_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_WORKING_OUTPUT_HPP

#include <type_traits>
#include <boost/geometry/strategies/relate/intersection_types.hpp>
#include <boost/geometry/strategies/detail/operation_binding.hpp>

namespace boost { namespace geometry { namespace detail { namespace overlay
{

namespace working_output_detail
{
template <typename Working, typename Collection, typename Strategy, typename A, typename B>
auto append(Working const& working, Collection& output, Strategy const& strategy, A const& a, B const& b, int)
    -> decltype(strategy.append_output(working, output, a, b), void())
{ strategy.append_output(working, output, a, b); }

template <typename Working, typename Collection, typename Strategy, typename A, typename B>
void append(Working const& working, Collection& output, Strategy const& strategy, A const&, B const&, long)
{ strategy.append_output(working, output); }
}

template <typename Collection, typename Strategy, typename A, typename B, typename Apply,
          typename Working = typename strategies::relate::services::overlay_output_type
              <Strategy, Collection>::type>
inline bool with_working_output(Collection& output, Strategy const&, A const&, B const&, Apply apply,
    std::enable_if_t<std::is_same<Collection, Working>::value>* = nullptr)
{
    typename strategies::relate::services::operation_scope<Strategy>::type scope;
    (void) scope;
    return apply(output);
}

template <typename Collection, typename Strategy, typename A, typename B, typename Apply,
          typename Working = typename strategies::relate::services::overlay_output_type
              <Strategy, Collection>::type>
inline bool with_working_output(Collection& output, Strategy const& strategy, A const& a, B const& b, Apply apply,
    std::enable_if_t<!std::is_same<Collection, Working>::value>* = nullptr)
{
    typename strategies::relate::services::operation_scope<Strategy>::type scope;
    (void) scope;
    Working working;
    bool const result = apply(working);
    working_output_detail::append(working, output, strategy, a, b, 0);
    return result;
}

}}}} // namespace boost::geometry::detail::overlay

#endif
