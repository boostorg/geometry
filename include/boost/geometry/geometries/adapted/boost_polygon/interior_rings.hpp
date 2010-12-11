// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_GEOMETRIES_ADAPTED_BOOST_POLYGON_INTERIORS_RINGS_HPP
#define BOOST_GEOMETRY_GEOMETRIES_ADAPTED_BOOST_POLYGON_INTERIORS_RINGS_HPP

#include <boost/polygon/polygon.hpp>


// bp_interiors -> proxy for interior_type, range-of-rings or range-of-ranges
template<typename Polygon>
struct bp_interiors
{
    typedef typename boost::polygon::polygon_traits<Polygon>::coordinate_type coordinate_type;
    typedef boost::polygon::polygon_data<coordinate_type> Ring;
    typedef bp_iterator<Polygon, Ring> iterator_type;


    inline bp_interiors(Polygon const& p)
    {
        this->first = iterator_type(boost::polygon::begin_holes(p));
        this->second = iterator_type(boost::polygon::end_holes(p));
    }

    //void resize(int n) {}
    //bp_ring<Polygon>& back() {}

    iterator_type first, second;
};


// 2. iterators, adapt Boost.Polygon to Boost.Range
namespace boost
{
    template<typename Polygon>
    struct range_iterator<bp_interiors<Polygon> >
    {
        typedef typename bp_interiors<Polygon>::iterator_type type;
    };

    template<typename Polygon>
    struct range_const_iterator<bp_interiors<Polygon> >
    {
        typedef typename bp_interiors<Polygon>::iterator_type type;
    };


} // namespace 'boost'


// 2b. free-standing function for Boost.Range ADP
template<typename Polygon>
inline typename bp_interiors<Polygon>::iterator_type range_begin(bp_interiors<Polygon>& ring)
{
    return ring.first;
}

template<typename Polygon>
inline typename bp_interiors<Polygon>::iterator_type range_begin(bp_interiors<Polygon> const& ring)
{
    return ring.first;
}

template<typename Polygon>
inline typename bp_interiors<Polygon>::iterator_type range_end(bp_interiors<Polygon>& ring)
{
    return ring.second;
}

template<typename Polygon>
inline typename bp_interiors<Polygon>::iterator_type range_end(bp_interiors<Polygon> const& ring)
{
    return ring.second;
}

#endif // BOOST_GEOMETRY_GEOMETRIES_ADAPTED_BOOST_POLYGON_INTERIORS_RINGS_HPP

