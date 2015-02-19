// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2015, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_POLICIES_IS_VALID_NULL_POLICY_HPP
#define BOOST_GEOMETRY_POLICIES_IS_VALID_NULL_POLICY_HPP

#include <boost/geometry/algorithms/validity_failure_type.hpp>


namespace boost { namespace geometry
{


struct is_valid_null_policy
{
    template <validity_failure_type>
    static inline void apply()
    {
    }

    template <validity_failure_type, typename Data>
    static inline void apply(Data const&)
    {
    }

    template <validity_failure_type, typename Data1, typename Data2>
    static inline void apply(Data1 const&, Data2 const&)
    {
    }
};


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_POLICIES_IS_VALID_NULL_POLICY_HPP
