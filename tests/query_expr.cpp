// Boost.Geometry Index
// Additional tests

// Copyright (c) 2011-2013 Adam Wulkiewicz, Lodz, Poland.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <boost/geometry/index/query_expr.hpp>

#include <boost/timer.hpp>
#include <fstream>
#include <iostream>

template <typename T>
struct Test
{
    inline Test(T const& t) : v(t) {}
    T v;
};

template <typename T1, typename T2, typename T3>
struct Raw
{
    inline explicit Raw(T1 const& t1, T2 const& t2, T3 const& t3) : v1(t1), v2(t2), v3(t3) {}
    T1 v1;
    T2 v2;
    T3 v3;
};

template <typename T1, typename T2> inline
boost::tuples::cons<
    Test<T2>,
    boost::tuples::cons<Test<T1>, boost::tuples::null_type>
>
operator&&(Test<T1> const& t1, Test<T2> const& t2)
{
    return
    boost::tuples::cons<
        Test<T2>,
        boost::tuples::cons<Test<T1>, boost::tuples::null_type>
    >(t2,
      boost::tuples::cons<Test<T1>, boost::tuples::null_type>(t1, boost::tuples::null_type())
     );
}

template <typename H, typename T, typename T2> inline
boost::tuples::cons<
    Test<T2>,
    boost::tuples::cons<H, T>
>
operator&&(boost::tuples::cons<H, T> const& t, Test<T2> const& t2)
{
    return
    boost::tuples::cons<
        Test<T2>,
        boost::tuples::cons<H, T>
    >(t2, t);
}

template <typename T>
struct Test2
{
    inline Test2(T const& t) : v(t) {}
    T v;
};

template <typename T1, typename T2> inline
boost::tuples::cons<
    Test2<T1>,
    boost::tuples::cons<Test2<T2>, boost::tuples::null_type>
>
operator&&(Test2<T1> const& t1, Test2<T2> const& t2)
{
    return
    boost::tuples::cons<
        Test2<T1>,
        boost::tuples::cons<Test2<T2>, boost::tuples::null_type>
    >(t1,
      boost::tuples::cons<Test2<T2>, boost::tuples::null_type>(t2, boost::tuples::null_type())
     );
}

template <typename T, typename Tuple, size_t I, size_t N>
struct add
{
    typedef
    boost::tuples::cons<
        typename boost::tuples::element<I, Tuple>::type,
        typename add<T, Tuple, I+1, N>::type
    > type;

    static type apply(T const& t, Tuple const& tup)
    {
        return
        type(
            boost::get<I>(tup),
            add<T, Tuple, I+1, N>::apply(t, tup)
        );
    }
};

template <typename T, typename Tuple, size_t I>
struct add<T, Tuple, I, I>
{
    typedef boost::tuples::cons<T, boost::tuples::null_type> type;

    static type apply(T const& t, Tuple const&)
    {
        return type(t, boost::tuples::null_type());
    }
};

template <typename H, typename T, typename T2> inline
typename add<
    Test2<T2>,
    boost::tuples::cons<H, T>,
    0,
    boost::tuples::length<boost::tuples::cons<H, T> >::value
>::type
operator&&(boost::tuples::cons<H, T> const& t, Test2<T2> const& t2)
{
    return
    add<
        Test2<T2>,
        boost::tuples::cons<H, T>,
        0,
        boost::tuples::length<boost::tuples::cons<H, T> >::value
    >::apply(t2, t);
}

template <typename T> inline
double test(T const& t)
{
    return boost::get<0>(t).v + boost::get<1>(t).v + boost::get<2>(t).v;
}

int main()
{
    using namespace boost::geometry::index::qe;

    boost::timer tim;

    size_t count = 200000000;
    int dummy = 0;
    std::ifstream f("blablabla");
    f >> dummy;

    std::cout << "&&1  &&2  b,   r,\n";

    double sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
    for(size_t i = 0 ; i < 20; ++i)
    {
        double foo = 0;

        {
            tim.restart();
            for ( size_t i = 0 ; i < count ; ++i )
            {
                foo += test(Test<int>(dummy) && Test<double>(dummy) && Test<float>(dummy));
            }
            double t = tim.elapsed();
            sum1 += t;
            std::cout << t << ' ';
        }

        {
            tim.restart();
            for ( size_t i = 0 ; i < count ; ++i )
            {
                foo += test(Test2<int>(dummy) && Test2<double>(dummy) && Test2<float>(dummy));
            }
            double t = tim.elapsed();
            sum2 += t;
            std::cout << t << ' ';
        }

        {
            tim.restart();
            for ( size_t i = 0 ; i < count ; ++i )
            {
                boost::tuple<Test<int>, Test<double>, Test<float> > tup
                    = boost::make_tuple(Test<int>(dummy), Test<double>(dummy), Test<float>(dummy));
                foo += boost::get<0>(tup).v + boost::get<1>(tup).v + boost::get<2>(tup).v;
            }
            double t = tim.elapsed();
            sum3 += t;
            std::cout << t << ' ';
        }

        {
            tim.restart();
            for ( size_t i = 0 ; i < count ; ++i )
            {
                Raw<Test<int>, Test<double>, Test<float> > tup(dummy, dummy, dummy);
                foo += tup.v1.v + tup.v2.v + tup.v3.v;
            }
            double t = tim.elapsed();
            sum4 += t;
            std::cout << t << ' ';
        }

        std::cout << foo << '\n';
    }

    std::cout << "sums:\n";
    std::cout << sum1 << " " << sum2 << " " << sum3 << " " << sum4;

    return 0;
}
