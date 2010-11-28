// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_NSPHERE_ALGORITHMS_AREA_HPP
#define BOOST_GEOMETRY_EXTENSIONS_NSPHERE_ALGORITHMS_AREA_HPP

#include <boost/math/constants/constants.hpp>

#include <boost/geometry/algorithms/area.hpp>

#include <boost/geometry/extensions/nsphere/core/tags.hpp>



namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace area
{

template<typename C, typename S>
struct circle_area
{
    typedef typename coordinate_type<C>::type coordinate_type;

    // Returning the coordinate precision, but if integer, returning a double
    typedef typename boost::mpl::if_c
            <
                boost::is_integral<coordinate_type>::type::value,
                double,
                coordinate_type
            >::type return_type;

    static inline return_type apply(C const& c, S const&)
    {
        // Currently only works for Cartesian circles
        // Todo: use strategy
        // Todo: use concept
        assert_dimension<C, 2>();

        return_type r = get_radius<0>(c);
        r *= r * boost::math::constants::pi<return_type>();
        return r;
    }
};



}} // namespace detail::area

#endif // DOXYGEN_NO_DETAIL

#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


template <typename Geometry, typename Strategy>
struct area<nsphere_tag, Geometry, Strategy>
    : detail::area::circle_area<Geometry, Strategy>
{};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH



}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_EXTENSIONS_NSPHERE_ALGORITHMS_AREA_HPP
