#include <boost/geometry/extensions/index/rtree/rtree.hpp>

#include <iostream>
#include <fstream>

#include <boost/timer.hpp>
#include <boost/foreach.hpp>

int main()
{
    boost::timer tim;

    typedef boost::geometry::model::point<float, 2, boost::geometry::cs::cartesian> P;
    typedef boost::geometry::model::box<P> B;

    // randomize boxes
    const size_t n = 1000000;
    //const size_t n = 300;
    const size_t ns = 100000;

    std::ifstream file_cfg("config.txt");
    std::ifstream file("test_coords.txt");

    std::cout << "loading data\n";
    std::vector< std::pair<float, float> > coords(n);
    for ( size_t i = 0 ; i < n ; ++i )
    {
        file >> coords[i].first;
        file >> coords[i].second;
    }
    std::cout << "loaded\n";
    
    //std::cin.get();

    size_t max_elems, min_elems;
    file_cfg >> max_elems;
    file_cfg >> min_elems;
    std::cout << "max: " << max_elems << ", min: " << min_elems << "\n";

    std::cout << "inserting time test...\n";
    tim.restart();
    boost::geometry::index::rtree< std::pair<B, size_t> > t(max_elems, min_elems);
    for (size_t i = 0 ; i < n ; ++i )
    {
        float x = coords[i].first;
        float y = coords[i].second;
        B b(P(x - 0.5f, y - 0.5f), P(x + 0.5f, y + 0.5f));

        // Zle wyswietla dane, obcina czesc ulamkowa
        // Zle buduje drzewo dla i == 228

        //TEST
        /*if ( i == 228 )
        {
            std::cout << std::fixed << x << ", " << y << "\n";
            boost::geometry::index::detail::rtree::visitors::detail::print_indexable(std::cout, b);
        }*/

        t.insert(std::make_pair(b, i));

        //TEST
        /*if ( !boost::geometry::index::are_boxes_ok(t) )
        {
            std::ofstream log("log1.txt", std::ofstream::trunc);
            log << std::fixed << i << " - " << x << ", " << y << " - inserted: ";
            boost::geometry::index::detail::rtree::visitors::detail::print_indexable(log, b);
            log << '\n';
            log << ( boost::geometry::index::are_boxes_ok(t) ? "boxes OK" : "WRONG BOXES!" );
            log << '\n' << t;
        }*/
    }
    std::cout << "time: " << tim.elapsed() << "s\n";

    std::cout << "searching time test...\n";
    tim.restart();    
    size_t temp = 0;
    for (size_t i = 0 ; i < ns ; ++i )
    {
        float x = coords[i].first;
        float y = coords[i].second;
        std::deque< std::pair<B, size_t> > result = t.find(B(P(x - 10, y - 10), P(x + 10, y + 10)));
        temp += result.size();
    }
    std::cout << "time: " << tim.elapsed() << "s\n";
    std::cout << temp << "\n";

    std::cin.get();

    return 0;
}
