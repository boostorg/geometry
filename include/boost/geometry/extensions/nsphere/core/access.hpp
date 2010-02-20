// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Bruno Lalande 2008, 2009
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_NSPHERE_CORE_ACCESS_HPP
#define BOOST_GEOMETRY_EXTENSIONS_NSPHERE_CORE_ACCESS_HPP



#include <boost/geometry/core/access.hpp>

#include <boost/geometry/extensions/nsphere/core/tags.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DISPATCH
namespace core_dispatch
{

template <typename Nsphere, typename CoordinateType, std::size_t Dimension>
struct access<nsphere_tag, Nsphere, CoordinateType, Dimension>
{
    static inline CoordinateType get(Nsphere const& nsphere)
    {
        return traits::access<Nsphere, Dimension>::get(nsphere);
    }
    static inline void set(Nsphere& s, CoordinateType const& value)
    {
        traits::access<Nsphere, Dimension>::set(s, value);
    }
};


} // namespace core_dispatch
#endif // DOXYGEN_NO_DISPATCH


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_EXTENSIONS_NSPHERE_CORE_ACCESS_HPP
