// Boost.Geometry Index
// Unit Test

// Copyright (c) 2011-2012 Adam Wulkiewicz, Lodz, Poland.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <boost/test/included/test_exec_monitor.hpp>
#include <boost/test/impl/execution_monitor.ipp>

#include <boost/geometry/extensions/index/static_vector.hpp>

using namespace boost::geometry::index;

class value_ndc
{
public:
    value_ndc(int a) : aa(a) {}
    ~value_ndc() {}
    bool operator==(value_ndc const& v) const { return aa == v.aa; }
private:
    value_ndc(value_ndc const&) {}
    value_ndc & operator=(value_ndc const&) { return *this; }
    int aa;
};

class value_nd
{
public:
    value_nd(int a) : aa(a) {}
    ~value_nd() {}
    bool operator==(value_nd const& v) const { return aa == v.aa; }
private:
    int aa;
};

class value_nc
{
public:
    value_nc(int a = 0) : aa(a) {}
    ~value_nc() {}
    bool operator==(value_nc const& v) const { return aa == v.aa; }
private:
    value_nc(value_nc const&) {}
    value_nc & operator=(value_ndc const&) { return *this; }
    int aa;
};

template <typename T, size_t N>
void test_ctor_ndc()
{
    static_vector<T, N> s;
    BOOST_CHECK(s.size() == 0);
    BOOST_CHECK(s.capacity() == N);
    BOOST_CHECK_THROW( s.at(0), std::out_of_range );
}

template <typename T, size_t N>
void test_ctor_nc(size_t n)
{
    static_vector<T, N> s(n);
    BOOST_CHECK(s.size() == n);
    BOOST_CHECK(s.capacity() == N);
    BOOST_CHECK_THROW( s.at(n), std::out_of_range );
    if ( 1 < n )
    {
        s[0] = T(10);
        BOOST_CHECK(T(10) == s[0]);
        BOOST_CHECK(T(10) == s.at(0));
        s.at(1) = T(20);
        BOOST_CHECK(T(20) == s[1]);
        BOOST_CHECK(T(20) == s.at(1));
    }
}

template <typename T, size_t N>
void test_ctor_nd(size_t n, T const& v)
{
    static_vector<T, N> s(n, v);
    BOOST_CHECK(s.size() == n);
    BOOST_CHECK(s.capacity() == N);
    BOOST_CHECK_THROW( s.at(n), std::out_of_range );
    if ( 1 < n )
    {
        BOOST_CHECK(v == s[0]);
        BOOST_CHECK(v == s.at(0));
        BOOST_CHECK(v == s[1]);
        BOOST_CHECK(v == s.at(1));
        s[0] = T(10);
        BOOST_CHECK(T(10) == s[0]);
        BOOST_CHECK(T(10) == s.at(0));
        s.at(1) = T(20);
        BOOST_CHECK(T(20) == s[1]);
        BOOST_CHECK(T(20) == s.at(1));
    }
}

template <typename T, size_t N>
void test_resize_nc(size_t n)
{
    static_vector<T, N> s;

    s.resize(n);
    BOOST_CHECK(s.size() == n);
    BOOST_CHECK(s.capacity() == N);
    BOOST_CHECK_THROW( s.at(n), std::out_of_range );
    if ( 1 < n )
    {
        s[0] = T(10);
        BOOST_CHECK(T(10) == s[0]);
        BOOST_CHECK(T(10) == s.at(0));
        s.at(1) = T(20);
        BOOST_CHECK(T(20) == s[1]);
        BOOST_CHECK(T(20) == s.at(1));
    }
}

