// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_GEOMETRIES_ADAPTED_BOOST_POLYGON_POLYGON_HPP
#define BOOST_GEOMETRY_GEOMETRIES_ADAPTED_BOOST_POLYGON_POLYGON_HPP

// Adapts Geometries from Boost.Polygon for usage in Boost.Geometry
// boost::polygon::polygon_with_holes_data -> boost::geometry::polygon


#include <cstddef>



#include <boost/geometry/core/tags.hpp>
#include <boost/geometry/core/ring_type.hpp>
#include <boost/geometry/core/exterior_ring.hpp>
#include <boost/geometry/core/interior_rings.hpp>

#include <boost/polygon/polygon.hpp>


#include <boost/geometry/geometries/adapted/boost_polygon/ring_type.hpp>
#include <boost/geometry/geometries/adapted/boost_polygon/iterator.hpp>
#include <boost/geometry/geometries/adapted/boost_polygon/interior_rings.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_TRAITS_SPECIALIZATIONS
namespace traits
{

template <typename CoordinateType>
struct tag<boost::polygon::polygon_with_holes_data<CoordinateType> >
{
    typedef polygon_tag type;
};



template <typename CoordinateType>
struct ring_type<boost::polygon::polygon_with_holes_data<CoordinateType> >
{
    typedef bp_ring<boost::polygon::polygon_data<CoordinateType> > type;
};


template <typename CoordinateType>
struct interior_type<boost::polygon::polygon_with_holes_data<CoordinateType> >
{
    // TEMPORARY! This meta-function will be removed, because it is always used in
    // the concept of "range-iterator<interior_type>" so it should define the range-iterator
    typedef typename bp_interiors<boost::polygon::polygon_with_holes_data<CoordinateType> > type;
};


template <typename CoordinateType>
struct exterior_ring<boost::polygon::polygon_with_holes_data<CoordinateType> >
{
    typedef boost::polygon::polygon_with_holes_data<CoordinateType> polygon_type;
    typedef bp_ring<boost::polygon::polygon_data<CoordinateType> > r_type;

    static inline r_type get(polygon_type& p)
    {
        return r_type(p);
    }

    static inline r_type get(polygon_type const& p)
    {
        return r_type(p);
    }
};

template <typename CoordinateType>
struct interior_rings<boost::polygon::polygon_with_holes_data<CoordinateType> >
{
    typedef boost::polygon::polygon_with_holes_data<CoordinateType> polygon_type;
    typedef typename bp_interiors<polygon_type> itype;

    static inline itype get(polygon_type& p)
    {
        return bp_interiors<polygon_type>(p);
    }

    static inline typename itype const get(polygon_type const& p)
    {
        return bp_interiors<polygon_type>(p);
    }
};



} // namespace traits
#endif // DOXYGEN_NO_TRAITS_SPECIALIZATIONS

}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_GEOMETRIES_ADAPTED_BOOST_POLYGON_POLYGON_HPP

