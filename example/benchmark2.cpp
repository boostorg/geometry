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
#include <set>

int main()
{
    namespace bg = boost::geometry;
    namespace bgi = bg::index;
    typedef boost::chrono::thread_clock clock_t;
    typedef boost::chrono::duration<float> dur_t;

    size_t values_count = 501;
    size_t count_start = 10;
    size_t count_stop = 500;
    size_t count_step = 10;
    size_t insrem_count = 3000000;

    std::vector< std::pair<float, float> > coords;

    //typedef bg::model::d2::point_xy<double> P;
    typedef bg::model::point<double, 1, bg::cs::cartesian> P;
    typedef bgi::rtree<P, bgi::linear<8, 3> > RT;
    //typedef bgi::rtree<P, bgi::runtime::linear > RT;
    //typedef bgi::rtree<P, bgi::quadratic<32, 8> > RT;
    //typedef bgi::rtree<P, bgi::runtime::quadratic > RT;
    //typedef bgi::rtree<P, bgi::rstar<32, 8> > RT;
    //typedef bgi::rtree<P, bgi::runtime::rstar > RT;

    RT t;
    std::set<float> s;
    size_t val_i = 0;
    for ( size_t curr_count = count_start ; curr_count < count_stop ; curr_count += count_step )
    {
        //RT t(bgi::runtime::linear(32, 8));
        //RT t(bgi::runtime::quadratic(32, 8));
        //RT t(bgi::runtime::rstar(32, 8));

        // inserting test
        {
            for (; val_i < curr_count ; ++val_i )
            {
                float v = val_i / 100.0f;
                P p(v);
                t.insert(p);
                s.insert(v);
            }

            float v = (val_i+1) / 100.0f;
            P test_p(v);

            std::cout << t.size() << ' ';

            clock_t::time_point start = clock_t::now();

            for (size_t i = 0 ; i < insrem_count ; ++i )
            {
                t.insert(test_p);
                t.remove(test_p);
            }

            dur_t time = clock_t::now() - start;
            std::cout << time.count() << ' ';

            start = clock_t::now();

            for (size_t i = 0 ; i < insrem_count ; ++i )
            {
                s.insert(v);
                s.erase(v);
            }

            time = clock_t::now() - start;
            std::cout << time.count() << ' ';
        }

        std::cout << '\n';
    }

    return 0;
}
