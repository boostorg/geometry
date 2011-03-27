#include <boost/geometry/extensions/index/rtree/rtree.hpp>

#include <iostream>

#include <boost/timer.hpp>
#include <boost/foreach.hpp>

int main()
{
    {
        typedef boost::geometry::model::point<float, 2, boost::geometry::cs::cartesian> P;
        typedef boost::geometry::model::box<P> B;

        boost::geometry::index::rtree<B>::leaf l;
        boost::geometry::index::rtree<B>::internal_node n;

        std::cout << "internal node size: " << sizeof(n) << '\n';
        std::cout << "leaf size: " << sizeof(l) << '\n';        
    }

    boost::timer tim;

    {
        typedef boost::geometry::model::point<float, 2, boost::geometry::cs::cartesian> P;
        typedef boost::geometry::model::box<P> B;

        // randomize boxes
        const size_t n = 10000;
        std::vector<B> v(n);
        for ( size_t i = 0 ; i < n ; ++i )
        {
            float x = float( rand() % 1000 );
            float y = float( rand() % 1000 );
            float w = float( rand() % 10 ) / 10.0f;
            float h = float( rand() % 10 ) / 10.0f;
            v[i] = B(P(x - w, y - h),P(x + w, y + h));
        }

        boost::geometry::index::rtree<B> t(4, 2);

        std::cout << "inserting time test...\n";

        tim.restart();    

        BOOST_FOREACH(B &b, v)
        {
            boost::geometry::index::insert(t, b);
        }

        std::cout << "time: " << tim.elapsed() << "s\n";
        
        std::cout << "deleting time test...\n";
        tim.restart();
    }
    std::cout << "time: " << tim.elapsed() << "s\n";

#ifdef _MSC_VER
    std::cin.get();
#endif

    return 0;
}
