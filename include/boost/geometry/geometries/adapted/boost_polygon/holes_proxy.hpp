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

#include <boost/geometry/geometries/adapted/boost_polygon/hole_iterator.hpp>
#include <boost/geometry/geometries/adapted/boost_polygon/ring_proxy.hpp>


namespace boost { namespace geometry
{

namespace adapt { namespace bp
{


// Polygon should implement the boost::polygon::polygon_with_holes_concept
// Specify constness in the template parameter if necessary
template<typename Polygon>
struct holes_proxy
{
    typedef ring_proxy
        <
            typename boost::mpl::if_
                <
                    typename boost::is_const<Polygon>,
                    Polygon const,
                    Polygon
                >::type
        > proxy_type;
    typedef hole_iterator<Polygon, proxy_type> iterator_type;

    inline holes_proxy(Polygon& p)
        : polygon(p)
    {}

    void clear()
    {
        Polygon empty;
        // Clear the holes
        polygon.set_holes
            (
                boost::polygon::begin_holes(empty),
                boost::polygon::end_holes(empty)
            );
    }

    void resize(std::size_t new_size)
    {
        std::vector<boost::polygon::polygon_data<double> > temporary_copy
            (
                boost::polygon::begin_holes(polygon),
                boost::polygon::end_holes(polygon)
            );
        temporary_copy.resize(new_size);
        polygon.set_holes(temporary_copy.begin(), temporary_copy.end());
    }

    Polygon& polygon;
};


// Support holes_proxy for Boost.Range ADP

// Const versions
template<typename Polygon>
inline typename boost::geometry::adapt::bp::holes_proxy<Polygon const>::iterator_type 
            range_begin(boost::geometry::adapt::bp::holes_proxy<Polygon const> const& proxy)
{
    typename boost::geometry::adapt::bp::holes_proxy<Polygon const>::iterator_type 
            begin(proxy.polygon, boost::polygon::begin_holes(proxy.polygon));
    return begin;
}

template<typename Polygon>
inline typename boost::geometry::adapt::bp::holes_proxy<Polygon const>::iterator_type 
            range_end(boost::geometry::adapt::bp::holes_proxy<Polygon const> const& proxy)
{
    typename boost::geometry::adapt::bp::holes_proxy<Polygon const>::iterator_type 
            end(proxy.polygon, boost::polygon::end_holes(proxy.polygon));
    return end;
}

// Mutable versions
template<typename Polygon>
inline typename boost::geometry::adapt::bp::holes_proxy<Polygon>::iterator_type 
            range_begin(boost::geometry::adapt::bp::holes_proxy<Polygon>& proxy)
{
    typename boost::geometry::adapt::bp::holes_proxy<Polygon>::iterator_type 
            begin(proxy.polygon, boost::polygon::begin_holes(proxy.polygon));
    return begin;
}

template<typename Polygon>
inline typename boost::geometry::adapt::bp::holes_proxy<Polygon>::iterator_type 
            range_end(boost::geometry::adapt::bp::holes_proxy<Polygon>& proxy)
{
    typename boost::geometry::adapt::bp::holes_proxy<Polygon>::iterator_type 
            end(proxy.polygon, boost::polygon::end_holes(proxy.polygon));
    return end;
}


}}

namespace traits
{

template <typename Polygon>
struct clear<adapt::bp::holes_proxy<Polygon> >
{
    static inline void apply(adapt::bp::holes_proxy<Polygon>& proxy)
    {
        proxy.clear();
    }
};

template <typename Polygon>
struct resize<adapt::bp::holes_proxy<Polygon> >
{
    static inline void apply(adapt::bp::holes_proxy<Polygon>& proxy, std::size_t new_size)
    {
        proxy.resize(new_size);
    }
};



} // namespace traits


}}


// Specialize holes_proxy for Boost.Range
namespace boost
{
    template<typename Polygon>
    struct range_mutable_iterator<geometry::adapt::bp::holes_proxy<Polygon> >
    {
        typedef typename geometry::adapt::bp::holes_proxy<Polygon>::iterator_type type;
    };

    template<typename Polygon>
    struct range_const_iterator<geometry::adapt::bp::holes_proxy<Polygon> >
    {
        typedef typename geometry::adapt::bp::holes_proxy<Polygon const>::iterator_type type;
    };

} // namespace boost


#endif // BOOST_GEOMETRY_GEOMETRIES_ADAPTED_BOOST_POLYGON_HOLES_PROXY_HPP
