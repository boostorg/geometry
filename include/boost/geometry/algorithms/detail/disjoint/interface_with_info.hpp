// Boost.Geometry

// Copyright (c) 2020, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_DISJOINT_INTERFACE_WITH_INFO_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_DISJOINT_INTERFACE_WITH_INFO_HPP

#include <cstddef>

#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/static_visitor.hpp>
#include <boost/variant/variant_fwd.hpp>

#include <boost/geometry/algorithms/detail/relate/interface.hpp>
#include <boost/geometry/algorithms/dispatch/disjoint_with_info.hpp>

#include <boost/geometry/geometries/concepts/check.hpp>

#include <boost/geometry/strategies/disjoint.hpp>


namespace boost { namespace geometry
{

namespace detail { namespace disjoint
{

template <typename Geometry1, typename Geometry2>
struct disjoint_with_info
{
    typedef typename point_type<Geometry1>::type point_type;

    typedef segment_intersection_points<point_type> intersection_return_type;

    template <typename Strategy>
    static inline intersection_return_type
    apply(Geometry1 const& geometry1,
          Geometry2 const& geometry2,
          Strategy const& strategy)
    {

        return dispatch::disjoint_with_info
                <
                    Geometry1, Geometry2
                >::apply(geometry1, geometry2, strategy);
    }
};


}}}} // namespace boost::geometry::detail


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_DISJOINT_INTERFACE_WITH_INFO_HPP
