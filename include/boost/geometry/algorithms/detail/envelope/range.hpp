// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2015 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2015 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2015 Mateusz Loskot, London, UK.

// This file was modified by Oracle on 2015.
// Modifications copyright (c) 2015, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_ENVELOPE_RANGE_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_ENVELOPE_RANGE_HPP

#include <cstddef>

#include <boost/range.hpp>

#include <boost/geometry/util/range.hpp>

#include <boost/geometry/algorithms/is_empty.hpp>

#include <boost/geometry/algorithms/detail/envelope/initialize.hpp>
#include <boost/geometry/algorithms/detail/expand/box.hpp>
#include <boost/geometry/algorithms/detail/expand/point.hpp>
#include <boost/geometry/algorithms/detail/expand/segment.hpp>
#include <boost/geometry/algorithms/dispatch/envelope.hpp>


namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace envelope
{


// implementation for simple ranges
template <std::size_t Dimension, std::size_t DimensionCount>
struct envelope_range
{
    template <typename Range, typename Box>
    static inline void apply(Range const& range, Box& mbr)
    {
        typedef typename boost::range_value<Range>::type value_type;

        // initialize MBR
        initialize<Box, Dimension, DimensionCount>::apply(mbr);

        typename boost::range_iterator<Range const>::type it 
            = boost::begin(range);

        if (it != boost::end(range))
        {
            // initialize box with first element in range
            dispatch::envelope
                <
                    value_type, Dimension, DimensionCount
                >::apply(*it, mbr);

            // consider now the remaining elements in the range (if any)
            for (++it; it != boost::end(range); ++it)
            {
                dispatch::expand
                    <
                        Box, value_type, Dimension, DimensionCount
                    >::apply(mbr, *it);
            }
        }
    }
};


// implementation for multi-ranges
template
<
    std::size_t Dimension,
    std::size_t DimensionCount,
    typename EnvelopePolicy
>
struct envelope_multi_range
{
    template <typename MultiRange, typename Box>
    static inline void apply(MultiRange const& multirange, Box& mbr)
    {
        typedef typename boost::range_iterator
            <
                MultiRange const
            >::type iterator_type;

        // initialize MBR
        initialize<Box, Dimension, DimensionCount>::apply(mbr);

        // skip through empty geometries
        iterator_type it = boost::begin(multirange);
        while (it != boost::end(multirange) && geometry::is_empty(*it))
        {
            ++it;
        }

        // if there are still elements left, apply the envelope policy
        if (it != boost::end(multirange))
        {
            // compute the initial envelope
            EnvelopePolicy::apply(*it, mbr);

            // for the remaining non-empty geometries just expand the mbr
            // using the envelope
            for (++it; it != boost::end(multirange); ++it)
            {
                if (! geometry::is_empty(*it))
                {
                    Box helper_mbr;
                    EnvelopePolicy::apply(*it, helper_mbr);

                    dispatch::expand
                        <
                            Box, Box, Dimension, DimensionCount
                        >::apply(mbr, helper_mbr);
                }
            }
        }
    }
};


}} // namespace detail::envelope
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_ENVELOPE_RANGE_HPP
