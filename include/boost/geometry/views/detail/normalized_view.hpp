// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2012 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2012 Mateusz Loskot, London, UK.

// This file was modified by Oracle on 2014-2021.
// Modifications copyright (c) 2014-2021 Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_VIEWS_DETAIL_NORMALIZED_VIEW_HPP
#define BOOST_GEOMETRY_VIEWS_DETAIL_NORMALIZED_VIEW_HPP

#include <type_traits>

#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/range/iterator.hpp>

#include <boost/geometry/views/closeable_view.hpp>
#include <boost/geometry/views/reversible_view.hpp>
#include <boost/geometry/util/order_as_direction.hpp>
#include <boost/geometry/util/type_traits_std.hpp>

namespace boost { namespace geometry {


#ifndef DOXYGEN_NO_DETAIL

namespace detail {


template
<
    typename Geometry,
    iterate_direction Direction = geometry::order_as_direction
        <
            geometry::point_order<Geometry>::value
        >::value,
    closure_selector Closure = geometry::closure<Geometry>::value
>
struct reverse_close_view
{
    using reverse_view = detail::reverse_view<Geometry const, Direction>;
    using view = detail::close_view<reverse_view const, Closure>;
    
    explicit inline reverse_close_view(Geometry const& g)
        : m_view(reverse_view(g))
    {}

    using iterator = typename boost::range_iterator<view const>::type;
    using const_iterator = typename boost::range_iterator<view const>::type;

    inline const_iterator begin() const { return boost::begin(m_view); }
    inline const_iterator end() const { return boost::end(m_view); }

private:
    view m_view;
};


template
<
    typename Geometry,
    closure_selector Closure = geometry::closure<Geometry>::value,
    iterate_direction Direction = geometry::order_as_direction
        <
            geometry::point_order<Geometry>::value
        >::value
>
struct close_reverse_view
{
    // These utilities are inconsistent since Closure defines the property of
    // the Geometry while Direction defines what to do with the Geometry.
    using close_view = detail::close_view<Geometry const, Closure>;
    using view = detail::reverse_view<close_view const, Direction>;
    
    explicit inline close_reverse_view(Geometry const& g)
        : m_view(close_view(g))
    {}

    using iterator = typename boost::range_iterator<view const>::type;
    using const_iterator = typename boost::range_iterator<view const>::type;

    inline const_iterator begin() const { return boost::begin(m_view); }
    inline const_iterator end() const { return boost::end(m_view); }

private:
    view m_view;
};


// Other possible names:
//   clockwise_closed_view
template
<
    typename Range,
    order_selector Order = geometry::point_order<Range>::value,
    closure_selector Closure = geometry::closure<Range>::value
>
using clockwise_close_view = reverse_close_view
    <
        Range,
        geometry::order_as_direction<Order>::value,
        Closure
    >;


// Other possible names:
//   closed_clockwise_view
template
<
    typename Range,
    closure_selector Closure = geometry::closure<Range>::value,
    order_selector Order = geometry::point_order<Range>::value    
>
using close_clockwise_view = close_reverse_view
    <
        Range,
        Closure,
        geometry::order_as_direction<Order>::value
    >;


} // namespace detail

#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_VIEWS_DETAIL_NORMALIZED_VIEW_HPP
