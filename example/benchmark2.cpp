// Boost.Geometry Index
// Additional tests

// Copyright (c) 2011-2013 Adam Wulkiewicz, Lodz, Poland.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>

#include <boost/geometry/index/rtree.hpp>

#include <boost/chrono.hpp>
#include <boost/foreach.hpp>
#include <boost/random.hpp>

int main()
{
    namespace bg = boost::geometry;
    namespace bgi = bg::index;
    typedef boost::chrono::thread_clock clock_t;
    typedef boost::chrono::duration<float> dur_t;

    size_t values_count = 30000001;
    size_t count_start = 500000;
    size_t count_stop = 30000000;
    size_t count_step = 500000;
    size_t insrem_count = 10000;

    std::vector< std::pair<float, float> > coords;

    //randomize values
    {
        boost::mt19937 rng;
        //rng.seed(static_cast<unsigned int>(std::time(0)));
        float max_val = static_cast<float>(values_count / 2);
        boost::uniform_real<float> range(-max_val, max_val);
        boost::variate_generator<boost::mt19937&, boost::uniform_real<float> > rnd(rng, range);

        coords.reserve(values_count);

        std::cout << "randomizing data\n";
        for ( size_t i = 0 ; i < values_count ; ++i )
        {
            coords.push_back(std::make_pair(rnd(), rnd()));
        }
        std::cout << "randomized\n";
    }

    //typedef bg::model::d2::point_xy<double> P;
    typedef bg::model::point<double, 2, bg::cs::cartesian> P;
    typedef bgi::rtree<P, bgi::linear<16, 4> > RT;
    //typedef bgi::rtree<P, bgi::runtime::linear > RT;
    //typedef bgi::rtree<P, bgi::quadratic<32, 8> > RT;
    //typedef bgi::rtree<P, bgi::runtime::quadratic > RT;
    //typedef bgi::rtree<P, bgi::rstar<32, 8> > RT;
    //typedef bgi::rtree<P, bgi::runtime::rstar > RT;

    std::cout << "sizeof rtree: " << sizeof(RT) << std::endl;

    for ( size_t curr_count = count_start ; curr_count < count_stop ; curr_count += count_step )
    {
        RT t;
        //RT t(bgi::runtime::linear(32, 8));
        //RT t(bgi::runtime::quadratic(32, 8));
        //RT t(bgi::runtime::rstar(32, 8));

        std::cout << curr_count << ' ';

        // inserting test
        {
            for (size_t i = 0 ; i < curr_count ; ++i )
            {
                float x = coords[i].first;
                float y = coords[i].second;
                P p(x, y);

                t.insert(p);
            }

            clock_t::time_point start = clock_t::now();

            for (size_t i = 0 ; i < insrem_count ; ++i )
            {
                float x = coords.back().first;
                float y = coords.back().second;
                P p(x, y);

                t.insert(p);
                t.remove(p);
            }

            dur_t time = clock_t::now() - start;
            std::cout << time.count() << ' ';
        }

        std::cout << '\n';
    }

    return 0;
}
