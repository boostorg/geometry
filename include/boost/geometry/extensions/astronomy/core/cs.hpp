// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSION_ASTRONOMY_CORE_CS_HPP
#define BOOST_GEOMETRY_EXTENSION_ASTRONOMY_CORE_CS_HPP

#include <boost/geometry/core/coordinate_system.hpp>
#include <boost/geometry/core/tags.hpp>


namespace boost { namespace geometry
{


namespace cs
{


namespace celestial
{

/*!
    \brief Ecliptic (celestial) coordinate system
    \details Defines the astronomical ecliptic coordinate system "that uses the ecliptic for its fundamental plane"
    It uses Beta and Lambda as its latitude and longitude.
    \see http://en.wikipedia.org/wiki/Ecliptic_coordinate_system
    \ingroup cs
*/
template<typename DegreeOrRadian>
struct ecliptic
{
    typedef DegreeOrRadian units;
};


} // namespace celestial

} // namespace cs


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_EXTENSION_ASTRONOMY_CORE_CS_HPP
