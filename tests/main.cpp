#include <tests/translators.hpp>
#include <tests/rtree_native.hpp>
#include <tests/rtree_filters.hpp>

int main()
{
    tests_translators_hpp();
    tests_rtree_native_hpp();
    tests_rtree_filters_hpp();

#ifdef _MSC_VER
    std::cin.get();
#endif

    return 0;
}
