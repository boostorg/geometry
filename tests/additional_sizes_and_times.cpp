// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.Index - example
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//#define BOOST_GEOMETRY_INDEX_USE_VARIANT_NODES
#include <boost/geometry/extensions/index/rtree/rtree.hpp>

#include <boost/geometry/extensions/index/rtree/visitors/are_boxes_ok.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/are_levels_ok.hpp>

#include <iostream>
#include <fstream>

#include <boost/timer.hpp>
#include <boost/foreach.hpp>
#include <boost/random.hpp>

int main()
{
    boost::timer tim;

    namespace bg = boost::geometry;
    namespace bgi = bg::index;

    typedef bg::model::point<float, 2, bg::cs::cartesian> P;
    typedef bg::model::box<P> B;
    //typedef bgi::rtree<std::pair<B, size_t>, bgi::default_parameter, bgi::linear_tag> RT;
    //typedef bgi::rtree<std::pair<B, size_t>, bgi::default_parameter, bgi::quadratic_tag> RT;
    typedef bgi::rtree<std::pair<B, size_t>, bgi::default_parameter, bgi::rstar_tag> RT;

    // load config file
    std::ifstream file_cfg("config.txt");
    size_t max_elems = 4;
    size_t min_elems = 2;
    size_t values_count = 0;
    size_t remove_count = 0;
    size_t queries_count = 0;
    std::string file_name("");
    file_cfg >> max_elems;
    file_cfg >> min_elems;
    file_cfg >> values_count;
    file_cfg >> remove_count;
    file_cfg >> queries_count;
    file_cfg >> file_name;
    std::cout << "max: " << max_elems << ", min: " << min_elems << "\n";
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
        float max_val = static_cast<float>(values_count / 2);
        boost::uniform_real<float> range(-max_val, max_val);
        boost::variate_generator<boost::mt19937&, boost::uniform_real<float> > rnd(rng, range);

        std::cout << "randomizing data\n";
        for ( size_t i = 0 ; i < values_count ; ++i )
        {
            coords.push_back(std::make_pair(rnd(), rnd()));
        }
        std::cout << "randomized\n";
    }
    
    // create rtree
    RT t(max_elems, min_elems);

    // elements inserting test
    {
        std::cout << "inserting time test...\n";
        tim.restart();
        for (size_t i = 0 ; i < values_count ; ++i )
        {
            float x = coords[i].first;
            float y = coords[i].second;
            B b(P(x - 0.5f, y - 0.5f), P(x + 0.5f, y + 0.5f));

            //if ( 341700 <= i )
            //    std::cout << i << "\n";

            // Czy mozliwe, ze w czasie powtornego reinserta
            // nagle drzewo sie powieksza o jeden poziom?
            // Tak, drzewo sie rozrasta, powinno sie wstawiac w poziomie liczac od lisci
            // TODO: relative_level zamiast level

            // TODO: asserty w operator(leaf)
            // current_level == leaf_level

            // Swoja droga to dziwne ze przy drzewie 4,2 
            // dzieje sie to samo dopiero dla obiektow o indeksie 300k a nie wczesniej
            // Dlaczego?
            // Przy 32 obiektach powtornie wstawianych jest 9 a przy 4 tylko 1

			// TODO: Zrobic kolejnego visitora sprawdzajacego czy odpowiednie wezly zostaly wstawione w dobrym miejscu
			// Np sprawdzajacego czy wszystkie liscie sa na tym samym poziomie

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
        std::cout << "searching time test...\n";
        tim.restart();    
        size_t temp = 0;
        for (size_t i = 0 ; i < queries_count ; ++i )
        {
            float x = coords[i].first;
            float y = coords[i].second;
            std::deque< std::pair<B, size_t> > result;
            t.find(B(P(x - 10, y - 10), P(x + 10, y + 10)), std::back_inserter(result));
            temp += result.size();
        }
        std::cout << "time: " << tim.elapsed() << "s\n";
        std::cout << "found: " << temp << "\n";
    }

    // elements removing test
    {
        std::cout << "removing time test...\n";
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
        std::cout << "searching time test...\n";
        tim.restart();    
        size_t temp = 0;
        for (size_t i = 0 ; i < queries_count ; ++i )
        {
            float x = coords[i].first;
            float y = coords[i].second;
            std::deque< std::pair<B, size_t> > result;
            t.find(B(P(x - 10, y - 10), P(x + 10, y + 10)), std::back_inserter(result));
            temp += result.size();
        }
        std::cout << "time: " << tim.elapsed() << "s\n";
        std::cout << "found: " << temp << "\n";
    }

    // inserting test
    {
        std::cout << "inserting time test...\n";
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
        std::cout << "searching time test...\n";
        tim.restart();    
        size_t temp = 0;
        for (size_t i = 0 ; i < queries_count ; ++i )
        {
            float x = coords[i].first;
            float y = coords[i].second;
            std::deque< std::pair<B, size_t> > result;
            t.find(B(P(x - 10, y - 10), P(x + 10, y + 10)), std::back_inserter(result));
            temp += result.size();
        }
        std::cout << "time: " << tim.elapsed() << "s\n";
        std::cout << "found: " << temp << "\n";
    }

    std::cin.get();

    return 0;
}
