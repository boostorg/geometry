// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2013, 2014.
// Modifications copyright (c) 2013-2014 Oracle and/or its affiliates.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_RANGE_HELPERS_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_RANGE_HELPERS_HPP

#include <boost/range.hpp>

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace range {

template <typename Range>
inline typename boost::range_value<Range>::type const&
front(Range const& rng)
{
    BOOST_ASSERT(!boost::empty(rng));
    return *boost::begin(rng);
}

template <typename Range>
inline typename boost::range_value<Range>::type &
front(Range & rng)
{
    BOOST_ASSERT(!boost::empty(rng));
    return *boost::begin(rng);
}

template <typename Range>
inline typename boost::range_value<Range>::type const&
back(Range const& rng)
{
    BOOST_ASSERT(!boost::empty(rng));
    return *(--boost::end(rng));
}

template <typename Range>
inline typename boost::range_value<Range>::type &
back(Range & rng)
{
    BOOST_ASSERT(!boost::empty(rng));
    return *(--boost::end(rng));
}

}} // namespace detail::range
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_RANGE_HELPERS_HPP
