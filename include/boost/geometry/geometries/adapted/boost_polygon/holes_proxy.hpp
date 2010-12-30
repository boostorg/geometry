// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_GEOMETRIES_ADAPTED_BOOST_POLYGON_HOLES_PROXY_HPP
#define BOOST_GEOMETRY_GEOMETRIES_ADAPTED_BOOST_POLYGON_HOLES_PROXY_HPP

// Adapts Geometries from Boost.Polygon for usage in Boost.Geometry
// boost::polygon::polygon_with_holes_data -> boost::geometry::polygon
//   pair{begin_holes, begin_holes} -> interior_proxy

#include <boost/polygon/polygon.hpp>


namespace boost { namespace geometry
{

namespace adapt { namespace bp
{


template<typename Polygon>
struct holes_proxy
{
    typedef typename boost::polygon::polygon_traits<Polygon>::coordinate_type coordinate_type;
    typedef boost::polygon::polygon_data<coordinate_type> Ring;
    typedef hole_iterator<Polygon, Ring> iterator_type;


    inline holes_proxy(Polygon const& p)
        : first(iterator_type(boost::polygon::begin_holes(p)))
        , second(iterator_type(boost::polygon::end_holes(p)))
    {
    }

    iterator_type first, second;
};


// Support holes_proxy for Boost.Range ADP
template<typename Polygon>
inline typename boost::geometry::adapt::bp::holes_proxy<Polygon>::iterator_type 
            range_begin(boost::geometry::adapt::bp::holes_proxy<Polygon>& ring)
{
    return ring.first;
}

template<typename Polygon>
inline typename boost::geometry::adapt::bp::holes_proxy<Polygon>::iterator_type 
            range_begin(boost::geometry::adapt::bp::holes_proxy<Polygon> const& ring)
{
    return ring.first;
}

template<typename Polygon>
inline typename boost::geometry::adapt::bp::holes_proxy<Polygon>::iterator_type 
            range_end(boost::geometry::adapt::bp::holes_proxy<Polygon>& ring)
{
    return ring.second;
}

template<typename Polygon>
inline typename boost::geometry::adapt::bp::holes_proxy<Polygon>::iterator_type 
            range_end(boost::geometry::adapt::bp::holes_proxy<Polygon> const& ring)
{
    return ring.second;
}


}}}}


// Specialize holes_proxy for Boost.Range
namespace boost
{
    template<typename Polygon>
    struct range_iterator<geometry::adapt::bp::holes_proxy<Polygon> >
    {
        typedef typename geometry::adapt::bp::holes_proxy<Polygon>::iterator_type type;
    };

    template<typename Polygon>
    struct range_const_iterator<geometry::adapt::bp::holes_proxy<Polygon> >
    {
        typedef typename geometry::adapt::bp::holes_proxy<Polygon>::iterator_type type;
    };


} // namespace boost



#endif // BOOST_GEOMETRY_GEOMETRIES_ADAPTED_BOOST_POLYGON_HOLES_PROXY_HPP
