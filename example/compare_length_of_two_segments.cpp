/*************************************************************************
    > File Name: test_geographic.cpp
    > Author: Rylynnn
    > Mail: jingry0321@gmail.com
    > Created Time: Wed 14 Jun 2017 06:38:20 AM CST
 ************************************************************************/
#include <ctime>

#include <iostream>

#include <boost/geometry.hpp>
#include <boost/geometry/core/srs.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/algorithms/distance.hpp>
#include <boost/geometry/algorithms/comparable_distance.hpp>
#include <boost/geometry/algorithms/comparable_geographic_distance.hpp>

#include <boost/numeric/conversion/bounds.hpp>

#define MAX 1000
#define eps 1e-6

namespace bg = boost::geometry;

typedef bg::model::point
		<double, 2,
         bg::cs::geographic<bg::degree>
        > point_type;

int compare_length_of_two_segments(point_type p1, point_type p2,
								   point_type p3, point_type p4)
{
	double distance_result1;
	double distance_result2;
	distance_result1 = bg::distance
			(p1, p2, bg::strategy::distance::pythagoras<void>());
	distance_result2 = bg::distance
		    (p3, p4, bg::strategy::distance::pythagoras<void>());
	if (fabs(distance_result1 - distance_result2) > eps && distance_result1 - distance_result2 < 0 )
	{
		return 1;
	}
	else if (fabs(distance_result1 - distance_result2) > eps && distance_result1 - distance_result2 > 0)
	{
		return 2;
	}
	else if (fabs(distance_result1 - distance_result2) <= eps)
	{
		return 3;
	}
}
int main()
{

    point_type P1(0, 0);
    point_type P2(10, 10);
    point_type P3(0, .1);
    point_type P4(10, 10.1);

	bg::srs::spheroid<double> earth;

    std::clock_t distance_start = std::clock();
    double distance_result1;
    double distance_result2;
	double distance_result;

    for (int i=0; i<MAX; i++)
    {
         distance_result1 = bg::distance
                (P1, P2, bg::strategy::distance::vincenty<bg::srs::spheroid<double> >(earth));
         distance_result2 = bg::distance
                (P3, P4, bg::strategy::distance::vincenty<bg::srs::spheroid<double> >(earth));
    }
	if (fabs(distance_result1 - distance_result2) > eps && distance_result1 - distance_result2 < 0 )
	{
		distance_result = 1;
	}
	else if (fabs(distance_result1 - distance_result2) > eps && distance_result1 - distance_result2 > 0)
	{
		distance_result = 2;
	}
	else if (fabs(distance_result1 - distance_result2) <= eps)
	{
		distance_result = 3;
	}
    std::clock_t distance_stop = std::clock();
    double secs_distance = double(distance_stop - distance_start) / (double)CLOCKS_PER_SEC;


    std::clock_t comparable_geographic_distance_start = std::clock();
    double comparable_geographic_distance_result;

    for (int i=0; i<MAX; i++)
    {
		comparable_geographic_distance_result = compare_length_of_two_segments(P1, P2, P3, P4);         
    }
    std::clock_t comparable_geographic_distance_stop = std::clock();
    double secs_comparable_geographic_distance = double(comparable_geographic_distance_stop - comparable_geographic_distance_start)
                                    / (double)CLOCKS_PER_SEC;


    std::cout << "comparable_geographic_distance:" << comparable_geographic_distance_result << std::endl;
	std::cout << "distance: " << distance_result << std::endl;

    if (comparable_geographic_distance_result == 1)
	{
		std::cout << "comparable_geographic_distance: P1, P2 less than P3, P4" << std::endl;
	}
	else if (comparable_geographic_distance_result == 2)
	{
		std::cout << "comparable_geographic_distance: P1, P2 lager than P3, P4" << std::endl;
	}
	else if (comparable_geographic_distance_result == 3)
	{
		std::cout << "comparable_geographic_distance: P1, P2 equal to P3, P4" << std::endl;
	}
	std::cout << "consistency:" << (distance_result == comparable_geographic_distance_result)
              << std::endl;

    std::cout << "distance (sec):" << secs_distance << std::endl;
    std::cout << "comparable_geographic_distance (sec):" << secs_comparable_geographic_distance << std::endl;

    return 0;
}
