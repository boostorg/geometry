// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2023 Barend Gehrels, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_PARTITION_LAMBDA_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_PARTITION_LAMBDA_HPP

#include <boost/range.hpp>
#include <boost/geometry/algorithms/detail/partition.hpp>

#include <functional>

namespace boost { namespace geometry { namespace experimental
{

template <typename F>
struct adapt_partition_visitor
{
    F m_f;

    explicit adapt_partition_visitor(F&& f)
        : m_f(std::move(f))
    {}

    template <typename ...Ts>
    decltype(auto) apply(Ts&& ...is) const
    {
        return m_f(std::forward<Ts>(is)...);
    }
};

struct partition_options
{
    // Include policy for the (first) range
    using include_policy1 = detail::partition::include_all_policy;

    // Include policy for the second range
    using include_policy2 = detail::partition::include_all_policy;

    // Defines the end of the iteration (as soon as range 1 or range 2 has
    // these number of elements, or less, it will switch to iterative mode)
    std::size_t min_elements = 16;
};

template
<
    typename BoxType,
    typename ForwardRange1,
    typename ForwardRange2,
    typename Options = partition_options
>
bool partition(ForwardRange1 const& forward_range1,
               ForwardRange2 const& forward_range2,
               const std::function
                   <
                       void(BoxType&,
                            typename boost::range_value<ForwardRange1>::type const&)
                   >& expand_policy1,
               const std::function
                   <
                       bool(BoxType const&,
                            typename boost::range_value<ForwardRange1>::type const&)
                   >& overlaps_policy1,
               const std::function
                   <
                       void(BoxType&,
                            typename boost::range_value<ForwardRange2>::type const&)
                   >& expand_policy2,
               const std::function
                   <
                       bool(BoxType const&,
                            typename boost::range_value<ForwardRange2>::type const&)
                   >& overlaps_policy2,
               const std::function
                   <
                       bool(typename boost::range_value<ForwardRange1>::type const&,
                            typename boost::range_value<ForwardRange2>::type const&)
                   >& visitor,
               const std::function
                   <
                       void(BoxType const&, int level)
                   >& box_visitor = [](auto const&, int) {},
               const Options& options = {})
{
    adapt_partition_visitor<decltype(visitor)> av(visitor);
    adapt_partition_visitor<decltype(expand_policy1)> aev1(expand_policy1);
    adapt_partition_visitor<decltype(expand_policy2)> aev2(expand_policy2);
    adapt_partition_visitor<decltype(overlaps_policy1)> aov1(overlaps_policy1);
    adapt_partition_visitor<decltype(overlaps_policy2)> aov2(overlaps_policy2);
    adapt_partition_visitor<decltype(box_visitor)> apbv(box_visitor);

    return geometry::partition
        <
            BoxType,
            typename Options::include_policy1,
            typename Options::include_policy2
        >::apply(forward_range1, forward_range2, av,
                 aev1, aov1,
                 aev2, aov2, options.min_elements, apbv);
}

template
<
    typename BoxType,
    typename ForwardRange,
    typename Options = partition_options
>
bool partition(ForwardRange const& forward_range,
               const std::function
                   <
                       void(BoxType&,
                            typename boost::range_value<ForwardRange>::type const&)
                   >& expand_policy,
               const std::function
                   <
                       bool(BoxType const&,
                            typename boost::range_value<ForwardRange>::type const&)
                   >& overlaps_policy,
               const std::function
                   <
                       bool(typename boost::range_value<ForwardRange>::type const&,
                            typename boost::range_value<ForwardRange>::type const&)
                   >& visitor,
               const std::function
                   <
                       void(BoxType const&, int level)
                   >& box_visitor = [](auto const&, int) {},
               const Options& options = {})
{
    adapt_partition_visitor<decltype(visitor)> av(visitor);
    adapt_partition_visitor<decltype(expand_policy)> aev(expand_policy);
    adapt_partition_visitor<decltype(overlaps_policy)> aov(overlaps_policy);
    adapt_partition_visitor<decltype(box_visitor)> apbv(box_visitor);

    return geometry::partition
        <
            BoxType,
            typename Options::include_policy1
        >::apply(forward_range, av,
                 aev, aov, options.min_elements, apbv);
}

}}} // namespace boost::geometry::experimental

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_PARTITION_LAMBDA_HPP
