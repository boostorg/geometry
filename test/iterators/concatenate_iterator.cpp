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

#include <cstddef>
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
    template <typename ConcatenateIterator, typename CombinedContainer>
    static inline void test_size(ConcatenateIterator first,
                                 ConcatenateIterator beyond,
                                 CombinedContainer const& combined)
    {
        BOOST_CHECK( combined.size() == std::distance(first, beyond) );

        std::size_t size(0);
        for (ConcatenateIterator it = first; it != beyond; ++it)
        {
            ++size;
        }
        BOOST_CHECK( combined.size() == size );

        size = 0;
        for (ConcatenateIterator it = beyond; it != first; --it)
        {
            ++size;
        }
        BOOST_CHECK( combined.size() == size );
    }


    template <typename ConcatenateIterator, typename CombinedContainer>
    static inline void test_equality(ConcatenateIterator first,
                                     ConcatenateIterator beyond,
                                     CombinedContainer const& combined)
    {
        typedef typename CombinedContainer::const_iterator iterator;

        iterator it = combined.begin();
        for (ConcatenateIterator cit = first; cit != beyond; ++cit, ++it)
        {
            BOOST_CHECK( *cit == *it );
        }

        if ( combined.begin() != combined.end() )
        {
            BOOST_CHECK( first != beyond );
            iterator it = combined.end();
            ConcatenateIterator cit = beyond;
            for (--cit, --it; cit != first; --cit, --it)
            {
                BOOST_CHECK( *cit == *it );
            }
            BOOST_CHECK( cit == first && it == combined.begin() );
            BOOST_CHECK( *cit == *it );
        }
        else
        {
            BOOST_CHECK( first == beyond );
        }
    }


    template <typename ConcatenateIterator, typename CombinedContainer>
    static inline void test_using_reverse(ConcatenateIterator first,
                                          ConcatenateIterator beyond,
                                          CombinedContainer& combined)
    {
        std::reverse(first, beyond);
        std::reverse(combined.begin(), combined.end());
        test_equality(first, beyond, combined);

#ifdef GEOMETRY_TEST_DEBUG
        print_container(std::cout, first, beyond, "reversed:") << std::endl;
#endif

        std::reverse(first, beyond);
        std::reverse(combined.begin(), combined.end());
        test_equality(first, beyond, combined);

#ifdef GEOMETRY_TEST_DEBUG
        print_container(std::cout, first, beyond, "re-reversed:") << std::endl;
#endif
    }


    template <typename ConcatenateIterator, typename CombinedContainer>
    static inline void test_using_remove_if(ConcatenateIterator first,
                                            ConcatenateIterator beyond,
                                            CombinedContainer& combined)
    {
        typedef typename std::iterator_traits
            <
                ConcatenateIterator
            >::value_type value_type;

#ifdef GEOMETRY_TEST_DEBUG
        std::cout << std::endl;
        std::cout << "odd elements removed:" << std::endl;
        print_container(std::cout, first, beyond, "before:")
            << std::endl;
#endif
        ConcatenateIterator new_beyond =
            std::remove_if(first, beyond, is_odd<value_type>());

        for (ConcatenateIterator it = first; it != new_beyond; ++it)
        {
            BOOST_CHECK( !is_odd<value_type>()(*it) );
        }     

#ifdef GEOMETRY_TEST_DEBUG
        print_container(std::cout, first, new_beyond, "after :")
            << std::endl;
#endif

    }

    template
    <
        typename ConcatenateIterator,
        typename ConstConcatenateIterator,
        typename Container
    >
    static inline
    void test_using_max_element(ConcatenateIterator first,
                                ConcatenateIterator beyond,
                                ConstConcatenateIterator const_first,
                                ConstConcatenateIterator const_beyond,
                                Container const& c,
                                std::size_t other_size,
                                bool second_container)
    {
        typedef typename std::iterator_traits
            <
                ConcatenateIterator
            >::value_type value_type;

        if ( c.size() == 0 )
        {
            return;
        }
        
        ConcatenateIterator c_first = first;
        if ( second_container )
        {
            std::size_t counter(0);
            while ( counter != other_size )
            {
                ++counter;
               c_first++;
            }
        }

        ConcatenateIterator it_max = std::max_element(first, beyond);
        ConstConcatenateIterator const_it_max =
            std::max_element(const_first, const_beyond);

        BOOST_CHECK( it_max == const_it_max );
        BOOST_CHECK( *it_max == *const_it_max );

        value_type old_value = *c_first;
        value_type new_value = *it_max + 1;

        *c_first = *it_max + 1;
        BOOST_CHECK( *c.begin() == new_value );

#ifdef GEOMETRY_TEST_DEBUG
        std::cout << std::endl;
        std::cout << "modified element of ";
        std::cout << (second_container ? "2nd" : "1st");
        std::cout << " container:" << std::endl;
        print_container(std::cout, c.begin(), c.end(),
                        (second_container ? "second  :" : "first  :"))
            << std::endl;
        print_container(std::cout, first, beyond, "combined:")
            << std::endl;
#endif

        *c_first = old_value;
        BOOST_CHECK( *c.begin() == old_value );
    }


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

        concat_iterator begin2(c1.end(), c1.end(), c2.begin(), c2.begin());
        const_concat_iterator const_begin2(c1.end(), c1.end(),
                                           c2.begin(), c2.begin());

        BOOST_CHECK( c1.empty() || *begin == *c1.begin() );
        BOOST_CHECK( c1.empty() || *const_begin == *c1.begin() );

        BOOST_CHECK( c2.empty() || *begin2 == *c2.begin() );
        BOOST_CHECK( c2.empty() || *const_begin2 == *c2.begin() );


        // test copying, dereferencing and element equality
        std::vector<typename Container1::value_type> combined;
        std::copy(begin, end, std::back_inserter(combined));
        test_equality(begin, end, combined);
        test_equality(const_begin, const_end, combined);

        combined.clear();
        std::copy(const_begin, const_end, std::back_inserter(combined));
        test_equality(begin, end, combined);
        test_equality(const_begin, const_end, combined);


        // test sizes (and std::distance)
        test_size(begin, end, combined);
        test_size(const_begin, const_end, combined);


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

        // check that we get the first element of the second container
        // properly

        // perform reversals (std::reverse)
        test_using_reverse(begin, end, combined);

        // test std::max_element, dereferencing and value assigment
        test_using_max_element(begin, end, const_begin, const_end,
                               c1, c2.size(), false);
        test_using_max_element(begin, end, const_begin, const_end,
                               c2, c1.size(), true);

        // test std::remove_if
        test_using_remove_if(begin, end, combined);

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
