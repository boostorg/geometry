// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ITERATORS_CONCATENATE_ITERATOR_HPP
#define BOOST_GEOMETRY_ITERATORS_CONCATENATE_ITERATOR_HPP

#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/iterator.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/iterator/iterator_categories.hpp>


namespace boost { namespace geometry
{



template <typename Iterator1, typename Iterator2, typename Value>
struct concatenate_iterator
    : public boost::iterator_facade
        <
            concatenate_iterator<Iterator1, Iterator2, Value>,
            Value,
            boost::forward_traversal_tag
        >
{
private:
    Iterator1 m_it1, m_end1;
    Iterator2 m_it2;

    struct enabler {};

public:
    typedef Iterator1 first_iterator_type;
    typedef Iterator2 second_iterator_type;

    // default constructor
    concatenate_iterator() {}

    // for begin
    concatenate_iterator(Iterator1 it1, Iterator1 end1,
                         Iterator2 it2)
        : m_it1(it1), m_end1(end1), m_it2(it2)
    {}

    // for end
    concatenate_iterator(Iterator1 end1, Iterator2 end2)
        : m_it1(end1), m_end1(end1), m_it2(end2)
    {}

    template <typename OtherIt1, typename OtherIt2, typename OtherValue>
    concatenate_iterator
    (concatenate_iterator<OtherIt1, OtherIt2, OtherValue> const& other,
     typename boost::enable_if_c
         <
             boost::is_convertible<OtherIt1, Iterator1>::value
             && boost::is_convertible<OtherIt2, Iterator2>::value,
             enabler
         >::type = enabler())
        : m_it1(other.m_it1), m_end1(other.m_end1), m_it2(other.m_it2)
    {}



private:
    friend class boost::iterator_core_access;

    template <typename It1, typename It2, typename V>
    friend class concatenate_iterator;

    Value& dereference() const
    {
        if ( m_it1 == m_end1 )
        {
            return *m_it2;
        }
        return *m_it1;
    }

    bool equal(concatenate_iterator const& other) const
    {
        return m_it1 == other.m_it1 && m_it2 == other.m_it2;
    }

    void increment()
    {
        if ( m_it1 == m_end1 )
        {
            ++m_it2;
        }
        else
        {
            ++m_it1;
        }
    }
};



}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ITERATORS_CONCATENATE_ITERATOR_HPP
