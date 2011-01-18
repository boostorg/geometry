// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Akira Takahashi 2011
// Copyright Barend Gehrels 2011, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2011
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_GEOMETRIES_ADAPTED_FUSION_GEOGRAPHIC_HPP
#define BOOST_GEOMETRY_GEOMETRIES_ADAPTED_FUSION_GEOGRAPHIC_HPP

#ifdef BOOST_GEOMETRY_ADAPTED_FUSION_COORDINATE_SYSTEM_DEFINED
#error Include only one headerfile to register coordinate coordinate_system for adapted fusion
#endif

#define BOOST_GEOMETRY_ADAPTED_FUSION_COORDINATE_SYSTEM_DEFINED


#include <boost/geometry/geometries/adapted/fusion.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_TRAITS_SPECIALIZATIONS
namespace traits
{
    template <typename Sequence>
    struct coordinate_system<
        Sequence,
        typename boost::enable_if<
            fusion_adapt_detail::is_fusion_sequence<Sequence> >::type>
    { typedef cs::geographic<degree> type; };

} // namespace traits
#endif


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_GEOMETRIES_ADAPTED_FUSION_GEOGRAPHIC_HPP