template <typename T, size_t N>
void test_resize_nd(size_t n, T const& v)
{
    static_vector<T, N> s;

    s.resize(n, v);
    BOOST_CHECK(s.size() == n);
    BOOST_CHECK(s.capacity() == N);
    BOOST_CHECK_THROW( s.at(n), std::out_of_range );
    if ( 1 < n )
    {
        BOOST_CHECK(v == s[0]);
        BOOST_CHECK(v == s.at(0));
        BOOST_CHECK(v == s[1]);
        BOOST_CHECK(v == s.at(1));
        s[0] = T(10);
        BOOST_CHECK(T(10) == s[0]);
        BOOST_CHECK(T(10) == s.at(0));
        s.at(1) = T(20);
        BOOST_CHECK(T(20) == s[1]);
        BOOST_CHECK(T(20) == s.at(1));
    }
}

template <typename T, size_t N>
void test_push_back_nd()
{
    static_vector<T, N> s;

    BOOST_CHECK(s.size() == 0);
    BOOST_CHECK_THROW( s.at(0), std::out_of_range );

    for ( size_t i = 0 ; i < N ; ++i )
    {
        s.push_back(T(i));
        BOOST_CHECK(s.size() == i + 1);
        BOOST_CHECK_THROW( s.at(i + 1), std::out_of_range );
        BOOST_CHECK(T(i) == s.at(i));
        BOOST_CHECK(T(i) == s[i]);
        BOOST_CHECK(T(i) == s.back());
        BOOST_CHECK(T(0) == s.front());
    }
}

template <typename T, size_t N>
void test_pop_back_nd()
{
    static_vector<T, N> s;

    for ( size_t i = 0 ; i < N ; ++i )
        s.push_back(i);    

    for ( size_t i = N ; i > 1 ; --i )
    {
        s.pop_back();
        BOOST_CHECK(s.size() == i - 1);
        BOOST_CHECK_THROW( s.at(i - 1), std::out_of_range );
        BOOST_CHECK(T(i - 2) == s.at(i - 2));
        BOOST_CHECK(T(i - 2) == s[i - 2]);
        BOOST_CHECK(T(i - 2) == s.back());
        BOOST_CHECK(T(0) == s.front());
    }
}

template <typename T, size_t N>
void test_copy_and_assign_nd()
{
    static_vector<T, N> s;

    for ( size_t i = 0 ; i < N ; ++i )
        s.push_back(i);    

    {
        static_vector<T, N> s1(s);
        BOOST_CHECK(s.size() == s1.size());
        for ( size_t i = 0 ; i < N ; ++i )
            BOOST_CHECK(s[i] == s1[i]) ;
    }
    {
        static_vector<T, N> s1;
        BOOST_CHECK(0 == s1.size());
        s1 = s;
        BOOST_CHECK(s.size() == s1.size());
        for ( size_t i = 0 ; i < N ; ++i )
            BOOST_CHECK(s[i] == s1[i]) ;
    }
    {
        static_vector<T, N> s1;
        BOOST_CHECK(0 == s1.size());
        s1.assign(s.begin(), s.end());
        BOOST_CHECK(s.size() == s1.size());
        for ( size_t i = 0 ; i < N ; ++i )
            BOOST_CHECK(s[i] == s1[i]) ;
    }
}

int test_main(int, char* [])
{
    test_ctor_ndc<int, 10>();
    test_ctor_ndc<value_ndc, 10>();
    test_ctor_nc<int, 10>(5);
    test_ctor_nc<value_nc, 10>(5);
    test_ctor_nd<int, 10>(5, 1);
    test_ctor_nd<value_nd, 10>(5, 1);

    test_resize_nc<int, 10>(5);
    test_resize_nc<value_nc, 10>(5);
    test_resize_nd<int, 10>(5, 1);
    test_resize_nd<value_nd, 10>(5, 1);

    test_push_back_nd<int, 10>();
    test_push_back_nd<value_nd, 10>();

    test_pop_back_nd<int, 10>();
    test_pop_back_nd<value_nd, 10>();

    test_copy_and_assign_nd<int, 10>();
    test_copy_and_assign_nd<value_nd, 10>();

    return 0;
}
