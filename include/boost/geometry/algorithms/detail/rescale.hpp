// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2013 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2013 Bruno Lalande, Paris, France.
// Copyright (c) 2013 Mateusz Loskot, London, UK.
// Copyright (c) 2013 Adam Wulkiewicz, Lodz, Poland.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_RESCALE_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_RESCALE_HPP

#include <boost/geometry/algorithms/detail/overlay/segment_ratio.hpp>

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL

// To be removed later, when we will initialize a real policy
namespace detail
{

struct no_rescale_policy
{

    // We don't rescale but return the reference. zero cost.
    template <std::size_t Dimension, typename Value>
    inline Value const& apply(Value const& value) const
    {
        return value;
    }
};

} // namespace detail


// Meta-function to define a robust point type
template <typename Point, typename Policy>
struct robust_point_type {};

// Meta-function to access segment-ratio
template <typename Point, typename Policy>
struct segment_ratio_type {};


// For no-rescaling
template <typename Point>
struct robust_point_type<Point, detail::no_rescale_policy>
{
    typedef Point type;
};

// Implementation for no-rescaling (using coordinate itself)
template <typename Point>
struct segment_ratio_type<Point, detail::no_rescale_policy>
{
    typedef typename geometry::coordinate_type<Point>::type coordinate_type;
    typedef segment_ratio<coordinate_type> type;
//    typedef segment_ratio<boost::long_long_type> type;
};


#endif

}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_RESCALE_HPP
