// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_UTIL_POINT_CONCATENATE_ITERATOR_HPP
#define BOOST_GEOMETRY_UTIL_POINT_CONCATENATE_ITERATOR_HPP

#include <boost/iterator/iterator_facade.hpp>


namespace boost { namespace geometry
{


namespace util
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

private:
    friend class boost::iterator_core_access;

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


} // namespace util


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_UTIL_CONCATENATE_ITERATOR_HPP
