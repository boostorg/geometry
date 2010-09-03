// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_NOT_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_NOT_HPP

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail
{



/*!
\brief Structure negating the result of specified policy
\tparam Geometry1 \tparam_geometry
\tparam Geometry2 \tparam_geometry
\tparam Policy
\param geometry1 \param_geometry
\param geometry2 \param_geometry
\return Negation of the result of the policy
 */
template <typename Geometry1, typename Geometry2, typename Policy>
struct not_
{
    static inline bool apply(Geometry1 const &geometry1, Geometry2 const& geometry2)
    {
        return ! Policy::apply(geometry1, geometry2);
    }
};


} // namespace detail
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_NOT_HPP
