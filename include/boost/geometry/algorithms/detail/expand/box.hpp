// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2015 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2015 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2015 Mateusz Loskot, London, UK.
// Copyright (c) 2014-2015 Samuel Debionne, Grenoble, France.

// This file was modified by Oracle on 2015.
// Modifications copyright (c) 2015, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_EXPAND_BOX_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_EXPAND_BOX_HPP

#include <cstddef>
#include <algorithm>

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/tags.hpp>

#include <boost/geometry/algorithms/detail/envelope/box.hpp>
#include <boost/geometry/algorithms/detail/envelope/range_of_boxes.hpp>
#include <boost/geometry/algorithms/detail/expand/indexed.hpp>
#include <boost/geometry/algorithms/dispatch/expand.hpp>


namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace expand
{

template <std::size_t Dimension, std::size_t DimensionCount, typename CS_Tag>
struct box_on_spheroid
{
    template <typename BoxOut, typename BoxIn>
    static inline void apply(BoxOut& box_out, BoxIn const& box_in)
    {
        typedef detail::envelope::envelope_box
            <
                Dimension, DimensionCount, CS_Tag
            > envelope_box;

        // normalize both boxes and convert box-in to be of type of box-out
        BoxOut mbrs[2];
        envelope_box::apply(box_in, mbrs[0]);
        envelope_box::apply(box_out, mbrs[1]);

        // compute the envelope of the two boxes
        detail::envelope::envelope_range_of_boxes
            <
                Dimension, DimensionCount
            >::apply(mbrs, box_out);
    }
};


}} // namespace detail::expand
#endif // DOXYGEN_NO_DETAIL

#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


// Box + box -> new box containing two input boxes
template
<
    typename BoxOut, typename BoxIn,
    std::size_t Dimension, std::size_t DimensionCount,
    typename StrategyLess, typename StrategyGreater,
    typename CSTagOut, typename CSTag
>
struct expand
    <
        BoxOut, BoxIn,
        Dimension, DimensionCount, StrategyLess, StrategyGreater,
        box_tag, box_tag, CSTagOut, CSTag
    > : detail::expand::expand_indexed
        <
           Dimension, DimensionCount, StrategyLess, StrategyGreater
        >
{};

template
<
    typename BoxOut, typename BoxIn,
    std::size_t Dimension, std::size_t DimensionCount,
    typename StrategyLess, typename StrategyGreater
>
struct expand
    <
        BoxOut, BoxIn,
        Dimension, DimensionCount, StrategyLess, StrategyGreater,
        box_tag, box_tag, spherical_equatorial_tag, spherical_equatorial_tag
    > : detail::expand::box_on_spheroid
        <
            Dimension, DimensionCount, spherical_equatorial_tag
        >
{};

template
<
    typename BoxOut, typename BoxIn,
    std::size_t Dimension, std::size_t DimensionCount,
    typename StrategyLess, typename StrategyGreater
>
struct expand
    <
        BoxOut, BoxIn,
        Dimension, DimensionCount, StrategyLess, StrategyGreater,
        box_tag, box_tag, geographic_tag, geographic_tag
    > : detail::expand::box_on_spheroid
        <
            Dimension, DimensionCount, geographic_tag
        >
{};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_EXPAND_INDEXED_HPP
