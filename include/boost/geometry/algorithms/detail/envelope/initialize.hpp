// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2015, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_ENVELOPE_INITIALIZE_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_ENVELOPE_INITIALIZE_HPP

#include <cstddef>

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/coordinate_dimension.hpp>
#include <boost/geometry/core/coordinate_type.hpp>


namespace boost { namespace geometry
{

namespace detail { namespace envelope
{

template <std::size_t Dimension, std::size_t DimensionCount>
struct initialize_loop
{
    template <typename Box, typename CoordinateType>
    static inline void apply(Box& box,
                             CoordinateType min_value,
                             CoordinateType max_value)
    {
        geometry::set<min_corner, Dimension>(box, min_value);
        geometry::set<max_corner, Dimension>(box, max_value);

        initialize_loop
            <
                Dimension + 1, DimensionCount
            >::apply(box, min_value, max_value);
    }
};

template <std::size_t DimensionCount>
struct initialize_loop<DimensionCount, DimensionCount>
{
    template <typename Box, typename CoordinateType>
    static inline void apply(Box&, CoordinateType, CoordinateType)
    {
    }
};


template
<
    typename Box,
    std::size_t Dimension = 0,
    std::size_t DimensionCount = dimension<Box>::value
>
struct initialize
{
    typedef typename coordinate_type<Box>::type coordinate_type;

    static inline void apply(Box& box,
                             coordinate_type min_value = 1,
                             coordinate_type max_value = -1)
    {
        initialize_loop
            <
                Dimension, DimensionCount
            >::apply(box, min_value, max_value);
    }
};

}} // detail::envelope

}} // boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_ENVELOPE_INITIALIZE_HPP
