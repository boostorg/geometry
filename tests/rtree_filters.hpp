#ifndef TESTS_RTREE_FILTERS_HPP
#define TESTS_RTREE_FILTERS_HPP

#include <boost/geometry/geometry.hpp>
#include <boost/geometry/extensions/index/rtree/rtree.hpp>

#include <boost/geometry/extensions/index/rtree/filters.hpp>
#include <boost/geometry/extensions/index/filters/nearest_filter.hpp>

#include <iostream>

#include <boost/range/algorithm.hpp>
#include <boost/foreach.hpp>

//template <typename R>
//void tests_rtree_filters_hpp_print_range(R const& r)
//{
//    BOOST_FOREACH(typename boost::iterator_value<typename R::const_iterator>::type const& b, r)
//    {
//        float min_x = b.min_corner().template get<0>();
//        float min_y = b.min_corner().template get<1>();
//        float max_x = b.max_corner().template get<0>();
//        float max_y = b.max_corner().template get<1>();
//        std::cout << "(" << min_x << ", " << min_y << ")";
//        std::cout << 'x';
//        std::cout << "(" << max_x << ", " << max_y << ")";
//        std::cout << '\n';
//    }
//    std::cout << std::endl;
//}

BOOST_AUTO_TEST_CASE(tests_rtree_query_filter)
{
#ifdef TEST_PRINT_INFO
	std::cout << "tests/rtree_filters.hpp\n";
#endif

    typedef boost::geometry::model::point<float, 2, boost::geometry::cs::cartesian> P;
    typedef boost::geometry::model::box<P> B;

    {
        namespace bgi = boost::geometry::index;

        bgi::rtree<B, bgi::rstar<4, 2> > t;
        bgi::insert(t, B(P(0, 0), P(1, 1)));
        bgi::insert(t, B(P(2, 2), P(3, 3)));
        bgi::insert(t, B(P(4, 4), P(5, 5)));
        bgi::insert(t, B(P(6, 6), P(7, 7)));
        bgi::insert(t, B(P(8, 8), P(9, 9)));

        size_t n = 0;
        BOOST_FOREACH(B const& b, t | bgi::query_filtered(B(P(1.5f, 1.5f), P(4.5f, 4.5f))))
        {
            ++n;
        }

        BOOST_CHECK(n == 2);
    }
}

#endif // TESTS_RTREE_FILTERS_HPP
