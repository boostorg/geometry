// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_GEOMETRIES_ADAPTED_BOOST_POLYGON_RING_HPP
#define BOOST_GEOMETRY_GEOMETRIES_ADAPTED_BOOST_POLYGON_RING_HPP

// Adapts Geometries from Boost.Polygon for usage in Boost.Geometry
// boost::polygon::polygon_data -> boost::geometry::ring

#include <cstddef>


#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/core/coordinate_dimension.hpp>
#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/core/tags.hpp>

#include <boost/polygon/polygon.hpp>

namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_TRAITS_SPECIALIZATIONS
namespace traits
{

template <typename CoordinateType>
struct tag<boost::polygon::polygon_data<CoordinateType> >
{
    typedef ring_tag type;
};


} // namespace traits
#endif // DOXYGEN_NO_TRAITS_SPECIALIZATIONS


}} // namespace boost::geometry



// ----------------------------------------------------------------------------
// Boost.Polygon's polygon is not Boost.Range compatible 
// So support below
// ----------------------------------------------------------------------------

namespace boost
{
    template<typename CoordinateType>
    struct range_iterator<boost::polygon::polygon_data<CoordinateType> >
    {
        typedef typename boost::polygon::polygon_traits
            <
                boost::polygon::polygon_data<CoordinateType>
            >::iterator_type type;
    };

    template<typename CoordinateType>
    struct range_const_iterator<boost::polygon::polygon_data<CoordinateType> >
    {
        typedef typename boost::polygon::polygon_traits
            <
                boost::polygon::polygon_data<CoordinateType>
            >::iterator_type type;
    };

    // RangeEx
    template<typename CoordinateType>
    struct range_size<boost::polygon::polygon_data<CoordinateType> >
    {
        typedef std::size_t type;
    };

} // namespace 'boost'


// Free-standing function for Boost.Range ADP
template<typename CoordinateType>
inline typename boost::polygon::polygon_traits
        <
            boost::polygon::polygon_data<CoordinateType>
        >::iterator_type range_begin(boost::polygon::polygon_data<CoordinateType>& polygon)
{
    return polygon.begin();
}

template<typename CoordinateType>
inline typename boost::polygon::polygon_traits
        <
            boost::polygon::polygon_data<CoordinateType>
        >::iterator_type range_begin(boost::polygon::polygon_data<CoordinateType> const& polygon)
{
    return polygon.begin();
}

template<typename CoordinateType>
inline typename boost::polygon::polygon_traits
        <
            boost::polygon::polygon_data<CoordinateType>
        >::iterator_type range_end(boost::polygon::polygon_data<CoordinateType>& polygon)
{
    return polygon.end();
}

template<typename CoordinateType>
inline typename boost::polygon::polygon_traits
        <
            boost::polygon::polygon_data<CoordinateType>
        >::iterator_type range_end(boost::polygon::polygon_data<CoordinateType> const& polygon)
{
    return polygon.end();
}

// RangeEx
template<typename CoordinateType>
inline std::size_t range_size(boost::polygon::polygon_data<CoordinateType> const& polygon)
{
    return polygon.size();
}


#endif // BOOST_GEOMETRY_GEOMETRIES_ADAPTED_BOOST_POLYGON_RING_HPP

