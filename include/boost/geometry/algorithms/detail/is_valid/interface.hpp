// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014-2015, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_IS_VALID_INTERFACE_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_IS_VALID_INTERFACE_HPP

#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/static_visitor.hpp>
#include <boost/variant/variant_fwd.hpp>

#include <boost/geometry/geometries/concepts/check.hpp>

#include <boost/geometry/algorithms/dispatch/is_valid.hpp>
#include <boost/geometry/policies/is_valid/null_policy.hpp>


namespace boost { namespace geometry
{


namespace resolve_variant {

template <typename Geometry>
struct is_valid
{
    template <typename VisitPolicy>
    static inline bool apply(Geometry const& geometry, VisitPolicy& visitor)
    {
        concept::check<Geometry const>();
        return dispatch::is_valid<Geometry>::apply(geometry, visitor);
    }
};

template <BOOST_VARIANT_ENUM_PARAMS(typename T)>
struct is_valid<boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)> >
{
    template <typename VisitPolicy>
    struct visitor : boost::static_visitor<bool>
    {
        visitor(VisitPolicy& policy) : m_policy(policy) {}

        template <typename Geometry>
        bool operator()(Geometry const& geometry) const
        {
            return is_valid<Geometry>::apply(geometry, m_policy);
        }

        VisitPolicy& m_policy;
    };

    template <typename VisitPolicy>
    static inline bool
    apply(boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)> const& geometry,
          VisitPolicy& policy_visitor)
    {
        return boost::apply_visitor(visitor<VisitPolicy>(policy_visitor),
                                    geometry);
    }
};

} // namespace resolve_variant


/*!
\brief \brief_check{is valid (in the OGC sense)}
\ingroup is_valid
\tparam Geometry \tparam_geometry
\param geometry \param_geometry
\return \return_check{is valid (in the OGC sense), with one exception:
multi-geometries with no elements are considered valid}

\qbk{[include reference/algorithms/is_valid.qbk]}
*/
template <typename Geometry>
inline bool is_valid(Geometry const& geometry)
{
    is_valid_null_policy<> policy_visitor;
    return resolve_variant::is_valid
        <
            Geometry
        >::apply(geometry, policy_visitor);
}


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_IS_VALID_INTERFACE_HPP
