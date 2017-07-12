/*************************************************************************
  > File Name: testcase_begin_equator.cpp
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

#include <boost/geometry/formulas/cartesine_distance.hpp>
#include <boost/geometry/formulas/compare_length_andoyer.hpp>
#include <boost/geometry/formulas/compare_length_haversine.hpp>
#include <boost/geometry/formulas/flat_approximation.hpp>

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
int main()
{
    double lon1, lat1, lon2, lat2, lon3, lat3, lon4, lat4;
    freopen("tc_begin_equator.in", "r", stdin);
    freopen("tc_begin_equator.out", "w", stdout);
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

        int cg_distance_result ;
        double secs_comparable_geographic_distance;

        if (lon1 != lon3 || lat1 != lat3)
        {
            std::clock_t comparable_geographic_distance_start = std::clock();

            for (int i=0; i<MAX; i++)
            {
                cg_distance_result = bg::formula::compare_length_haversine<double>
                                                          ::apply(P1, P2, P3, P4, earth);
            }
            std::clock_t comparable_geographic_distance_stop = std::clock();
            secs_comparable_geographic_distance = double(comparable_geographic_distance_stop -
                                                         comparable_geographic_distance_start)
                                                         / (double)CLOCKS_PER_SEC;
            if(distance_result != cg_distance_result){
                 std::cout << "comparable_geographic_distancie(Haversine):" 
                           << cg_distance_result << std::endl;
                 std::cout << "distance: " << distance_result << std::endl;
                 std::cout << "P1: " << lon1 << ", " << lat1 
                           << " P2: " << lon2 << ", " << lat2 << std::endl;
                 std::cout << "P3: " << lon3 << ", " << lat3 
                           << " P4: " << lon4 << ", " << lat4 << std::endl;
                 
                 std::cout << "consistency:" 
                           << (distance_result == cg_distance_result)
                           << std::endl;

                 std::cout << "distance (sec):" << secs_distance << std::endl;
                 std::cout << "comparable_geographic_distance (sec):" 
                           << secs_comparable_geographic_distance 
                           << std::endl << std::endl;
            }
        }
    }
    return 0;
}
