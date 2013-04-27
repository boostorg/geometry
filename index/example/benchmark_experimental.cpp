// Boost.Geometry Index
// Additional tests

// Copyright (c) 2011-2013 Adam Wulkiewicz, Lodz, Poland.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#define BOOST_GEOMETRY_INDEX_DETAIL_EXPERIMENTAL
#define BOOST_GEOMETRY_INDEX_DETAIL_ENABLE_TYPE_ERASED_ITERATORS

#include <iostream>

#include <boost/chrono.hpp>
#include <boost/foreach.hpp>
#include <boost/random.hpp>

#include <boost/geometry/index/rtree.hpp>

namespace bg = boost::geometry;
namespace bgi = bg::index;

typedef bg::model::point<double, 2, bg::cs::cartesian> P;
typedef bg::model::box<P> B;

#ifdef BOOST_GEOMETRY_INDEX_DETAIL_ENABLE_TYPE_ERASED_ITERATORS

#include <boost/type_erasure/iterator.hpp>

typedef boost::type_erasure::any<
    boost::type_erasure::iterator<
        boost::single_pass_traversal_tag,
        boost::type_erasure::_self,
        B const&,
        std::ptrdiff_t,
        const B
    >
> iterator;

#endif

int main()
{
    typedef boost::chrono::thread_clock clock_t;
    typedef boost::chrono::duration<float> dur_t;

    size_t values_count = 1000000;
    size_t queries_count = 100000;
    size_t nearest_queries_count = 10000;
    unsigned neighbours_count = 10;

    std::vector< std::pair<float, float> > coords;

    //randomize values
    {
        boost::mt19937 rng;
        //rng.seed(static_cast<unsigned int>(std::time(0)));
        float max_val = static_cast<float>(values_count / 2);
        boost::uniform_real<float> range(-max_val, max_val);
        boost::variate_generator<boost::mt19937&, boost::uniform_real<float> > rnd(rng, range);

        coords.reserve(values_count);

        std::cout << "randomizing data\n";
        for ( size_t i = 0 ; i < values_count ; ++i )
        {
            coords.push_back(std::make_pair(rnd(), rnd()));
        }
        std::cout << "randomized\n";
    }

    typedef bgi::rtree<B, bgi::linear<16, 4> > RT;
    //typedef bgi::rtree<B, bgi::quadratic<8, 3> > RT;
    //typedef bgi::rtree<B, bgi::rstar<8, 3> > RT;

    std::cout << "sizeof rtree: " << sizeof(RT) << std::endl;

    for (;;)
    {
        RT t;

        // inserting test
        {
            clock_t::time_point start = clock_t::now();
            for (size_t i = 0 ; i < values_count ; ++i )
            {
                float x = coords[i].first;
                float y = coords[i].second;
                B b(P(x - 0.5f, y - 0.5f), P(x + 0.5f, y + 0.5f));

                t.insert(b);
            }
            dur_t time = clock_t::now() - start;
            std::cout << time << " - insert " << values_count << '\n';
        }

        std::vector<B> result;
        result.reserve(100);
        B result_one;

        {
            clock_t::time_point start = clock_t::now();
            size_t temp = 0;
            for (size_t i = 0 ; i < queries_count ; ++i )
            {
                float x = coords[i].first;
                float y = coords[i].second;
                result.clear();
                t.query(bgi::intersects(B(P(x - 10, y - 10), P(x + 10, y + 10))), std::back_inserter(result));
                temp += result.size();
            }
            dur_t time = clock_t::now() - start;
            std::cout << time << " - query(B) " << queries_count << " found " << temp << '\n';
        }

#ifdef BOOST_GEOMETRY_INDEX_DETAIL_EXPERIMENTAL
        {
            clock_t::time_point start = clock_t::now();
            size_t temp = 0;
            for (size_t i = 0 ; i < queries_count ; ++i )
            {
                float x = coords[i].first;
                float y = coords[i].second;
                result.clear();
                std::copy(t.qbegin(bgi::intersects(B(P(x - 10, y - 10), P(x + 10, y + 10)))),
                          t.qend(bgi::intersects(B(P(x - 10, y - 10), P(x + 10, y + 10)))),
                          std::back_inserter(result));
                temp += result.size();
            }
            dur_t time = clock_t::now() - start;
            std::cout << time << " - query iterator(B) " << queries_count << " found " << temp << '\n';
        }
#ifdef BOOST_GEOMETRY_INDEX_DETAIL_ENABLE_TYPE_ERASED_ITERATORS
        {
            clock_t::time_point start = clock_t::now();
            size_t temp = 0;
            for (size_t i = 0 ; i < queries_count ; ++i )
            {
                float x = coords[i].first;
                float y = coords[i].second;
                result.clear();
                iterator first = t.qbegin(bgi::intersects(B(P(x - 10, y - 10), P(x + 10, y + 10))));
                iterator last = t.qend(bgi::intersects(B(P(x - 10, y - 10), P(x + 10, y + 10))));
                std::copy(first, last, std::back_inserter(result));
                temp += result.size();
            }
            dur_t time = clock_t::now() - start;
            std::cout << time << " - type-erased query iterator(B) " << queries_count << " found " << temp << '\n';
        }
#endif
#endif

        {
            clock_t::time_point start = clock_t::now();
            size_t temp = 0;
            for (size_t i = 0 ; i < queries_count / 2 ; ++i )
            {
                float x1 = coords[i].first;
                float y1 = coords[i].second;
                float x2 = coords[i+1].first;
                float y2 = coords[i+1].second;
                float x3 = coords[i+2].first;
                float y3 = coords[i+2].second;
                result.clear();
                t.query(
                    bgi::intersects(B(P(x1 - 10, y1 - 10), P(x1 + 10, y1 + 10)))
                    &&
                    !bgi::within(B(P(x2 - 10, y2 - 10), P(x2 + 10, y2 + 10)))
                    &&
                    !bgi::overlaps(B(P(x3 - 10, y3 - 10), P(x3 + 10, y3 + 10)))
                    ,
                    std::back_inserter(result)
                    );
                temp += result.size();
            }
            dur_t time = clock_t::now() - start;
            std::cout << time << " - query(i && !w && !o) " << queries_count << " found " << temp << '\n';
        }

        result.clear();

        {
            clock_t::time_point start = clock_t::now();
            size_t temp = 0;
            for (size_t i = 0 ; i < nearest_queries_count ; ++i )
            {
                float x = coords[i].first + 100;
                float y = coords[i].second + 100;
                result.clear();
                temp += t.query(bgi::nearest(P(x, y), neighbours_count), std::back_inserter(result));
            }
            dur_t time = clock_t::now() - start;
            std::cout << time << " - query(nearest(P, " << neighbours_count << ")) " << nearest_queries_count << " found " << temp << '\n';
        }

#ifdef BOOST_GEOMETRY_INDEX_DETAIL_EXPERIMENTAL
        {
            clock_t::time_point start = clock_t::now();
            size_t temp = 0;
            for (size_t i = 0 ; i < nearest_queries_count ; ++i )
            {
                float x = coords[i].first + 100;
                float y = coords[i].second + 100;
                result.clear();
                std::copy(t.qbegin(bgi::nearest(P(x, y), neighbours_count)),
                          t.qend(bgi::nearest(P(x, y), neighbours_count)),
                          std::back_inserter(result));
                temp += result.size();
            }
            dur_t time = clock_t::now() - start;
            std::cout << time << " - nearest_iterator(P, " << neighbours_count << ")) " << nearest_queries_count << " found " << temp << '\n';
        }
#ifdef BOOST_GEOMETRY_INDEX_DETAIL_ENABLE_TYPE_ERASED_ITERATORS
        {
            clock_t::time_point start = clock_t::now();
            size_t temp = 0;
            for (size_t i = 0 ; i < queries_count ; ++i )
            {
                float x = coords[i].first;
                float y = coords[i].second;
                result.clear();
                iterator first = t.qbegin(bgi::nearest(P(x, y), neighbours_count));
                iterator last = t.qend(bgi::nearest(P(x, y), neighbours_count));
                std::copy(first, last, std::back_inserter(result));
                temp += result.size();
            }
            dur_t time = clock_t::now() - start;
            std::cout << time << " - type-erased nearest_iterator(P, " << neighbours_count << ")) " << nearest_queries_count << " found " << temp << '\n';
        }
#endif
#endif

        {
            clock_t::time_point start = clock_t::now();
            for (size_t i = 0 ; i < values_count / 10 ; ++i )
            {
                float x = coords[i].first;
                float y = coords[i].second;
                B b(P(x - 0.5f, y - 0.5f), P(x + 0.5f, y + 0.5f));

                t.remove(b);
            }
            dur_t time = clock_t::now() - start;
            std::cout << time << " - remove " << values_count / 10 << '\n';
        }

        std::cout << "------------------------------------------------\n";
    }

    return 0;
}
