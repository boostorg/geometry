// Boost.Geometry
// Unit Test

// Copyright (c) 2014 Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <geometry_test_common.hpp>

#include <vector>
#include <boost/geometry/util/range.hpp>

namespace bgt {

template <bool MutableIterator>
struct beginner
{
    template <typename Range>
    typename boost::range_iterator<Range>::type
    operator()(Range & rng)
    {
        return boost::begin(rng);
    }
};
template <>
struct beginner<false>
{
    template <typename Range>
    typename boost::range_iterator<Range const>::type
    operator()(Range & rng)
    {
        return boost::const_begin(rng);
    }
};

template <bool MutableIterator>
struct ender
{
    template <typename Range>
    typename boost::range_iterator<Range>::type
    operator()(Range & rng)
    {
        return boost::end(rng);
    }
};
template <>
struct ender<false>
{
    template <typename Range>
    typename boost::range_iterator<Range const>::type
    operator()(Range & rng)
    {
        return boost::const_end(rng);
    }
};

} // namespace bgt

template <bool MutableIterator>
void test_all()
{
    namespace bgr = bg::range;

    bgt::beginner<MutableIterator> begin;
    bgt::ender<MutableIterator> end;

    std::vector<int> v;
    for (int i = 0 ; i < 20 ; ++i)
    {
        bgr::push_back(v, i);
    }

    for (int i = 0 ; i < 20 ; ++i)
    {
        BOOST_CHECK(bgr::at(v, i) == i);
    }

    BOOST_CHECK(bgr::front(v) == 0);
    BOOST_CHECK(bgr::back(v) == 19);

    BOOST_CHECK(boost::size(v) == 20); // [0,19]
    bgr::resize(v, 15);
    BOOST_CHECK(boost::size(v) == 15); // [0,14]
    BOOST_CHECK(bgr::back(v) == 14);

    bgr::pop_back(v);
    BOOST_CHECK(boost::size(v) == 14); // [0,13]
    BOOST_CHECK(bgr::back(v) == 13);
    
    bgr::erase(v, end(v) - 1);
    BOOST_CHECK(boost::size(v) == 13); // [0,12]
    BOOST_CHECK(bgr::back(v) == 12);

    bgr::erase(v, end(v) - 3, end(v));
    BOOST_CHECK(boost::size(v) == 10); // [0,9]
    BOOST_CHECK(bgr::back(v) == 9);

    bgr::erase(v, begin(v) + 2);
    BOOST_CHECK(boost::size(v) == 9); // {0,1,3..9}
    BOOST_CHECK(bgr::at(v, 1) == 1);
    BOOST_CHECK(bgr::at(v, 2) == 3);
    BOOST_CHECK(bgr::back(v) == 9);

    bgr::erase(v, begin(v) + 2, begin(v) + 2);
    BOOST_CHECK(boost::size(v) == 9); // {0,1,3..9}
    BOOST_CHECK(bgr::at(v, 1) == 1);
    BOOST_CHECK(bgr::at(v, 2) == 3);
    BOOST_CHECK(bgr::back(v) == 9);

    bgr::erase(v, begin(v) + 2, begin(v) + 5);
    BOOST_CHECK(boost::size(v) == 6); // {0,1,6..9}
    BOOST_CHECK(bgr::at(v, 1) == 1);
    BOOST_CHECK(bgr::at(v, 2) == 6);
    BOOST_CHECK(bgr::back(v) == 9);

    bgr::erase(v, begin(v));
    BOOST_CHECK(boost::size(v) == 5); // {1,6..9}
    BOOST_CHECK(bgr::at(v, 0) == 1);
    BOOST_CHECK(bgr::at(v, 1) == 6);
    BOOST_CHECK(bgr::back(v) == 9);

    bgr::erase(v, begin(v), begin(v) + 3);
    BOOST_CHECK(boost::size(v) == 2); // {8,9}
    BOOST_CHECK(bgr::at(v, 0) == 8);
    BOOST_CHECK(bgr::at(v, 1) == 9);
    BOOST_CHECK(bgr::back(v) == 9);

    bgr::erase(v, begin(v), end(v));
    BOOST_CHECK(boost::size(v) == 0);
}

int test_main(int, char* [])
{
    test_all<true>();
    test_all<false>();

    return 0;
}
