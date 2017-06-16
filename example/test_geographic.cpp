/*************************************************************************
    > File Name: test_geographic.cpp
    > Author: Rylynnn
    > Mail: jingry0321@gmail.com
    > Created Time: Wed 14 Jun 2017 06:38:20 AM CST
 ************************************************************************/
#include <ctime>

#include <iostream>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/algorithms/distance.hpp>
#include <boost/geometry/algorithms/comparable_distance.hpp>

#include <boost/numeric/conversion/bounds.hpp>

#define MAX 1000

namespace bg = boost::geometry;

int main()
{

    typedef bg::model::point
            <double, 3,
             bg::cs::geographic<bg::degree>
            > point_type;

    point_type P1(0, 0, 0);
    point_type P2(10, 10, 10);
    point_type P3(0, 0.1, 9);
    point_type P4(10, 10.1, 10.0);

    double const earth_a = 6378.137;
	double const earth_b = 6356.752314245;

    std::clock_t distance_start = std::clock();
    double distance_result1;
    double distance_result2;

    for (int i=0; i<MAX; i++)
    {
         distance_result1 = bg::distance
                (P1, P2, bg::strategy::distance::vincenty<double>(earth_a, earth_b));
         distance_result2 = bg::distance
                (P3, P4, bg::strategy::distance::vincenty<double>(earth_a, earth_b));
    }
    std::clock_t distance_stop = std::clock();
    double secs_distance = double(distance_stop - distance_start) / (double)CLOCKS_PER_SEC;


    std::clock_t comparable_geographic_distance_start = std::clock();
    double comparable_geographic_distance_result1;
    double comparable_geographic_distance_result2;

    for (int i=0; i<MAX; i++)
    {
         comparable_geographic_distance_result1 = bg::comparable_geographic_distance
                (P1, P2, bg::strategy::distance::vincenty<double>(earth_a, earth_b));
         comparable_geographic_distance_result2 = bg::comparable_geographic_distance
                (P3, P4, bg::strategy::distance::vincenty<double>(earth_a, earth_b));
    }
    std::clock_t comparable_geographic_distance_stop = std::clock();
    double secs_comparable_geographic_distance = double(comparable_geographic_distance_stop - comparable_geographic_distance_start)
                                    / (double)CLOCKS_PER_SEC;


    std::cout << "distance 1:" << distance_result1 << std::endl;
    std::cout << "comparable_geographic_distance 1:" << comparable_geographic_distance_result1 << std::endl;

    std::cout << "distance 2:" << distance_result2 << std::endl;
    std::cout << "comparable_geographic_distance 2:" << comparable_geographic_distance_result2 << std::endl;

    std::cout << "Consistency:" << ((distance_result1 < distance_result2)
                                == (comparable_geographic_distance_result1 < comparable_geographic_distance_result2))
              << std::endl;

    std::cout << "distance (sec):" << secs_distance << std::endl;
    std::cout << "comparable_geographic_distance (sec):" << secs_comparable_geographic_distance << std::endl;

    return 0;
}
