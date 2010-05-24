// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ITERATORS_CLOSING_ITERATOR_HPP
#define BOOST_GEOMETRY_ITERATORS_CLOSING_ITERATOR_HPP

#include <boost/range.hpp>
#include <boost/iterator.hpp>
#include <boost/iterator/iterator_adaptor.hpp>
#include <boost/iterator/iterator_categories.hpp>

#include <boost/geometry/iterators/base.hpp>


namespace boost { namespace geometry
{

/*!
    \brief Iterator which iterates through a range, but adds first element at end of the range
    \tparam Range range on which this class is based on
    \ingroup iterators
    \note Use with "closing_iterator<Range> or "closing_iterator<Range const> 
            to get non-const / const behaviour
    \note This class is normally used from "closeable_view" if Close==true
*/
template <typename Range>
struct closing_iterator 
    : public detail::iterators::iterator_base
        <
            closing_iterator<Range>,
            typename boost::range_iterator<Range>::type,
            boost::forward_traversal_tag
        >
{
    friend class boost::iterator_core_access;

    explicit inline closing_iterator(Range& range)
        : m_range(range)
        , m_beyond(false)
        , m_end(boost::end(m_range))
    {
        this->base_reference() = boost::begin(m_range);
    }

    // Constructor to indicate the end of a range
    explicit inline closing_iterator(Range& range, bool)
        : m_range(range)
        , m_beyond(true)
        , m_end(boost::end(m_range))
    {
        this->base_reference() = m_end;
    }

    inline bool equal(closing_iterator const& other) const
    {
        return this->base() == other.base() 
            && this->m_beyond == other.m_beyond;
    }


private:

    inline void increment()
    {
        if (m_beyond)
        {
            this->base_reference() = m_end;
        }
        else if (this->base() != m_end)
        {
            (this->base_reference())++;

            if (this->base() == m_end)
            {
                // Now beyond last position -> set to "begin" again
                // and set flag "beyond" such that next increment
                // will finish traversal
                this->base_reference() = boost::begin(m_range);
                m_beyond = true;
            }
        }
    }

    Range& m_range;
    bool m_beyond;
    typename boost::range_iterator<Range>::type m_end;
};


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ITERATORS_CLOSING_ITERATOR_HPP
