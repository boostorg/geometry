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
#include <iterator>

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

struct test_concatenate_iterator
{
    template <typename Container1, typename Container2>
    static inline void apply(Container1& c1, Container2& c2,
                             std::string const& case_id,
                             std::string const& containers_id)
    {
#ifdef GEOMETRY_TEST_DEBUG
        std::stringstream sstream;
        sstream << case_id << " [" << containers_id << "]";

        std::cout << "case id: " << sstream.str() << std::endl;
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


        // test constructors/assignment operators
        concat_iterator begin(c1.begin(), c1.end(), c2.begin(), c2.begin());
        concat_iterator end(c1.end(), c2.begin(), c2.end());
        const_concat_iterator const_begin(begin);
        const_concat_iterator const_end(end);
        const_begin = begin;
        const_end = end;

        // test sizes (and std::distance)
        BOOST_CHECK( c1.size() + c2.size() == std::distance(begin, end) );
        BOOST_CHECK( c1.size() + c2.size()
                     == std::distance(const_begin, const_end) );

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

        size = 0;
        for (concat_iterator it = end; it != begin; --it)
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

        // test element equality of elements and dereferencing
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

        if ( c1.begin() != c1.end() && c2.begin() != c2.end() )
        {
            const_iterator1 it1 = c1.end();
            const_iterator2 it2 = --c2.end();
            const_concat_iterator it = const_end;
            for (--it; it != const_begin; --it)
            {
                if ( it2 != c2.begin() )
                {
                    BOOST_CHECK( *it == *it2 );
                    --it2;
                }
                else if ( it1 == c1.end() && it2 == c2.begin() )
                {
                    BOOST_CHECK( *it == *it2 );
                    --it1;
                }
                else
                {
                    BOOST_CHECK( *it == *it1 );
                    --it1;
                }
            }
            BOOST_CHECK( it == const_begin && *it == *it1 );
        }

        // perform reversals (std::reverse)
        std::reverse(begin, end);
#ifdef GEOMETRY_TEST_DEBUG
        print_container(std::cout, begin, end, "reversed:") << std::endl;
#endif
        std::reverse(begin, end);
#ifdef GEOMETRY_TEST_DEBUG
        print_container(std::cout, begin, end, "re-reversed:") << std::endl;
#endif

        // test std::max_element and dereferencing
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
            BOOST_CHECK( *it_max == *const_it_max );

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
            BOOST_CHECK( *it_max == *const_it_max );

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

        // test std::remove_if
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


template <typename Container1, typename Container2>
void test_concatenation_of_containers(Container1& c1, Container2& c2,
                                      std::string const& containers_id)
{
#ifdef GEOMETRY_TEST_DEBUG
    std::cout << std::endl << std::endl;
    std::cout << "************************************" << std::endl
              << " TESTING CONTAINERS COMBINATION: "
              << containers_id << std::endl
              << "************************************" << std::endl
              << std::endl;
#endif

    c1.clear();
    c2.clear();    
    test_concatenate_iterator::apply(c1, c2, "empty_both", containers_id);

    c2 += 10,11,12,13,14,15,16,17,18,19,20;
    test_concatenate_iterator::apply(c1, c2, "empty_first", containers_id);

    c2.clear();
    c1 += 0,1,2,3,4,5,6;
    test_concatenate_iterator::apply(c1, c2, "empty_second", containers_id);

    c1.clear();
    c2.clear();
    c1 += 0,1,2,3,4,5,6;
    c2 += 10,11,12,13,14,15,16,17,18,19,20;
    test_concatenate_iterator::apply(c1, c2, "non_empty", containers_id);
}


BOOST_AUTO_TEST_CASE( test_concatenate_iterator_all )
{
    std::vector<int> v, vv;
    std::list<int> l, ll;
        
    test_concatenation_of_containers(v, vv, "VV");
    test_concatenation_of_containers(v, l, "VL");
    test_concatenation_of_containers(l, v, "LV");
    test_concatenation_of_containers(l, ll, "LL");
}
