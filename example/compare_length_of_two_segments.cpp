/*************************************************************************
  > File Name: compare_length_of_two_segments.cpp
  > Author: Rylynnn
  > Mail: jingry0321@gmail.com
  > Created Time: Wed 14 Jun 2017 06:38:20 AM CST
 ************************************************************************/
#include <ctime>
#include <cmath>

#include <iostream>

#include <boost/geometry/core/srs.hpp>
#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/radian_access.hpp>

#include <boost/geometry/util/math.hpp>

#include <boost/geometry/geometries/geometries.hpp>

#include <boost/geometry/algorithms/distance.hpp>
#include <boost/geometry/algorithms/comparable_distance.hpp>
#include <boost/geometry/algorithms/comparable_geographic_distance.hpp>

#include <boost/numeric/conversion/bounds.hpp>

#define MAX 1000
#define EPS 1e-9

namespace bg = boost::geometry;

typedef bg::model::point
    <double, 2,
	bg::cs::geographic<bg::degree>
	> point_type;

	double const earth_f = 1 / 298.257223563;
	double const earth_e2 = earth_f * (2 - earth_f);
	double const earth_r = 6317.0;

int compare_length_of_two_segments_local_earth(point_type p1, point_type p2,
					                           point_type p4)
{
	double distance_result1;
	double distance_result2;
	double lon1, lat1, lon2, lat2, lon4, lat4;
	double dlat, dlon, R1, R2, dis_North, dis_East;
	lon1 = bg::get_as_radian<0>(p1);
	lat1 = bg::get_as_radian<1>(p1);
	lon2 = bg::get_as_radian<0>(p2);	
	lat2 = bg::get_as_radian<1>(p2);	
	lon4 = bg::get_as_radian<0>(p4);
	lat4 = bg::get_as_radian<1>(p4);

	dlat = lat2 - lat1;
	dlon = lon2 - lon1;

	R1 = earth_r * (1 - earth_e2) /
		bg::math::sqrt((1 - earth_e2 * bg::math::sqr(sin(lat1)))
				     * (1 - earth_e2 * bg::math::sqr(sin(lat1)))
				     * (1 - earth_e2 * bg::math::sqr(sin(lat1))));
	R2 = earth_r / bg::math::sqrt(1 - earth_e2 * bg::math::sqr(sin(lat1)));

	dis_North =	R1 * dlat;
	dis_East = R2 * cos(lat1) * dlon;

	distance_result1 = bg::math::sqrt(bg::math::sqr(dis_North) + bg::math::sqr(dis_East));

	dlat = lat2 - lat1;
	dlon = lon2 - lon1;

	R1 = earth_r * (1 - earth_e2) / 
		bg::math::sqrt((1 - earth_e2 * bg::math::sqr(sin(lat1)))
				     * (1 - earth_e2 * bg::math::sqr(sin(lat1)))
				     * (1 - earth_e2 * bg::math::sqr(sin(lat1))));
	R2 = earth_r / bg::math::sqrt(1 - earth_e2 * bg::math::sqr(sin(lat1)));

	dis_North =	R1 * dlat;
	dis_East = R2 * cos(lat1) * dlon;

	distance_result2 = bg::math::sqrt(bg::math::sqr(dis_North) + bg::math::sqr(dis_East));

	double result = distance_result1 - distance_result2;

	if (result < -EPS)
	{
		return 1;
	}
	else if (result > EPS)
	{
		return 2;
	}
	else if (fabs(result) < EPS)
	{
		return 3;
	}

}

