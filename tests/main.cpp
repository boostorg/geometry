#include <tests/translators.hpp>
#include <tests/rtree_native.hpp>
#include <tests/rtree_filters.hpp>

#include <boost/geometry/extensions/index/rtree/visitors/are_boxes_ok.hpp>

int main()
{
    tests_translators_hpp();
    tests_rtree_native_hpp<boost::geometry::index::linear_tag>();
    //tests_rtree_native_hpp<boost::geometry::index::rstar_tag>();
    tests_rtree_filters_hpp();

    {
        namespace bg = boost::geometry;
        namespace bgi = boost::geometry::index;
        typedef bg::model::point<float, 2, bg::cs::cartesian> P;
        typedef bg::model::box<P> B;
        typedef std::pair<B, int> V;

        bgi::rtree<V, bgi::default_parameter, bgi::linear_tag> t(4, 2);
        const int m = 15;
        for ( int i = 0 ; i < m ; ++i )
        {
            bgi::insert(t, V(B(P(i*2, i*2), P(i*2+1, i*2+1)), i));
        }
        std::cout << t << "\n------------------------------------\n";
        std::cin.get();

        for ( int i = 0 ; i < m ; ++i )
        {
            bgi::remove(t, V(B(P(i*2, i*2), P(i*2+1, i*2+1)), i));
            std::cout << t << '\n';
            std::cout << ( boost::geometry::index::are_boxes_ok(t) ? "boxes OK" : "WRONG BOXES!" );
            std::cout << "\n------------------------------------\n";
            std::cin.get();
        }
    }

#ifdef _MSC_VER
    std::cin.get();
#endif

    return 0;
}
