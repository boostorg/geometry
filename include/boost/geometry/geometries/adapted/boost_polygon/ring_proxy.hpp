// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_GEOMETRIES_ADAPTED_BOOST_POLYGON_RING_PROXY_HPP
#define BOOST_GEOMETRY_GEOMETRIES_ADAPTED_BOOST_POLYGON_RING_PROXY_HPP

// Adapts Geometries from Boost.Polygon for usage in Boost.Geometry
// boost::polygon::polygon_with_holes_data -> boost::geometry::polygon
//   pair{begin_points, end_points} -> ring_proxy

#include <boost/polygon/polygon.hpp>
#include <boost/range.hpp>



namespace boost { namespace geometry
{

namespace adapt { namespace bp
{


// Polygon should be a "polygon_data" here; NOT polygon_with_holes_data
template<typename Polygon>
struct ring_proxy
{
    typedef typename boost::polygon::polygon_traits
        <
            Polygon
        >::iterator_type iterator_type;

    inline ring_proxy(Polygon const& p)
        : first(boost::polygon::begin_points(p))
        , second(boost::polygon::end_points(p))
    {
    }

    inline ring_proxy(iterator_type begin, iterator_type end)
        : first(begin)
        , second(end)
    {
    }

    iterator_type first, second;
};


// Support geometry::adapt::bp::ring_proxy for Boost.Range ADP
template<typename Polygon>
inline typename boost::geometry::adapt::bp::ring_proxy<Polygon>::iterator_type
            range_begin(boost::geometry::adapt::bp::ring_proxy<Polygon>& ring)
{
    return ring.first;
}

template<typename Polygon>
inline typename boost::geometry::adapt::bp::ring_proxy<Polygon>::iterator_type
            range_begin(boost::geometry::adapt::bp::ring_proxy<Polygon> const& ring)
{
    return ring.first;
}

template<typename Polygon>
inline typename boost::geometry::adapt::bp::ring_proxy<Polygon>::iterator_type
            range_end(boost::geometry::adapt::bp::ring_proxy<Polygon>& ring)
{
    return ring.second;
}

template<typename Polygon>
inline typename boost::geometry::adapt::bp::ring_proxy<Polygon>::iterator_type
            range_end(boost::geometry::adapt::bp::ring_proxy<Polygon> const& ring)
{
    return ring.second;
}


}} // namespace adapt::bp


namespace traits
{

template <typename Polygon>
struct tag<adapt::bp::ring_proxy<Polygon> >
{
    typedef ring_tag type;
};

} // namespace traits

}} // namespace boost::geometry

// Specialize ring_proxy for Boost.Range
namespace boost
{
    template<typename Polygon>
    struct range_iterator<geometry::adapt::bp::ring_proxy<Polygon> >
    {
        typedef typename geometry::adapt::bp::ring_proxy<Polygon>::iterator_type type;
    };

    template<typename Polygon>
    struct range_const_iterator<geometry::adapt::bp::ring_proxy<Polygon> >
    {
        typedef typename geometry::adapt::bp::ring_proxy<Polygon>::iterator_type type;
    };


} // namespace boost


#endif // BOOST_GEOMETRY_GEOMETRIES_ADAPTED_BOOST_POLYGON_RING_PROXY_HPP

