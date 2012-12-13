// Boost.Geometry Index
// Additional tests

// Copyright (c) 2011-2012 Adam Wulkiewicz, Lodz, Poland.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <boost/geometry/extensions/index/static_vector.hpp>

#include <iostream>

struct A
{
    A() : v(0) { std::cout << "A()" << std::endl; }
    A(int i) : v(i) { std::cout << "A(" << i << ")" << std::endl; }
    ~A() { std::cout << "~A" << v << "()" << std::endl; }
    A(A const& a) { std::cout << "A(A(" << a.v << "))" << std::endl; v = a.v; }
    A & operator=(A const& a) { std::cout << "A" << v << " = A" << a.v << std::endl; v = a.v; return *this; }
    int v;
};

int main()
{
    namespace bgi = boost::geometry::index;

    bgi::static_vector<A, 4> sv;
    std::cout << "resize(2)" << std::endl;
    sv.resize(2);           // 0 0
    std::cout << "push_back(A(22))" << std::endl;
    sv.push_back(A(22));    // 0 0 22
    std::cout << "push_back(A(23))" << std::endl;
    sv.push_back(A(23));    // 0 0 22 23
    std::cout << "pop_back()" << std::endl;
    sv.pop_back();          // 0 0 22
    sv.front().v = 10;      // 10 0 22
    sv.back().v = 100;      // 10 0 100
    sv[1].v = 50;           // 10 50 100
    std::cout << "resize(1)" << std::endl;
    sv.resize(1);           // 10
    std::cout << "~static_vector()" << std::endl;
    return 0;
}
