// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2014, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_TEST_MODULE
#define BOOST_TEST_MODULE test_concatenate_iterator
#endif

#include <boost/test/included/unit_test.hpp>

#include <iostream>
#include <string>
#include <algorithm>

#include <vector>
#include <list>

#include <boost/assign/std/vector.hpp>
#include <boost/assign/std/list.hpp>
#include <boost/geometry/iterators/concatenate_iterator.hpp>

using namespace boost::assign;

template <typename Iterator>
std::ostream& print_container(std::ostream& os,
                              Iterator begin, Iterator end,
                              std::string const& header)
{
    std::cout << header;
    for (Iterator it = begin; it != end; ++it)
    {
        os << " " << *it;
    }
    return os;
}


template <typename T>
struct is_odd
{
    inline bool operator()(T const& t) const
    {
        return t % 2 != 0;
    }
};

struct test_concat_iterator
{
    template <typename Container1, typename Container2>
    static inline void apply(Container1& c1, Container2& c2,
                             std::string const& case_id)
    {
#ifdef GEOMETRY_TEST_DEBUG
        std::cout << "case id: " << case_id << std::endl;
#endif
        typedef typename Container1::const_iterator const_iterator1;
        typedef typename Container2::const_iterator const_iterator2;
        typedef typename Container1::iterator iterator1;
        typedef typename Container2::iterator iterator2;

        typedef boost::geometry::concatenate_iterator
            <
                const_iterator1, const_iterator2,
                typename Container1::value_type const
            > const_concat_iterator;

        typedef boost::geometry::concatenate_iterator
            <
                iterator1, iterator2, typename Container1::value_type
            > concat_iterator;


        concat_iterator begin(c1.begin(), c1.end(), c2.begin());
        concat_iterator end(c1.end(), c2.end());
        const_concat_iterator const_begin(begin);
        const_concat_iterator const_end(end);
        const_begin = begin;
        const_end = end;

        std::size_t size(0);
        for (const_concat_iterator it = const_begin; it != const_end; ++it)
        {
            ++size;
        }
        BOOST_CHECK( c1.size() + c2.size() == size );

        size = 0;
        for (concat_iterator it = begin; it != end; ++it)
        {
            ++size;
        }
        BOOST_CHECK( c1.size() + c2.size() == size );

#ifdef GEOMETRY_TEST_DEBUG
        print_container(std::cout, c1.begin(), c1.end(), "first   :")
            << std::endl;
        print_container(std::cout, c2.begin(), c2.end(), "second  :")
            << std::endl;
        print_container(std::cout, begin, end,           "combined:")
            << std::endl;

        if ( begin != end )
        {
            std::cout << "min element: "
                      << *std::min_element(begin, end)
                      << std::endl;
            std::cout << "max element: "
                      << *std::max_element(const_begin, const_end)
                      << std::endl;
        }
#endif

        {
            const_iterator1 it1 = c1.begin();
            const_iterator2 it2 = c2.begin();
            for (const_concat_iterator it = const_begin; it != const_end; ++it)
            {
                if ( it1 != c1.end() )
                {
                    BOOST_CHECK( *it == *it1 );
                    ++it1;
                }
                else
                {
                    BOOST_CHECK( *it == *it2 );
                    ++it2;
                }
            }
        }

        typedef typename std::iterator_traits
            <
                concat_iterator
            >::value_type value_type;

        if ( c1.size() != 0 )
        {
            concat_iterator it_max = std::max_element(begin, end);
            const_concat_iterator const_it_max =
                std::max_element(const_begin, const_end);

            BOOST_CHECK( it_max == const_it_max );

            value_type old_value = *const_begin;
            value_type new_value = *it_max + 1;

            *begin = *it_max + 1;
            BOOST_CHECK( *c1.begin() == new_value );

#ifdef GEOMETRY_TEST_DEBUG
            std::cout << std::endl;
            std::cout << "modified element of 1st container:" << std::endl;
            print_container(std::cout, c1.begin(), c1.end(), "first   :")
                << std::endl;
            print_container(std::cout, begin, end,           "combined:")
                << std::endl;
#endif

            *begin = old_value;
            BOOST_CHECK( *c1.begin() == old_value );
        }

        if ( c2.size() != 0 )
        {
            concat_iterator begin2 = begin;
            std::size_t counter(0);
            while ( counter != c1.size() )
            {
                ++counter;
               begin2++;
            }

            concat_iterator it_max = std::max_element(begin, end);
            const_concat_iterator const_it_max =
                std::max_element(const_begin, const_end);

            BOOST_CHECK( it_max == const_it_max );

            value_type old_value = *begin2;
            value_type new_value = *it_max + 1;

            *begin2 = *it_max + 1;
            BOOST_CHECK( *c2.begin() == new_value );

#ifdef GEOMETRY_TEST_DEBUG
            std::cout << std::endl;
            std::cout << "modified element of 2nd container:" << std::endl;
            print_container(std::cout, c2.begin(), c2.end(), "second  :")
                << std::endl;
            print_container(std::cout, begin, end,           "combined:")
                << std::endl;
#endif

            *begin2 = old_value;
            BOOST_CHECK( *c2.begin() == old_value );
        }

#ifdef GEOMETRY_TEST_DEBUG
        std::cout << std::endl;
        std::cout << "odd elements removed:" << std::endl;
        print_container(std::cout, begin, end, "before:")
            << std::endl;
#endif
        concat_iterator new_end =
            std::remove_if(begin, end, is_odd<value_type>());

        for (const_concat_iterator it = const_begin; it != new_end; ++it)
        {
            BOOST_CHECK( !is_odd<value_type>()(*it) );
        }     

#ifdef GEOMETRY_TEST_DEBUG
        print_container(std::cout, begin, new_end, "after :")
            << std::endl;
#endif

#ifdef GEOMETRY_TEST_DEBUG
        std::cout << "====================" << std::endl << std::endl;
#endif
    }
};


BOOST_AUTO_TEST_CASE( test_concatenate_iterator )
{
    {
        std::vector<int> v;
        std::list<int> l;
        
        test_concat_iterator::apply(v, l, "empty_both");
    }
    {
        std::vector<int> v;
        std::list<int> l;
        l += 10,11,12,13,14,15,16,17,18,19,20;
        
        test_concat_iterator::apply(v, l, "empty_first");
    }
    {
        std::vector<int> v;
        v += 0,1,2,3,4,5,6;
        std::list<int> l;
        
        test_concat_iterator::apply(v, l, "empty_second");
    }
    {
        std::vector<int> v;
        v += 0,1,2,3,4,5,6;
        std::list<int> l;
        l += 10,11,12,13,14,15,16,17,18,19,20;
        
        test_concat_iterator::apply(l, v, "non_empty");
    }
}
