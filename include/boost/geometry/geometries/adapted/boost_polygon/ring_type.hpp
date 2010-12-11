// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_GEOMETRIES_ADAPTED_BOOST_POLYGON_RING_PROXY_HPP
#define BOOST_GEOMETRY_GEOMETRIES_ADAPTED_BOOST_POLYGON_RING_PROXY_HPP

// Adapts Geometries from Boost.Polygon for usage in Boost.Geometry

#include <boost/range.hpp>
#include <boost/polygon/polygon.hpp>


// bp_ring -> proxy for ring
// Polygon should be a "polygon_data" here; NOT polygon_with_holes_data

// TODO: there should probably be a namespace here. boost::geometry::bp ?

template<typename Polygon>
struct bp_ring : public std::pair<typename Polygon::iterator_type, typename Polygon::iterator_type>
{
    typedef boost::polygon::polygon_traits<Polygon> traits;
    typedef typename traits::coordinate_type coordinate_type;
    typedef typename traits::iterator_type iterator_type;

    inline bp_ring(Polygon const& p)
    {
        this->first = boost::polygon::begin_points(p);
        this->second = boost::polygon::end_points(p);
    }

    inline bp_ring(boost::polygon::polygon_with_holes_data<coordinate_type> const& p)
    {
        this->first = boost::polygon::begin_points(p);
        this->second = boost::polygon::end_points(p);
    }
};


namespace boost { namespace geometry
{

namespace traits
{

template <typename Polygon>
struct tag<bp_ring<Polygon> >
{
    typedef ring_tag type;
};

}

}}

// 2. iterators, adapt Boost.Polygon to Boost.Range
namespace boost
{
    template<typename Polygon>
    struct range_iterator<bp_ring<Polygon> >
    {
        typedef typename bp_ring<Polygon>::iterator_type type;
    };

    template<typename Polygon>
    struct range_const_iterator<bp_ring<Polygon> >
    {
        typedef typename bp_ring<Polygon>::iterator_type type;
    };


} // namespace 'boost'


// 2b. free-standing function for Boost.Range ADP
template<typename Polygon>
inline typename bp_ring<Polygon>::iterator_type range_begin(bp_ring<Polygon>& ring)
{
    return ring.first;
}

template<typename Polygon>
inline typename bp_ring<Polygon>::iterator_type range_begin(bp_ring<Polygon> const& ring)
{
    return ring.first;
}

template<typename Polygon>
inline typename bp_ring<Polygon>::iterator_type range_end(bp_ring<Polygon>& ring)
{
    return ring.second;
}

template<typename Polygon>
inline typename bp_ring<Polygon>::iterator_type range_end(bp_ring<Polygon> const& ring)
{
    return ring.second;
}


#endif // BOOST_GEOMETRY_GEOMETRIES_ADAPTED_BOOST_POLYGON_RING_PROXY_HPP

