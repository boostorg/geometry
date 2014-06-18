// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ALGORITHMS_IS_SIMPLE_HPP
#define BOOST_GEOMETRY_ALGORITHMS_IS_SIMPLE_HPP

#include <boost/variant/static_visitor.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/variant_fwd.hpp>

#include <boost/geometry/algorithms/detail/is_simple/always_simple.hpp>
#include <boost/geometry/algorithms/detail/is_simple/areal.hpp>
#include <boost/geometry/algorithms/detail/is_simple/linear.hpp>
#include <boost/geometry/algorithms/detail/is_simple/multipoint.hpp>

#include <boost/geometry/algorithms/dispatch/is_simple.hpp>


namespace boost { namespace geometry
{


namespace resolve_variant {

template <typename Geometry>
struct is_simple
{
    static inline bool apply(Geometry const& geometry)
    {
        concept::check<Geometry const>();
        return dispatch::is_simple<Geometry>::apply(geometry);
    }
};

template <BOOST_VARIANT_ENUM_PARAMS(typename T)>
struct is_simple<boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)> >
{
    struct visitor : boost::static_visitor<bool>
    {
        template <typename Geometry>
        bool operator()(Geometry const& geometry) const
        {
            return is_simple<Geometry>::apply(geometry);
        }
    };

    static inline bool
    apply(boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)> const& geometry)
    {
        return boost::apply_visitor(visitor(), geometry);
    }
};

} // namespace resolve_variant



template <typename Geometry>
inline bool is_simple(Geometry const& g)
{
    return resolve_variant::is_simple<Geometry>::apply(g);
}


}} // namespace boost::geometry



#endif // BOOST_GEOMETRY_ALGORITHMS_IS_SIMPLE_HPP
