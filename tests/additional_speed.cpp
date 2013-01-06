// Boost.Geometry Index
// Additional tests

// Copyright (c) 2011-2012 Adam Wulkiewicz, Lodz, Poland.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <fstream>

#include <boost/geometry/extensions/index/rtree/rtree.hpp>
#include <boost/geometry/extensions/index/inserter.hpp>

#include <boost/geometry/extensions/index/rtree/visitors/are_boxes_ok.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/are_levels_ok.hpp>

#include <boost/timer.hpp>
#include <boost/foreach.hpp>
#include <boost/random.hpp>

#include <boost/pool/pool_alloc.hpp>

int main()
{
    boost::timer tim;

    namespace bg = boost::geometry;
    namespace bgi = bg::index;

    size_t values_count = 1000000;
    size_t queries_count = 100000;

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
    typedef bg::model::box<P> B;
    typedef bgi::rtree<B, bgi::linear<32, 8> > RT;
    //typedef bgi::rtree<B, bgi::runtime::linear > RT;
    //typedef bgi::rtree<B, bgi::quadratic<32, 8> > RT;
   // typedef bgi::rtree<B, bgi::runtime::quadratic > RT;
    //typedef bgi::rtree<B, bgi::rstar<32, 8> > RT;
    //typedef bgi::rtree<B, bgi::runtime::rstar > RT;
    
    for ( ;; )
    {
        RT t;
        //RT t(bgi::runtime::linear(32, 8));
        //RT t(bgi::runtime::quadratic(32, 8));
        //RT t(bgi::runtime::rstar(32, 8));

        // inserting test
        {
            std::cout << "rtree inserting time test... ("
                << values_count << ")\n";
            tim.restart();
            for (size_t i = 0 ; i < values_count ; ++i )
            {
                float x = coords[i].first;
                float y = coords[i].second;
                B b(P(x - 0.5f, y - 0.5f), P(x + 0.5f, y + 0.5f));

                t.insert(b);
            }
            std::cout << "time: " << tim.elapsed() << "s\n";
        }

        {
            float x = coords[0].first;
            float y = coords[0].second;
            B b(P(x - 0.5f, y - 0.5f), P(x + 0.5f, y + 0.5f));
            t.remove(b);
        }

        std::vector<B> result;
        result.reserve(100);
        B result_one;

        // query test
        {
            std::cout << "query(B) searching time test... ("
                << queries_count << ")\n";
            tim.restart();    
            size_t temp = 0;
            for (size_t i = 0 ; i < queries_count ; ++i )
            {
                float x = coords[i].first;
                float y = coords[i].second;
                result.clear();
                t.spatial_query(B(P(x - 10, y - 10), P(x + 10, y + 10)), std::back_inserter(result));
                temp += result.size();
            }
            std::cout << "time: " << tim.elapsed() << "s\n";
            std::cout << "found: " << temp << "\n";
        }

        result.clear();

        // searching test
        {
            std::cout << "nearest 10 searching time test... ("
                << queries_count / 10 << ")\n";
            tim.restart();    
            size_t temp = 0;
            for (size_t i = 0 ; i < queries_count / 10 ; ++i )
            {
                float x = coords[i].first + 100;
                float y = coords[i].second + 100;
                result.clear();
                temp += t.nearest_query(P(x, y), 5, std::back_inserter(result));
            }
            std::cout << "time: " << tim.elapsed() << "s\n";
            std::cout << "found: " << temp << "\n";
        }

        {
            std::cout << "nearest 1 searching time test... ("
                << queries_count / 10 << ")\n";
            tim.restart();    
            size_t temp = 0;
            for (size_t i = 0 ; i < queries_count / 10 ; ++i )
            {
                float x = coords[i].first + 100;
                float y = coords[i].second + 100;
                temp += t.nearest_query(P(x, y), result_one);
            }
            std::cout << "time: " << tim.elapsed() << "s\n";
            std::cout << "found: " << temp << "\n";
        }
    }

    return 0;
}
