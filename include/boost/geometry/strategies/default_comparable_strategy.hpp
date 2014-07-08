// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_STRATEGIES_DEFAULT_COMPARABLE_STRATEGY_HPP
#define BOOST_GEOMETRY_STRATEGIES_DEFAULT_COMPARABLE_STRATEGY_HPP


namespace boost { namespace geometry
{

// This is a comparable strategy placeholder type, which is passed by
// the algorithm free functions to the multi-stage resolving
// process. It's resolved into an actual strategy type during the
// resolve_strategy stage, possibly depending on the input geometry
// type(s). This typically happens after the resolve_variant stage, as
// it needs to be based on concrete geometry types - as opposed to
// variant geometry types.

struct default_comparable_strategy {};

}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_STRATEGIES_DEFAULT_COMPARABLE_STRATEGY_HPP
