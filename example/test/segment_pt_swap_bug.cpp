#include <iostream>
#include <string>


#define BOOST_GEOMETRY_DEBUG_GEOGRAPHIC_CROSS_TRACK
#include <boost/geometry.hpp>

namespace bg = boost::geometry;

int main()
{
    //Spherical pt-box and box-box
    {
        double const earth_radius = 6372795;
        typedef bg::model::point<double, 2,
                bg::cs::spherical_equatorial<bg::degree> > point;

        typedef bg::model::segment<point> segment;

        std::cout << "ps1=" << bg::distance(segment(point(0,2),point(0,3)), point(2.5,3)) << std::endl;
        std::cout << "ps2=" << bg::distance(segment(point(0,3),point(0,2)), point(2.5,3)) << std::endl;

    }
    //Geo pt-box and box-box
    {
        typedef bg::model::point<double, 2,
                bg::cs::geographic<bg::degree> > point;

        typedef bg::model::segment<point> segment;

        std::cout << "ps1=" << bg::distance(segment(point(2,0),point(3,0)), point(2.5,3)) << std::endl;
        std::cout << "ps2=" << bg::distance(segment(point(3,0),point(2,0)), point(2.5,3)) << std::endl;

    }

    return 0;
}

