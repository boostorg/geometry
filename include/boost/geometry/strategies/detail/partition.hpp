// Boost.Geometry (aka GGL, Generic Geometry Library)
// Copyright (c) 2011-2015 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2017 Adam Wulkiewicz, Lodz, Poland.
// Modifications copyright (c) 2015-2020 Oracle and/or its affiliates.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_DETAIL_PARTITION_HPP
#define BOOST_GEOMETRY_STRATEGIES_DETAIL_PARTITION_HPP

#include <cstddef>
#include <type_traits>

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/coordinate_type.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{
namespace partition
{

// Partitioning is only a search optimization: the split need not represent a
// geometric construction. Keep integral midpoint rounding and overflow safety.
template <typename T, bool IsIntegral = std::is_integral<T>::value>
struct divide_interval
{
    static inline T apply(T const& minimum, T const& maximum)
    {
        static T const two = 2;
        return (minimum + maximum) / two;
    }
};

template <typename T>
struct divide_interval<T, true>
{
    static inline T apply(T const& minimum, T const& maximum)
    {
        return minimum / 2 + maximum / 2 + (minimum % 2 + maximum % 2) / 2;
    }
};

template <std::size_t Dimension, typename Box>
struct divide_box_policy
{
    static void apply(Box const& box, Box& lower_box, Box& upper_box)
    {
        using coordinate = coordinate_type_t<Box>;
        coordinate const midpoint = divide_interval<coordinate>::apply
            (geometry::get<min_corner, Dimension>(box), geometry::get<max_corner, Dimension>(box));
        lower_box = box;
        upper_box = box;
        geometry::set<max_corner, Dimension>(lower_box, midpoint);
        geometry::set<min_corner, Dimension>(upper_box, midpoint);
    }
};

template <std::size_t Dimension, typename Box>
inline void divide_box(Box const& box, Box& lower_box, Box& upper_box)
{
    divide_box_policy<Dimension, Box>::apply(box, lower_box, upper_box);
}

} // namespace partition
}}}} // namespace boost::geometry::strategies::detail

#endif // BOOST_GEOMETRY_STRATEGIES_DETAIL_PARTITION_HPP
