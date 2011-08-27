// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.Index - example
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <tests/translators.hpp>
#include <tests/rtree_native.hpp>
#include <tests/rtree_filters.hpp>

#include <boost/geometry/extensions/index/rtree/visitors/are_boxes_ok.hpp>

int main()
{
    tests_translators_hpp();
    tests_rtree_native_hpp<boost::geometry::index::linear<4, 2> >();
    tests_rtree_native_hpp<boost::geometry::index::quadratic<4, 2> >();
    tests_rtree_native_hpp<boost::geometry::index::rstar<4, 2> >();
    tests_rtree_filters_hpp();
    /*
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
            bgi::insert(t, V(B(P(i*2.0f, i*2.0f), P(i*2.0f+1, i*2.0f+1)), i));
        }
        std::cout << t << "\n------------------------------------\n";
        std::cin.get();

        for ( int i = 0 ; i < m ; ++i )
        {
            bgi::remove(t, V(B(P(i*2.0f, i*2.0f), P(i*2.0f+1, i*2.0f+1)), i));
            std::cout << t << '\n';
            std::cout << ( boost::geometry::index::are_boxes_ok(t) ? "boxes OK" : "WRONG BOXES!" );
            std::cout << "\n------------------------------------\n";
            std::cin.get();
        }
    }
    */
#ifdef _MSC_VER
    std::cin.get();
#endif

    return 0;
}
