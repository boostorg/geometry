// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.Index - example
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <fstream>

#include <boost/geometry/extensions/index/rtree/rtree.hpp>

#include <boost/geometry/extensions/index/rtree/visitors/are_boxes_ok.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/are_levels_ok.hpp>

#include <boost/timer.hpp>
#include <boost/foreach.hpp>
#include <boost/random.hpp>

int main()
{
    boost::timer tim;

    namespace bg = boost::geometry;
    namespace bgi = bg::index;

    //typedef bg::model::d2::point_xy<double> P;
    typedef bg::model::point<double, 2, bg::cs::cartesian> P;
    typedef bg::model::box<P> B;
    typedef bgi::rtree<std::pair<B, size_t>, bgi::linear<32, 8> > RT;
    //typedef bgi::rtree<std::pair<B, size_t>, bgi::quadratic<32, 8> > RT;
    //typedef bgi::rtree<std::pair<B, size_t>, bgi::rstar<32, 8> > RT;
    /*typedef bgi::rtree<
        std::pair<B, size_t>,
        bgi::options::rtree<bgi::rstar<32, 8, 0, 10>, bgi::reinsert_tag, bgi::choose_by_area_diff_tag, bgi::rstar_tag, bgi::default_tag>
    > RT;*/

    // load config file
    std::ifstream file_cfg("config.txt");
    size_t values_count = 0;
    size_t remove_count = 0;
    size_t queries_count = 0;
    std::string file_name("");
    file_cfg >> values_count;
    file_cfg >> remove_count;
    file_cfg >> queries_count;
    file_cfg >> file_name;
    std::cout << "v: " << values_count << ", r: " << remove_count << ", q: " << queries_count << "\n";

    if ( values_count < remove_count )
    {
        std::cout << "can't remove more data than was inserted\n";
        return 0;
    }

    // prepare data buffer
    std::vector< std::pair<float, float> > coords;
    coords.reserve(values_count);
    
    // load test coordinates
    if ( file_name != "" )
    {
        std::ifstream file(file_name.c_str());
        if ( !file.is_open() )
        {
            std::cout << "can't open file containing coordinates\n";
            return 0;
        }

        std::cout << "loading data\n";
        for ( size_t i = 0 ; i < values_count ; ++i )
        {
            std::pair<float, float> v;
            file >> v.first;
            file >> v.second;
            coords.push_back(v);
        }
        std::cout << "loaded\n";

        if ( coords.size() != values_count || coords.size() < remove_count )
        {
            std::cout << "not enough coordinates loaded\n";
            return 0;
        }
    }
    // randomize
    else
    {
        boost::mt19937 rng;
        //rng.seed(static_cast<unsigned int>(std::time(0)));
        float max_val = static_cast<float>(values_count / 2);
        boost::uniform_real<float> range(-max_val, max_val);
        boost::variate_generator<boost::mt19937&, boost::uniform_real<float> > rnd(rng, range);
        
        //::srand( ::time(NULL) );
        //float factor = values_count / float(RAND_MAX);
        //float comp = values_count / 2;

        std::cout << "randomizing data\n";
        for ( size_t i = 0 ; i < values_count ; ++i )
        {
            coords.push_back(std::make_pair(rnd(), rnd()));
            //float x = rand() * factor + comp;
            //float y = rand() * factor + comp;
            //coords.push_back( std::make_pair( x, y ));
        }
        std::cout << "randomized\n";
    }
    
    // create rtree
    RT t;

    // elements inserting test
    {
        std::cout << "rtree inserting time test... ("
            << values_count << ")\n";
        tim.restart();
        for (size_t i = 0 ; i < values_count ; ++i )
        {
            float x = coords[i].first;
            float y = coords[i].second;
            B b(P(x - 0.5f, y - 0.5f), P(x + 0.5f, y + 0.5f));

            t.insert(std::make_pair(b, i));
        }
        std::cout << "time: " << tim.elapsed() << "s\n";
    }

    std::vector< std::pair<B, size_t> > v;

    // elements inserting test
    {
        std::cout << "vector inserting time test... ("
            << values_count << ")\n";
        tim.restart();
        for (size_t i = 0 ; i < values_count ; ++i )
        {
            float x = coords[i].first;
            float y = coords[i].second;
            B b(P(x - 0.5f, y - 0.5f), P(x + 0.5f, y + 0.5f));

            v.push_back(std::make_pair(b, i));
        }
        std::cout << "time: " << tim.elapsed() << "s\n";
    }

    // check
    if ( bgi::are_boxes_ok(t) )
        std::cout << "BOXES OK\n";
    else
        std::cout << "WRONG BOXES\n";
    if ( bgi::are_levels_ok(t) )
        std::cout << "LEVELS OK\n";
    else
        std::cout << "WRONG LEVELS\n";

    // searching test
    {
        std::cout << "query(intersects(B)) searching time test... ("
            << queries_count << ")\n";
        tim.restart();    
        size_t temp = 0;
        for (size_t i = 0 ; i < queries_count ; ++i )
        {
            float x = coords[i].first;
            float y = coords[i].second;
            std::deque< std::pair<B, size_t> > result;
            t.query(bgi::intersects(B(P(x - 10, y - 10), P(x + 10, y + 10))), std::back_inserter(result));
            temp += result.size();
        }
        std::cout << "time: " << tim.elapsed() << "s\n";
        std::cout << "found: " << temp << "\n";
    }

    // searching test
    {
        std::cout << "query(B) searching time test... ("
            << queries_count << ")\n";
        tim.restart();    
        size_t temp = 0;
        for (size_t i = 0 ; i < queries_count ; ++i )
        {
            float x = coords[i].first;
            float y = coords[i].second;
            std::deque< std::pair<B, size_t> > result;
            t.query(B(P(x - 10, y - 10), P(x + 10, y + 10)), std::back_inserter(result));
            temp += result.size();
        }
        std::cout << "time: " << tim.elapsed() << "s\n";
        std::cout << "found: " << temp << "\n";
    }

    // searching test
    {
        std::cout << "vector searching time test... ("
            << queries_count / 1000 << ")\n";
        tim.restart();    
        size_t temp = 0;
        for (size_t i = 0 ; i < queries_count / 1000 ; ++i )
        {
            float x = coords[i].first;
            float y = coords[i].second;
            std::deque< std::pair<B, size_t> > result;
            for ( std::vector< std::pair<B, size_t> >::const_iterator it = v.begin();
                  it != v.end() ;
                  ++it )
            {
                if ( bg::intersects(
                        it->first,
                        B(P(x - 10, y - 10), P(x + 10, y + 10))
                     )
                )
                    result.push_back(*it);
            }
            temp += result.size();
        }
        std::cout << "time: " << tim.elapsed() << "s\n";
        std::cout << "found: " << temp << "\n";
    }

    // searching test
    {
        std::cout << "nearest searching time test... ("
            << queries_count / 10 << ")\n";
        tim.restart();    
        size_t temp = 0;
        for (size_t i = 0 ; i < queries_count / 10 ; ++i )
        {
            float x = coords[i].first + 100;
            float y = coords[i].second + 100;
            std::pair<B, size_t> result;
            temp += t.nearest(P(x, y), result);
        }
        std::cout << "time: " << tim.elapsed() << "s\n";
        std::cout << "found: " << temp << "\n";
    }

    // searching test
    {
        std::cout << "nearest 5 searching time test... ("
            << queries_count / 10 << ")\n";
        tim.restart();    
        size_t temp = 0;
        for (size_t i = 0 ; i < queries_count / 10 ; ++i )
        {
            float x = coords[i].first + 100;
            float y = coords[i].second + 100;
            std::vector< std::pair<B, size_t> > result;
            temp += t.nearest(P(x, y), 5, std::back_inserter(result));
        }
        std::cout << "time: " << tim.elapsed() << "s\n";
        std::cout << "found: " << temp << "\n";
    }

    // searching test
    {
        std::cout << "vector nearest searching time test... ("
            << queries_count / 1000 << ")\n";
        tim.restart();    
        size_t temp = 0;
        for (size_t i = 0 ; i < queries_count / 1000 ; ++i )
        {
            typedef bg::default_distance_result<P, B>::type distance_type;

            float x = coords[i].first + 100;
            float y = coords[i].second + 100;
            std::pair<B, size_t> result;
            distance_type dist = std::numeric_limits<distance_type>::max();

            for ( std::vector< std::pair<B, size_t> >::const_iterator it = v.begin();
                it != v.end();
                ++it )
            {
                distance_type cd = bgi::mindist(P(x, y), it->first);

                if ( cd < dist )
                {
                    result = *it;
                    dist = cd;
                }
            }
            temp += dist < std::numeric_limits<distance_type>::max() ? 1 : 0;
        }
        std::cout << "time: " << tim.elapsed() << "s\n";
        std::cout << "found: " << temp << "\n";
    }

    // elements removing test
    {
        std::cout << "removing time test... ("
            << remove_count << ")\n";
        tim.restart();
        for (size_t i = 0 ; i < remove_count ; ++i )
        {
            float x = coords[i].first;
            float y = coords[i].second;
            B b(P(x - 0.5f, y - 0.5f), P(x + 0.5f, y + 0.5f));

            t.remove(std::make_pair(b, i));
        }
        std::cout << "time: " << tim.elapsed() << "s\n";
    }

    // check
    if ( bgi::are_boxes_ok(t) )
        std::cout << "BOXES OK\n";
    else
        std::cout << "WRONG BOXES\n";
    if ( bgi::are_levels_ok(t) )
        std::cout << "LEVELS OK\n";
    else
        std::cout << "WRONG LEVELS\n";

    // searching test
    {
        std::cout << "searching time test... ("
            << queries_count << ")\n";
        tim.restart();    
        size_t temp = 0;
        for (size_t i = 0 ; i < queries_count ; ++i )
        {
            float x = coords[i].first;
            float y = coords[i].second;
            std::deque< std::pair<B, size_t> > result;
            t.query(bgi::intersects(B(P(x - 10, y - 10), P(x + 10, y + 10))), std::back_inserter(result));
            temp += result.size();
        }
        std::cout << "time: " << tim.elapsed() << "s\n";
        std::cout << "found: " << temp << "\n";
    }

    // inserting test
    {
        std::cout << "inserting time test... ("
            << remove_count << ")\n";
        tim.restart();
        for (size_t i = 0 ; i < remove_count ; ++i )
        {
            float x = coords[i].first;
            float y = coords[i].second;
            B b(P(x - 0.5f, y - 0.5f), P(x + 0.5f, y + 0.5f));

            t.insert(std::make_pair(b, i));
        }
        std::cout << "time: " << tim.elapsed() << "s\n";
    }

    // check
    if ( bgi::are_boxes_ok(t) )
        std::cout << "BOXES OK\n";
    else
        std::cout << "WRONG BOXES\n";
    if ( bgi::are_levels_ok(t) )
        std::cout << "LEVELS OK\n";
    else
        std::cout << "WRONG LEVELS\n";

    // searching test
    {
        std::cout << "searching time test... ("
            << queries_count << ")\n";
        tim.restart();    
        size_t temp = 0;
        for (size_t i = 0 ; i < queries_count ; ++i )
        {
            float x = coords[i].first;
            float y = coords[i].second;
            std::deque< std::pair<B, size_t> > result;
            t.query(bgi::intersects(B(P(x - 10, y - 10), P(x + 10, y + 10))), std::back_inserter(result));
            temp += result.size();
        }
        std::cout << "time: " << tim.elapsed() << "s\n";
        std::cout << "found: " << temp << "\n";
    }

    std::cin.get();

    return 0;
}
