// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef BOOST_GEOMETRY_MULTI_CORE_INTERIOR_RINGS_HPP
#define BOOST_GEOMETRY_MULTI_CORE_INTERIOR_RINGS_HPP


#include <cstddef>

#include <boost/range.hpp>

#include <boost/geometry/core/interior_rings.hpp>
#include <boost/geometry/multi/core/tags.hpp>


namespace boost { namespace geometry
{




#ifndef DOXYGEN_NO_DISPATCH
namespace core_dispatch
{


template <typename MultiPolygon>
struct interior_type<multi_polygon_tag, MultiPolygon>
{
    typedef typename core_dispatch::interior_type
        <
            polygon_tag,
            typename boost::range_value<MultiPolygon>::type
        >::type type;
};



template <typename MultiPolygon>
struct num_interior_rings<multi_polygon_tag, MultiPolygon>
{
    static inline std::size_t apply(MultiPolygon const& multi_polygon)
    {
        std::size_t n = 0;
        for (typename boost::range_iterator<MultiPolygon const>::type
            it = boost::begin(multi_polygon);
            it != boost::end(multi_polygon);
            ++it)
        {
            n += geometry::num_interior_rings(*it);
        }
        return n;
    }

};


} // namespace core_dispatch
#endif



}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_MULTI_CORE_INTERIOR_RINGS_HPP
