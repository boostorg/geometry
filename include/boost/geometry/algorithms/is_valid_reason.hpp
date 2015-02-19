// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2015, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ALGORITHMS_IS_VALID_REASON_HPP
#define BOOST_GEOMETRY_ALGORITHMS_IS_VALID_REASON_HPP

#include <string>

#include <boost/geometry/algorithms/is_valid.hpp>
#include <boost/geometry/policies/is_valid/failing_reason_policy.hpp>


namespace boost { namespace geometry
{


// MK::need to add documentation
template <typename Geometry>
inline std::string is_valid_reason(Geometry const& geometry)
{
    std::ostringstream sstr;
    failing_reason_policy policy_visitor(sstr);
    resolve_variant::is_valid<Geometry>::apply(geometry, policy_visitor);
    return sstr.str();
}


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_IS_VALID_REASON_HPP
