// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2014, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_TEST_MODULE
#define BOOST_TEST_MODULE test_flatten_iterator
#endif

#include <boost/test/included/unit_test.hpp>

#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>

#include <vector>
#include <list>

#include <boost/mpl/if.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/assign/std/list.hpp>
#include <boost/assign/std/set.hpp>
#include <boost/geometry/iterators/flatten_iterator.hpp>

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


template <typename Iterator>
std::ostream& print_nested_container(std::ostream& os,
                                     Iterator begin, Iterator end,
                                     std::string const& header)
{
    std::cout << header;
    for (Iterator outer = begin; outer != end; ++outer)
    {
        os << " (";
        for (BOOST_AUTO_TPL(inner, outer->begin());
             inner != outer->end(); ++inner)
        {
            if ( inner != outer->begin() )
            {
                os << " ";
            }
            os << *inner;
        }
        os << ") ";
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


template <typename T>
struct is_even
{
    inline bool operator()(T const& t) const
    {
        return !is_odd<T>()(t);
    }
};



template <typename InnerContainer>
struct access_begin
{
    typedef typename boost::mpl::if_
    <
        typename boost::is_const<InnerContainer>::type,
        typename InnerContainer::const_iterator,
        typename InnerContainer::iterator
    >::type return_type;

    static inline return_type apply(InnerContainer& inner)
    {
        return inner.begin();
    }
};


template <typename InnerContainer>
struct access_end
{
    typedef typename boost::mpl::if_
    <
        typename boost::is_const<InnerContainer>::type,
        typename InnerContainer::const_iterator,
        typename InnerContainer::iterator
    >::type return_type;

    static inline return_type apply(InnerContainer& inner)
    {
        return inner.end();
    }
};


template <typename NestedContainer>
std::size_t number_of_elements(NestedContainer const& c)
{
    std::size_t num_elements(0);
    for (typename NestedContainer::const_iterator outer = c.begin();
         outer != c.end(); ++outer)
    {
        num_elements += outer->size();
    }
    return num_elements;
}


struct test_flatten_iterator
{
    template <typename NestedContainer>
    static inline void apply(NestedContainer& c,
                             std::string const& case_id,
                             std::string const& container_id)
    {
#ifdef GEOMETRY_TEST_DEBUG
        std::stringstream sstream;
        sstream << case_id << " " << "[" << container_id << "]";

        std::cout << "case id: " << sstream.str() << std::endl;
#endif
        typedef typename NestedContainer::const_iterator const_outer_iterator;
        typedef typename NestedContainer::iterator outer_iterator;
        typedef typename NestedContainer::value_type inner_container;

        typedef typename inner_container::const_iterator const_inner_iterator;
        typedef typename inner_container::iterator inner_iterator;

        typedef boost::geometry::flatten_iterator
            <
                const_outer_iterator,
                const_inner_iterator,
                typename inner_container::value_type const,
                access_begin<inner_container const>,
                access_end<inner_container const>
            > const_flatten_iterator;

        typedef boost::geometry::flatten_iterator
            <
                outer_iterator,
                inner_iterator,
                typename inner_container::value_type,
                access_begin<inner_container>,
                access_end<inner_container>
            > flatten_iterator;


        flatten_iterator begin(c.begin(), c.end());
        flatten_iterator end(c.end());
        const_flatten_iterator const_begin(begin);
        const_flatten_iterator const_end(end);
        const_begin = begin;
        const_end = end;

        std::size_t size(0);
        for (const_flatten_iterator it = const_begin; it != const_end; ++it)
        {
            ++size;
        }
        BOOST_CHECK( number_of_elements(c) == size );

        size = 0;
        for (flatten_iterator it = begin; it != end; ++it)
        {
            ++size;
        }
        BOOST_CHECK( number_of_elements(c) == size );

#ifdef GEOMETRY_TEST_DEBUG
        print_nested_container(std::cout, c.begin(), c.end(), "nested    :")
            << std::endl;
        print_container(std::cout, begin, end, "flattened :")
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
            const_flatten_iterator it = begin;
            const_outer_iterator outer_begin = c.begin();
            const_outer_iterator outer_end = c.end();
            for (const_outer_iterator outer = outer_begin;
                 outer != outer_end; ++outer)
            {
                const_inner_iterator inner_begin = outer->begin();
                const_inner_iterator inner_end = outer->end();
                for (const_inner_iterator inner = inner_begin;
                     inner != inner_end; ++inner, it++)
                    // it++, instead of ++it, on purpose here
                {
                    BOOST_CHECK( *it == *inner );
                }
            }
        }

        typedef typename std::iterator_traits
            <
                flatten_iterator
            >::value_type value_type;

        if ( begin != end )
        {
            flatten_iterator it_max = std::max_element(begin, end);
            const_flatten_iterator const_it_max =
                std::max_element(const_begin, const_end);

            BOOST_CHECK( it_max == const_it_max );
            BOOST_CHECK( *it_max == *const_it_max );

            value_type old_value = *const_begin;
            value_type new_value = *it_max + 1;

            *begin = *it_max + 1;
            const_outer_iterator outer = c.begin();
            while ( outer->begin() == outer->end() )
            {
                ++outer;
            }
            const_inner_iterator inner = outer->begin();
            
            BOOST_CHECK( *inner == new_value );

#ifdef GEOMETRY_TEST_DEBUG
            std::cout << std::endl;
            std::cout << "modified 1st element of 1st non-empty "
                      << "inner container:" << std::endl;
            print_nested_container(std::cout, c.begin(), c.end(), "nested   :")
                << std::endl;
            print_container(std::cout, begin, end, "flattened:")
                << std::endl;
#endif

            *begin = old_value;
            BOOST_CHECK( *inner == old_value );
        }


        if ( begin != end )
        {
#ifdef GEOMETRY_TEST_DEBUG
            std::cout << std::endl;
            std::cout << "odd elements removed:" << std::endl;
            print_nested_container(std::cout, c.begin(), c.end(),
                                   "nested before:")
                << std::endl;
            print_container(std::cout, begin, end,
                            "flattended before:")
                << std::endl;
#endif
            typename std::iterator_traits<flatten_iterator>::difference_type
                num_even = std::count_if(begin, end, is_even<int>());

            flatten_iterator new_end =
                std::remove_if(begin, end, is_odd<value_type>());

            std::size_t new_size(0);
            for (const_flatten_iterator it = const_begin; it != new_end; ++it)
            {
                ++new_size;
                BOOST_CHECK( !is_odd<value_type>()(*it) );
            }
            BOOST_CHECK( new_size == static_cast<std::size_t>(num_even) );

#ifdef GEOMETRY_TEST_DEBUG
            std::cout << std::endl;
            print_nested_container(std::cout, c.begin(), c.end(),
                                   "nested after (all elements) :")
                << std::endl;
            print_container(std::cout, begin, new_end, "flattened after :")
                << std::endl;
#endif
        }

#ifdef GEOMETRY_TEST_DEBUG
            std::cout << "====================" << std::endl << std::endl;
#endif
    }
};



// the actual test cases -- START
template <int CaseNumber>
struct test_case_per_container;

template<>
struct test_case_per_container<0>
{
    template <typename NestedContainer>
    static inline void apply(std::string const& case_id,
                             std::string const& container_id)
    {
        NestedContainer c;
        test_flatten_iterator::apply(c, case_id, container_id);
    }
};

template<>
struct test_case_per_container<1>
{
    template <typename NestedContainer>
    static inline void apply(std::string const& case_id,
                             std::string const& container_id)
    {
        NestedContainer c;
        for (int i = 0; i < 5; ++i)
        {
            c += typename NestedContainer::value_type();
        }
        test_flatten_iterator::apply(c, case_id, container_id);
    }
};

template<>
struct test_case_per_container<2>
{
    template <typename NestedContainer>
    static inline void apply(std::string const& case_id,
                             std::string const& container_id)
    {
        NestedContainer c;
        typename NestedContainer::value_type ic[4];

        ic[0] += 5,4,3,2,1;
        ic[1] += 6,7,8;
        ic[2] += 9;
        ic[3] += 9,8,7,6,5;
        c += ic[0],ic[1],ic[2],ic[3];

        test_flatten_iterator::apply(c, case_id, container_id);
    }
};

template<>
struct test_case_per_container<3>
{
    template <typename NestedContainer>
    static inline void apply(std::string const& case_id,
                             std::string const& container_id)
    {
        NestedContainer c;
        typename NestedContainer::value_type ic[20];

        ic[2] += 5,4,3,2,1;
        ic[3] += 6,7,8;
        ic[8] += 9;
        ic[9] += 9,8,7,6,5;
        ic[14] += 4,3,2,1;
        for (std::size_t i = 0; i < 20; ++i)
        {
            c += ic[i];
        }

        test_flatten_iterator::apply(c, case_id, container_id);
    }
};
// the actual test cases -- END



template <int CaseNumber>
inline void test_case_all_containers(std::string const& case_id)
{
    typedef typename std::vector<std::vector<int> > VV;
    typedef typename std::vector<std::list<int> > VL;
    typedef typename std::list<std::vector<int> > LV;
    typedef typename std::list<std::list<int> > LL;

#ifdef GEOMETRY_TEST_DEBUG
    std::cout << std::endl << std::endl;
#endif
    test_case_per_container<CaseNumber>::template apply<VV>(case_id, "VV");
    test_case_per_container<CaseNumber>::template apply<VL>(case_id, "VL");
    test_case_per_container<CaseNumber>::template apply<LV>(case_id, "LV");
    test_case_per_container<CaseNumber>::template apply<LL>(case_id, "LL");

#ifdef GEOMETRY_TEST_DEBUG
    std::cout << std::endl;
    std::cout << "********************************************************"
              << std::endl << std::endl;
#endif
}



BOOST_AUTO_TEST_CASE( test_flatten_iterator_all )
{
    test_case_all_containers<0>("empty");
    test_case_all_containers<1>("case1");
    test_case_all_containers<2>("case2");
    test_case_all_containers<3>("case3");
}
