// Boost.Geometry Index
// Additional tests

// Copyright (c) 2011-2013 Adam Wulkiewicz, Lodz, Poland.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <fstream>

#include <boost/geometry/index/rtree.hpp>
#include <boost/geometry/index/inserter.hpp>

#include <boost/timer.hpp>
#include <boost/foreach.hpp>
#include <boost/random.hpp>

#include <boost/pool/pool_alloc.hpp>

int main()
{
    boost::timer tim;

    namespace bg = boost::geometry;
    namespace bgi = bg::index;

    size_t values_count = 1000000;
    size_t queries_count = 100000;

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

    //typedef bg::model::d2::point_xy<double> P;
    typedef bg::model::point<double, 2, bg::cs::cartesian> P;
    typedef bg::model::box<P> B;
    typedef bgi::rtree<B, bgi::linear<32, 8> > RT;
    //typedef bgi::rtree<B, bgi::runtime::linear > RT;
    //typedef bgi::rtree<B, bgi::quadratic<32, 8> > RT;
   // typedef bgi::rtree<B, bgi::runtime::quadratic > RT;
    //typedef bgi::rtree<B, bgi::rstar<32, 8> > RT;
    //typedef bgi::rtree<B, bgi::runtime::rstar > RT;

    for (unsigned i = 0 ; i < 10 ; ++i)
    {
        RT t;
        //RT t(bgi::runtime::linear(32, 8));
        //RT t(bgi::runtime::quadratic(32, 8));
        //RT t(bgi::runtime::rstar(32, 8));

        // inserting test
        {
            tim.restart();
            for (size_t i = 0 ; i < values_count ; ++i )
            {
                float x = coords[i].first;
                float y = coords[i].second;
                B b(P(x - 0.5f, y - 0.5f), P(x + 0.5f, y + 0.5f));

                t.insert(b);
            }
            double time = tim.elapsed();
            std::cout << time << "s - insert " << values_count << '\n';
        }

        std::vector<B> result;
        result.reserve(100);
        B result_one;

        {
            tim.restart();    
            size_t temp = 0;
            for (size_t i = 0 ; i < queries_count ; ++i )
            {
                float x = coords[i].first;
                float y = coords[i].second;
                result.clear();
                t.spatial_query(B(P(x - 10, y - 10), P(x + 10, y + 10)), std::back_inserter(result));
                temp += result.size();
            }
            double time = tim.elapsed();
            std::cout << time << "s - spatial_query(B) " << queries_count << " found " << temp << '\n';
        }

        {
            tim.restart();    
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
                t.spatial_query(
                    boost::make_tuple(
                    bgi::intersects(B(P(x1 - 10, y1 - 10), P(x1 + 10, y1 + 10))),
                    !bgi::within(B(P(x2 - 10, y2 - 10), P(x2 + 10, y2 + 10))),
                    !bgi::overlaps(B(P(x3 - 10, y3 - 10), P(x3 + 10, y3 + 10)))
                    ),
                    std::back_inserter(result)
                    );
                temp += result.size();
            }
            double time = tim.elapsed();
            std::cout << time << "s - spatial_query(i, !w, !o) " << queries_count << " found " << temp << '\n';
        }

        {
            tim.restart();    
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
                t.spatial_query(
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
            double time = tim.elapsed();
            std::cout << time << "s - spatial_query(i && !w && !o) " << queries_count << " found " << temp << '\n';
        }

        {
            tim.restart();    
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
            double time = tim.elapsed();
            std::cout << time << "s - query(i && !w && !o) " << queries_count << " found " << temp << '\n';
        }

        result.clear();

        {
            tim.restart();    
            size_t temp = 0;
            for (size_t i = 0 ; i < queries_count / 10 ; ++i )
            {
                float x = coords[i].first + 100;
                float y = coords[i].second + 100;
                temp += t.nearest_query(P(x, y), result_one);
            }
            double time = tim.elapsed();
            std::cout << time << "s - nearest_query(P) " << (queries_count / 10) << " found " << temp << '\n';
        }

        {
            tim.restart();    
            size_t temp = 0;
            for (size_t i = 0 ; i < queries_count / 10 ; ++i )
            {
                float x = coords[i].first + 100;
                float y = coords[i].second + 100;
                result.clear();
                temp += t.nearest_query(P(x, y), 5, std::back_inserter(result));
            }
            double time = tim.elapsed();
            std::cout << time << "s - nearest_query(P, 5) " << (queries_count / 10) << " found " << temp << '\n';
        }

        {
            tim.restart();    
            size_t temp = 0;
            for (size_t i = 0 ; i < queries_count / 10 ; ++i )
            {
                float x = coords[i].first + 100;
                float y = coords[i].second + 100;
                result.clear();
                temp += t.nearest_query(
                    bgi::bounded(
                        bgi::to_nearest(P(x, y)),
                        bgi::to_centroid(0),
                        bgi::to_furthest(100000)
                    ),
                    5,
                    std::back_inserter(result)
                );
            }
            double time = tim.elapsed();
            std::cout << time << "s - nearest_query(bounded(n, c, f), 5) " << (queries_count / 10) << " found " << temp << '\n';
        }

        {
            tim.restart();    
            size_t temp = 0;
            for (size_t i = 0 ; i < queries_count / 10 ; ++i )
            {
                float x = coords[i].first + 100;
                float y = coords[i].second + 100;
                temp += t.query(bgi::nearest(P(x, y)), result_one);
            }
            double time = tim.elapsed();
            std::cout << time << "s - query(nearest(P)) " << (queries_count / 10) << " found " << temp << '\n';
        }

        {
            tim.restart();    
            size_t temp = 0;
            for (size_t i = 0 ; i < queries_count / 10 ; ++i )
            {
                float x = coords[i].first + 100;
                float y = coords[i].second + 100;
                result.clear();
                temp += t.query(bgi::nearest(P(x, y), 5), std::back_inserter(result));
            }
            double time = tim.elapsed();
            std::cout << time << "s - query(nearest(P, 5)) " << (queries_count / 10) << " found " << temp << '\n';
        }

        {
            tim.restart();    
            size_t temp = 0;
            for (size_t i = 0 ; i < queries_count / 10 ; ++i )
            {
                float x = coords[i].first + 100;
                float y = coords[i].second + 100;
                result.clear();
                temp += t.query(bgi::nearest(
                                    bgi::bounded(
                                    bgi::to_nearest(P(x, y)),
                                    bgi::to_centroid(0),
                                    bgi::to_furthest(100000)
                                ),
                            5),
                            std::back_inserter(result)
                        );
            }
            double time = tim.elapsed();
            std::cout << time << "s - query(nearest(bounded(n, c, f), 5)) " << (queries_count / 10) << " found " << temp << '\n';
        }

        {
            tim.restart();
            for (size_t i = 0 ; i < values_count / 10 ; ++i )
            {
                float x = coords[i].first;
                float y = coords[i].second;
                B b(P(x - 0.5f, y - 0.5f), P(x + 0.5f, y + 0.5f));

                t.remove(b);
            }
            double time = tim.elapsed();
            std::cout << time << "s - remove " << values_count / 10 << '\n';
        }

        std::cout << "------------------------------------------------\n";
    }

    return 0;
}
