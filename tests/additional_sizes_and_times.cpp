#include <boost/geometry/extensions/index/rtree/rtree.hpp>

#include <iostream>
#include <fstream>

#include <boost/timer.hpp>
#include <boost/foreach.hpp>

#include <boost/geometry/extensions/index/rtree/visitors/save.hpp>

int main()
{
    boost::timer tim;

    namespace bg = boost::geometry;
    namespace bgi = bg::index;

    typedef bg::model::point<float, 2, bg::cs::cartesian> P;
    typedef bg::model::box<P> B;
    typedef bgi::rtree<std::pair<B, size_t>, bgi::default_parameter, bgi::linear_tag> RT;
    //typedef bgi::rtree<std::pair<B, size_t>, bgi::default_parameter, bgi::quadratic_tag> RT;

    std::ifstream file_cfg("config.txt");
    size_t max_elems = 4;
    size_t min_elems = 2;
    size_t values_count = 0;
    size_t queries_count = 0;
    char save_ch = 'n';
    file_cfg >> max_elems;
    file_cfg >> min_elems;
    file_cfg >> values_count;
    file_cfg >> queries_count;
    file_cfg >> save_ch;
    std::cout << "max: " << max_elems << ", min: " << min_elems << "\n";
    std::cout << "v: " << values_count << ", q: " << queries_count << "\n";

    std::ifstream file("test_coords.txt");
    std::cout << "loading data\n";
    std::vector< std::pair<float, float> > coords(values_count);
    for ( size_t i = 0 ; i < values_count ; ++i )
    {
        file >> coords[i].first;
        file >> coords[i].second;
    }
    std::cout << "loaded\n";
    
    RT t(max_elems, min_elems);

    {
        std::cout << "inserting time test...\n";
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

    if ( save_ch == 's' )
    {
        std::cout << "saving...\n";
        std::ofstream file("save_new.txt", std::ofstream::trunc);
        file << std::fixed;
        bgi::detail::rtree::visitors::save<
            RT::value_type,
            RT::translator_type,
            RT::box_type,
            RT::tag_type
        > saving_v(file, t.get_translator());
        t.apply_visitor(saving_v);
        std::cout << "saved...\n";
    }

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
        std::cout << temp << "\n";
    }

    {
        std::cout << "removing time test...\n";
        tim.restart();
        for (size_t i = 0 ; i < values_count / 2 ; ++i )
        {
            float x = coords[i].first;
            float y = coords[i].second;
            B b(P(x - 0.5f, y - 0.5f), P(x + 0.5f, y + 0.5f));

            t.remove(std::make_pair(b, i));
        }
        std::cout << "time: " << tim.elapsed() << "s\n";
    }

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
        std::cout << temp << "\n";
    }

    {
        std::cout << "inserting time test...\n";
        tim.restart();
        for (size_t i = 0 ; i < values_count / 2 ; ++i )
        {
            float x = coords[i].first;
            float y = coords[i].second;
            B b(P(x - 0.5f, y - 0.5f), P(x + 0.5f, y + 0.5f));

            t.insert(std::make_pair(b, i));
        }
        std::cout << "time: " << tim.elapsed() << "s\n";
    }

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
        std::cout << temp << "\n";
    }

    std::cin.get();

    return 0;
}
