#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <boost/lexical_cast.hpp>

#include <boost/geometry.hpp>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/accumulators/statistics/min.hpp>

/** Data from here: https://zenodo.org/record/32156#.WOyaMrUmv7C
 *
  Each line of the test set gives 10 space delimited numbers

  0 latitude at point 1, φ1 (degrees, exact)
  1 longitude at point 1, λ1 (degrees, always 0)
  2 azimuth at point 1, α1 (clockwise from north in degrees, exact)
  3 latitude at point 2, φ2 (degrees, accurate to 10−18 deg)
  4 longitude at point 2, λ2 (degrees, accurate to 10−18 deg)
  5 azimuth at point 2, α2 (degrees, accurate to 10−18 deg)
  6 geodesic distance from point 1 to point 2, s12 (meters, exact)
  7 arc distance on the auxiliary sphere, σ12 (degrees, accurate to 10−18 deg)
  8 reduced length of the geodesic, m12 (meters, accurate to 0.1 pm)
  9 the area between the geodesic and the equator, S12 (m2, accurate to 1 mm2)
 **/

namespace bg = boost::geometry;
namespace ba = boost::accumulators;

template<typename Out>
void split(const std::string &s, Out result)
{
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, ' '))
    {
        try
        {
            *(result++) = boost::lexical_cast<double>(item);
        }
        catch (boost::bad_lexical_cast const&)
        {
            *(result++) = 0;
        }
    }
}


int main()
{
    std::string line;
    std::ifstream myfile ("/home/vissarion/Documents/GIS_algorithms_implementations/data/GeodTest.dat");

    std::cout.precision(10);

    double total_time = 0;

    if (myfile.is_open())
    {
        int count=0;

        size_t const num_of_methods = 3;
        size_t const num_of_values = 6;

        // Define an accumulator set for calculating the mean
        ba::accumulator_set<double, ba::stats<ba::tag::mean,
                                              ba::tag::max,
                                              ba::tag::min> > acc[num_of_methods][num_of_values];

        while ( std::getline(myfile, line) && count++ < 100000 )
        {
            //read data from file
            std::vector<double> data;
            split(line, std::back_inserter(data));
            
            //compute
            typedef bg::model::point<double, 2, bg::cs::geographic<bg::degree> > point;

            double const d2r = bg::math::d2r<double>();
            double const r2d = bg::math::r2d<double>();

            // WGS84
            bg::srs::spheroid<double> spheroid(6378137.0, 6356752.3142451793);

            bg::formula::result_inverse<double> results[3];
            double areas[3];

//            clock_t startTime = clock();
//            for (int j=0; j < 100; j++)
//          {
//                typedef bg::formula::vincenty_inverse<double, true, false, false, false, false> vi_t;
            typedef bg::formula::vincenty_inverse<double, true, true, true, true, true> vi_t;
            results[0] = vi_t::apply(data[1] * d2r, data[0] * d2r,
                                         data[4] * d2r, data[3] * d2r,
                                         spheroid);
//            }
//            total_time += double( clock() - startTime ) / (double)CLOCKS_PER_SEC;

            //thomas
            typedef bg::formula::thomas_inverse<double, true, true, true, true, true> th_t;
            results[1] = th_t::apply(data[1] * d2r, data[0] * d2r,
                                   data[4] * d2r, data[3] * d2r,
                                   spheroid);

            //andoyer
            typedef bg::formula::andoyer_inverse<double, true, true, true, true, true> an_t;
            results[2] = an_t::apply(data[1] * d2r, data[0] * d2r,
                                   data[4] * d2r, data[3] * d2r,
                                   spheroid);

            bg::strategy::area::geographic<point, bg::strategy::vincenty> vi_s;
            bg::strategy::area::geographic<point, bg::strategy::thomas> th_s;
            bg::strategy::area::geographic<point, bg::strategy::andoyer> an_s;

            bg::model::polygon<point> poly{{{data[1], data[0]},
                                            {data[4], data[3]},
                                            {data[4], 0.0},
                                            {0.0, 0.0}}};


            areas[0] = bg::area(poly, vi_s);
            areas[1] = bg::area(poly, th_s);
            areas[2] = bg::area(poly, an_s);

            for (size_t i = 0; i < num_of_methods; i++)
            {

                acc[i][0](bg::math::abs((results[i].distance - data[6]) / data[6]));
                acc[i][1](bg::math::abs(results[i].distance - data[6]));
                acc[i][2](bg::math::abs((results[i].azimuth * r2d - data[2]) / data[2]));
                acc[i][3](bg::math::abs((results[i].reverse_azimuth * r2d - data[5]) / data[5]));
                acc[i][4](bg::math::abs((results[i].reduced_length - data[8]) / data[8]));
                acc[i][5](bg::math::abs((areas[i] - data[9]) / data[9]));
/*
                std::cout
                        << bg::math::abs((results[i].distance - data[6]) / data[6])
                        << " "
                        << bg::math::abs((results[i].azimuth * r2d - data[2]) / data[2])
                        << " "
                        << bg::math::abs((results[i].reverse_azimuth * r2d - data[5]) / data[5])
                        << " "
                        << bg::math::abs((results[i].reduced_length - data[8]) / data[8])
                        << " "
                        << bg::math::abs((areas[i] - data[9]) / data[9])
                        << std::endl;
*/
            }
/*
            //print data
            if (bg::math::abs((results[0].distance - data[6]) / data[6]) > 0.003)
            {
                for (std::vector<double>::iterator iit=data.begin(); iit < data.end(); ++iit)
                {
                    std::cout << *iit << " ";
                }
                std::cout << "\n" << results[0].distance;
                std::cout << "\n" << results[1].distance;
                std::cout << "\n" << results[2].distance;
                std::cout << std::endl << std::endl;
            }
*/
        }
        myfile.close();

        std::cout << "time=" << total_time << "\n";

        // Display statistics
        std::cout << "Mean:" << std::endl;
        for (size_t i = 0; i < num_of_methods; i++)
        {
            for (size_t j = 0; j < num_of_values; j++)
            {
                std::cout << ba::mean(acc[i][j]) << " ";
            }
            std::cout << std::endl;
        }
        std::cout << "Max:" << std::endl;
        for (size_t i = 0; i < num_of_methods; i++)
        {
            for (size_t j = 0; j < num_of_values; j++)
            {
                std::cout << ba::max(acc[i][j]) << " ";
            }
            std::cout << std::endl;
        }
        std::cout << "Min:" << std::endl;
        for (size_t i = 0; i < num_of_methods; i++)
        {
            for (size_t j = 0; j < num_of_values; j++)
            {
                std::cout << ba::min(acc[i][j]) << " ";
            }
            std::cout << std::endl;
        }
    }

    else std::cout << "Unable to open file";

    return 0;
}