int compare_length_of_two_segments_haversine(point_type p1, point_type p2,
							                 point_type p3, point_type p4)
{
	double distance_result1;
	double distance_result2;
	double lon1, lat1, lon2, lat2, lon3, lat3, lon4, lat4;
	lon1 = bg::get_as_radian<0>(p1);
	lat1 = bg::get_as_radian<1>(p1);
	lon2 = bg::get_as_radian<0>(p2);	
	lat2 = bg::get_as_radian<1>(p2);	
	lon3 = bg::get_as_radian<0>(p3);
	lat3 = bg::get_as_radian<1>(p3);
	lon4 = bg::get_as_radian<0>(p4);
	lat4 = bg::get_as_radian<1>(p4);
	distance_result1 = bg::math::hav(lat2 - lat1)
		+ cos(lat1) * cos(lat2) * bg::math::hav(lon2 - lon1);
	distance_result2 = bg::math::hav(lat4 - lat3)
		+ cos(lat3) * cos(lat4) * bg::math::hav(lon4 - lon3);

	double result = distance_result1 - distance_result2;

	if (result < -EPS)
	{
		return 1;
	}
	else if (result > EPS)
	{
		return 2;
	}
	else if (fabs(result) < EPS)
	{
		return 3;
	}
}

int compare_length_of_two_segments_cartesian_distance(point_type p1, point_type p2,
							                          point_type p3, point_type p4)
{
	double distance_result1;
	double distance_result2;

	distance_result1 = bg::distance
		(p1, p2, bg::strategy::distance::pythagoras<void>());
	distance_result2 = bg::distance
		(p3, p4, bg::strategy::distance::pythagoras<void>());

	double result = distance_result1 - distance_result2;

	if (result < -EPS)
	{
		return 1;
	}
	else if (result > EPS)
	{
		return 2;
	}
	else if (fabs(result) < EPS)
	{
		return 3;
	}
}
int main()
{
	double lon1, lat1, lon2, lat2, lon3, lat3, lon4, lat4;
	while(std::cin >> lon1 >> lat1 >> lon2 >> lat2 >> lon3 >> lat3 >> lon4 >> lat4){

		point_type P1(lon1, lat1);
		point_type P2(lon2, lat2);	
		point_type P3(lon3, lat3);
		point_type P4(lon4, lon4);

		bg::srs::spheroid<double> earth;

		std::clock_t distance_start = std::clock();
		double distance_result1;
		double distance_result2;
		int distance_result;

		for (int i=0; i<MAX; i++)
		{
			distance_result1 = bg::distance
				(P1, P2, bg::strategy::distance::vincenty<bg::srs::spheroid<double> >(earth));
			distance_result2 = bg::distance
				(P3, P4, bg::strategy::distance::vincenty<bg::srs::spheroid<double> >(earth));
		}
		if (distance_result1 - distance_result2 < -EPS)
		{
			distance_result = 1;
		}
		else if (distance_result1 - distance_result2 > EPS)
		{
			distance_result = 2;
		}
		else if (fabs(distance_result1 - distance_result2) < EPS)
		{
			distance_result = 3;
		}

		std::clock_t distance_stop = std::clock();
		double secs_distance = double(distance_stop - distance_start) / (double)CLOCKS_PER_SEC;

		int comparable_geographic_distance_result;
		double secs_comparable_geographic_distance;

		if (lon1 != lon3 || lat1 != lat3)
		{
			std::clock_t comparable_geographic_distance_start = std::clock();

			for (int i=0; i<MAX; i++)
			{
				comparable_geographic_distance_result = compare_length_of_two_segments_haversine(P1, P2, P3, P4);
			}
			std::clock_t comparable_geographic_distance_stop = std::clock();
			secs_comparable_geographic_distance = double(comparable_geographic_distance_stop -
					comparable_geographic_distance_start)
				/ (double)CLOCKS_PER_SEC;
		}
		else if (lon1 == lon3 && lat1 == lat3)
		{
			std::clock_t comparable_geographic_distance_start = std::clock();

			for (int i=0; i<MAX; i++)
			{
				comparable_geographic_distance_result = compare_length_of_two_segments_local_earth(P1, P2, P4);			
			}
			std::clock_t comparable_geographic_distance_stop = std::clock();
			secs_comparable_geographic_distance = double(comparable_geographic_distance_stop -
					comparable_geographic_distance_start)
				/ (double)CLOCKS_PER_SEC;
		}

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
		std::cout << "comparable_geographic_distance (sec):" << secs_comparable_geographic_distance 
			<< std::endl << std::endl;
	}

	return 0;
}
