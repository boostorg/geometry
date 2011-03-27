#include <tests/translators.hpp>
#include <tests/rtree_native.hpp>
#include <tests/rtree_filters.hpp>

int main()
{
    tests_translators_hpp();
    tests_rtree_native_hpp();
    tests_rtree_filters_hpp();

    /*namespace g = boost::geometry;
    typedef g::model::point<float, 2, g::cs::cartesian> P;
    typedef g::model::box<P> B;

    g::index::rtree<B> tree(4, 2);
    g::index::insert(tree, B(P(1, 6),P(6, 19)));
    g::index::insert(tree, B(P(10, 1),P(18, 18)));
    g::index::insert(tree, B(P(22, 6),P(27, 20)));
    g::index::insert(tree, B(P(29, 2),P(34, 18)));
    g::index::insert(tree, B(P(35, 3),P(39, 19)));

    std::cout << tree;*/

#ifdef _MSC_VER
    std::cin.get();
#endif

    return 0;
}
