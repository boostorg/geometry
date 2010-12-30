// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// Custom triangle template Example

#include <iostream>

#include <boost/array.hpp>
#include <boost/tuple/tuple.hpp>

#include <boost/geometry/algorithms/area.hpp>
#include <boost/geometry/algorithms/centroid.hpp>
#include <boost/geometry/geometries/adapted/tuple.hpp>
#include <boost/geometry/geometries/adapted/tuple_cartesian.hpp>
#include <boost/geometry/geometries/register/ring.hpp>
#include <boost/geometry/strategies/strategies.hpp>
#include <boost/geometry/util/write_dsv.hpp>



template <typename P>
struct triangle : public boost::array<P, 3>
{
};

// Register triangle<P>
BOOST_GEOMETRY_REGISTER_RING_TEMPLATIZED(triangle)


namespace boost { namespace geometry { namespace dispatch {

// Specializations of area dispatch structure, implement algorithm
template<typename P, typename S>
struct area<ring_tag, triangle<P>, S>
{
    static inline double apply(triangle<P> const& t, S const&)
    {
        return 0.5  * ((get<0>(t[1]) - get<0>(t[0])) * (get<1>(t[2]) - get<1>(t[0]))
                     - (get<0>(t[2]) - get<0>(t[0])) * (get<1>(t[1]) - get<1>(t[0])));
    }
};

}}} // namespace boost::geometry::dispatch


int main()
{
    //triangle<boost::geometry::point_xy<double> > t;
    triangle<boost::tuple<double, double> > t;
    t[0] = boost::make_tuple(0, 0);
    t[1] = boost::make_tuple(5, 0);
    t[2] = boost::make_tuple(2.5, 2.5);

    std::cout << "Triangle: " << boost::geometry::dsv(t) << std::endl;
    std::cout << "Area: " << boost::geometry::area(t) << std::endl;

    //boost::geometry::point_xy<double> c;
    boost::tuple<double, double> c;
    boost::geometry::centroid(t, c);
    std::cout << "Centroid: " << boost::geometry::dsv(c) << std::endl;

    return 0;
}
