// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_UTIL_FLATTEN_ITERATOR_HPP
#define BOOST_GEOMETRY_UTIL_FLATTEN_ITERATOR_HPP

#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/iterator/iterator_facade.hpp>


namespace boost { namespace geometry
{


namespace util
{


template
<
    typename OuterIterator,
    typename InnerIterator,
    typename Value,
    typename AccessInnerBegin,
    typename AccessInnerEnd
>
struct flatten_iterator
    : public boost::iterator_facade
        <
            flatten_iterator
                <
                    OuterIterator,
                    InnerIterator,
                    Value,
                    AccessInnerBegin,
                    AccessInnerEnd>,
            Value,
            boost::forward_traversal_tag
        >
{
private:
    OuterIterator m_outer_it, m_outer_end;
    InnerIterator m_inner_it;

    struct enabler {};

public:
    typedef OuterIterator outer_iterator_type;
    typedef InnerIterator inner_iterator_type;

    // default constructor
    flatten_iterator() {}

    // for begin
    flatten_iterator(OuterIterator outer_it, OuterIterator outer_end)
        : m_outer_it(outer_it), m_outer_end(outer_end)
    {
        if ( m_outer_it != m_outer_end )
        {
            m_inner_it = AccessInnerBegin::apply(*m_outer_it);
            //        m_inner_it = AccessInnerBegin(*m_outer_it);
        }
        advance_through_empty();
    }

    // for end
    flatten_iterator(OuterIterator outer_end)
        : m_outer_it(outer_end), m_outer_end(outer_end)
    {}

    template
    <
        typename OtherOuterIterator,
        typename OtherInnerIterator,
        typename OtherValue
    >
    explicit flatten_iterator(flatten_iterator
                              <
                                  OtherOuterIterator,
                                  OtherInnerIterator,
                                  OtherValue,
                                  AccessInnerBegin,
                                  AccessInnerEnd
                              > const& other,
                              typename boost::enable_if
                              <
                                  boost::is_convertible
                                  <
                                      OtherOuterIterator*, OuterIterator*
                                  >, enabler
                              >::type = enabler()
                              )
        : m_outer_it(other.m_outer_it),
          m_outer_end(other.m_outer_end),
          m_inner_it(other.m_inner_it)
    {}

    template
    <
        typename OtherOuterIterator,
        typename OtherInnerIterator,
        typename OtherValue
    >
    flatten_iterator operator=(flatten_iterator
                               <
                                   OtherOuterIterator,
                                   OtherInnerIterator,
                                   OtherValue,
                                   AccessInnerBegin,
                                   AccessInnerEnd
                               > const& other)
    {
        m_outer_it = other.m_outer_it;
        m_outer_end = other.m_outer_end;
        m_inner_it = other.m_inner_it;
        return *this;
    }

private:
    friend class boost::iterator_core_access;

    template
    <
        typename OuterIterator1,
        typename InnerIterator1,
        typename Value1,
        typename AccessInnerBegin1,
        typename AccessInnerEnd1
    >
    friend class flatten_iterator;

    static inline bool empty(OuterIterator outer_it)
    {
        return
            AccessInnerBegin::apply(*outer_it) == AccessInnerEnd::apply(*outer_it);
        //            AccessInnerBegin(*outer_it) == AccessInnerEnd(*outer_it);
    }

    void advance_through_empty()
    {
        while ( m_outer_it != m_outer_end && empty(m_outer_it) )
        {
            ++m_outer_it;
        }

        if ( m_outer_it != m_outer_end )
        {
            m_inner_it = AccessInnerBegin::apply(*m_outer_it);
            //            m_inner_it = AccessInnerBegin(*m_outer_it);
        }
    }

    Value& dereference() const
    {
        BOOST_ASSERT( m_outer_it != m_outer_end );
        BOOST_ASSERT( m_inner_it != AccessInnerEnd::apply(*m_outer_it) );
        //        BOOST_ASSERT( m_inner_it != AccessInnerEnd(*m_outer_it) );
        return *m_inner_it;
    }


    template
    <
        typename OtherOuterIterator,
        typename OtherInnerIterator,
        typename OtherValue
    >
    bool equal(flatten_iterator
               <
                   OtherOuterIterator,
                   OtherInnerIterator,
                   OtherValue,
                   AccessInnerBegin,
                   AccessInnerEnd
               > const& other) const
    {
        if ( this->m_outer_it != other.m_outer_it )
        {
            return false;
        }

        if ( this->m_outer_it == m_outer_end )
        {
            return true;
        }

        return this->m_inner_it == other.m_inner_it;
    }

    void increment()
    {
        BOOST_ASSERT( m_outer_it != m_outer_end );
        BOOST_ASSERT( m_inner_it != AccessInnerEnd::apply(*m_outer_it) );
        //        BOOST_ASSERT( m_inner_it != AccessInnerEnd(*m_outer_it) );

        ++m_inner_it;
        if ( m_inner_it == AccessInnerEnd::apply(*m_outer_it) )
        //if ( m_inner_it == AccessInnerEnd(*m_outer_it) )
        {
            ++m_outer_it;
            advance_through_empty();
        }
    }
};


} // namespace util


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_UTIL_FLATTEN_INTERATOR_HPP
