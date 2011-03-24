#ifndef TESTS_RTREE_NATIVE_HPP
#define TESTS_RTREE_NATIVE_HPP

#include <boost/geometry/geometry.hpp>

#include <boost/geometry/extensions/index/rtree/rtree.hpp>
#include <boost/geometry/extensions/index/translator/index.hpp>

#include <boost/geometry/extensions/index/rtree/visitors/print.hpp>

#include <boost/range/algorithm.hpp>
#include <boost/foreach.hpp>

#include <map>

void tests_rtree_native_hpp()
{
	std::cout << "tests/rtree_native.hpp\n";
	
    std::cout << "Boxes\n";
    {
        typedef boost::geometry::model::point<float, 2, boost::geometry::cs::cartesian> P;
        typedef boost::geometry::model::box<P> B;

        boost::geometry::index::rtree<B> t(4, 2);
        boost::geometry::index::insert(t, B(P(0, 0), P(1, 1)));
        boost::geometry::index::insert(t, B(P(2, 2), P(3, 3)));
        boost::geometry::index::insert(t, B(P(4, 4), P(5, 5)));
        boost::geometry::index::insert(t, B(P(6, 6), P(7, 7)));
        boost::geometry::index::insert(t, B(P(8, 8), P(9, 9)));
        std::cerr << t;
    }

    std::cout << "-------------------------------------------------\n";
    std::cout << "-------------------------------------------------\n";

    std::cout << "Points\n";
    {
        typedef boost::geometry::model::point<float, 2, boost::geometry::cs::cartesian> P;
        typedef boost::geometry::model::box<P> B;

        boost::geometry::index::rtree<P> t(4, 2);
        boost::geometry::index::insert(t, P(0, 0));
        boost::geometry::index::insert(t, P(2, 2));
        boost::geometry::index::insert(t, P(4, 4));
        boost::geometry::index::insert(t, P(6, 6));
        boost::geometry::index::insert(t, P(8, 8));
        std::cerr << t;
    }

    std::cout << "-------------------------------------------------\n";
    std::cout << "-------------------------------------------------\n";

    std::cout << "std::pair<Box, int>\n";
    {
        typedef boost::geometry::model::point<float, 2, boost::geometry::cs::cartesian> P;
        typedef boost::geometry::model::box<P> B;
        typedef std::pair<B, int> V;

        boost::geometry::index::rtree<V> t(4, 2);
        boost::geometry::index::insert(t, V(B(P(0, 0), P(1, 1)), 0));
        boost::geometry::index::insert(t, V(B(P(2, 2), P(3, 3)), 1));
        boost::geometry::index::insert(t, V(B(P(4, 4), P(5, 5)), 2));
        boost::geometry::index::insert(t, V(B(P(6, 6), P(7, 7)), 3));
        boost::geometry::index::insert(t, V(B(P(8, 8), P(9, 9)), 4));
        std::cerr << t;
    }

    std::cout << "-------------------------------------------------\n";
    std::cout << "-------------------------------------------------\n";

    std::cout << "boost::shared_ptr< std::pair<Box, int> >\n";
    {
        typedef boost::geometry::model::point<float, 2, boost::geometry::cs::cartesian> P;
        typedef boost::geometry::model::box<P> B;
        
        typedef boost::shared_ptr< std::pair<B, int> > V;

        V v1( new std::pair<B, int>(B(P(0, 0), P(1, 1)), 0) );
        V v2( new std::pair<B, int>(B(P(2, 2), P(3, 3)), 1) );
        V v3( new std::pair<B, int>(B(P(4, 4), P(5, 5)), 2) );
        V v4( new std::pair<B, int>(B(P(6, 6), P(7, 7)), 3) );
        V v5( new std::pair<B, int>(B(P(8, 8), P(9, 9)), 4) );

        boost::geometry::index::rtree<V> t(4, 2);
        boost::geometry::index::insert(t, v1);
        boost::geometry::index::insert(t, v2);
        boost::geometry::index::insert(t, v3);
        boost::geometry::index::insert(t, v4);
        boost::geometry::index::insert(t, v5);
        std::cerr << t;
    }

    std::cout << "-------------------------------------------------\n";
    std::cout << "-------------------------------------------------\n";

    std::cout << "std::map<int, Box>::iterator\n";
    {
        typedef boost::geometry::model::point<float, 2, boost::geometry::cs::cartesian> P;
        typedef boost::geometry::model::box<P> B;
        
        typedef std::map<int, B>::iterator V;

        std::map<int, B> m;
        m.insert(std::pair<int, B>(0, B(P(0, 0), P(1, 1))));
        m.insert(std::pair<int, B>(1, B(P(2, 2), P(3, 3))));
        m.insert(std::pair<int, B>(2, B(P(4, 4), P(5, 5))));
        m.insert(std::pair<int, B>(3, B(P(6, 6), P(7, 7))));
        m.insert(std::pair<int, B>(4, B(P(8, 8), P(9, 9))));

        boost::geometry::index::rtree<V> t(4, 2);
        V vit = m.begin();
        boost::geometry::index::insert(t, vit++);
        boost::geometry::index::insert(t, vit++);
        boost::geometry::index::insert(t, vit++);
        boost::geometry::index::insert(t, vit++);
        boost::geometry::index::insert(t, vit);
        std::cerr << t;
    }

    std::cout << "-------------------------------------------------\n";
    std::cout << "-------------------------------------------------\n";

    std::cout << "size_t\n";
    {
        typedef boost::geometry::model::point<float, 2, boost::geometry::cs::cartesian> P;
        typedef boost::geometry::model::box<P> B;

        typedef size_t V;
        typedef boost::geometry::index::translator::index<std::vector<B> > T;

        std::vector<B> v;
        v.push_back(B(P(0, 0), P(1, 1)));
        v.push_back(B(P(2, 2), P(3, 3)));
        v.push_back(B(P(4, 4), P(5, 5)));
        v.push_back(B(P(6, 6), P(7, 7)));
        v.push_back(B(P(8, 8), P(9, 9)));

        boost::geometry::index::rtree<V, T> t(4, 2, T(v));

        boost::geometry::index::insert(t, 0u);
        boost::geometry::index::insert(t, 1u);
        boost::geometry::index::insert(t, 2u);
        boost::geometry::index::insert(t, 3u);
        boost::geometry::index::insert(t, 4u);
        std::cerr << t;
    }
}

#endif // TESTS_RTREE_NATIVE_HPP
