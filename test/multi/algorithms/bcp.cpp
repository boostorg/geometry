#include <iostream>
#include <fstream>

#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/multi/algorithms/distance.hpp>
#include <boost/geometry/multi/geometries/multi_point.hpp>
#include <boost/geometry/multi/algorithms/detail/distance/bichromatic_closest_pair.hpp>

#ifdef BOOST_GEOMETRY_USE_TIMER
#include <boost/timer/timer.hpp>
#endif

namespace bg = ::boost::geometry;

typedef bg::model::d2::point_xy<double> point;
typedef bg::model::multi_point<point> multi_point;

typedef bg::detail::distance::bichromatic_closest_pair<multi_point,multi_point>
BCP;

int main(int argc, char** argv)
{
    multi_point mp1, mp2;

    mp1.push_back(point(0,0));
    mp1.push_back(point(1,1));
    mp1.push_back(point(0.5,1));
    mp1.push_back(point(1,0));
    mp1.push_back(point(0,1));

    mp2.push_back(point(-1,-1));
    mp2.push_back(point(-2,-2));
    mp2.push_back(point(-0.5,-1));
    mp2.push_back(point(-1,0));
    mp2.push_back(point(0,-1));

    std::cout << "MP1: " << bg::wkt(mp1) << std::endl;
    std::cout << "MP2: " << bg::wkt(mp2) << std::endl;
    std::cout << "distance (Rabin-like) = "
              << BCP::apply(mp1, mp2) << std::endl;
    std::cout << "distance (brute force) = "
              << bg::distance(mp1, mp2) << std::endl;

    if ( argc == 1 ) { return 0; }

    std::ifstream ifs(argv[1]);
    assert( ifs );

    bg::clear(mp1);
    bg::clear(mp2);

    std::size_t n_red, n_blue;

    ifs >> n_red >> n_blue;

    for (std::size_t i = 0; i < n_red; ++i)
    {
        double x, y;
        ifs >> x >> y;
        mp1.push_back( point(x, y) );
    }

    for (std::size_t i = 0; i < n_blue; ++i)
    {
        double x, y;
        ifs >> x >> y;
        mp2.push_back( point(x, y) );
    }

    std::cout.precision(16);
    std::cout << "MP1 size: " << boost::size(mp1) << std::endl;
    std::cout << "MP2 size: " << boost::size(mp2) << std::endl;
    {
#ifdef BOOST_GEOMETRY_USE_TIMER
        boost::timer::auto_cpu_timer t;
#endif
        std::cout << "distance (Rabin-like) = "
                  << BCP::apply(mp1, mp2) << std::endl;
    }
    {
#ifdef BOOST_GEOMETRY_USE_TIMER
        boost::timer::auto_cpu_timer t;
#endif
        std::cout << "distance (brute force) = "
                  << bg::distance(mp1, mp2) << std::endl;
    }

    return 0;
}
