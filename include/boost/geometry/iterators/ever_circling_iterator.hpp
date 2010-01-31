// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ITERATORS_EVER_CIRCLING_ITERATOR_HPP
#define BOOST_GEOMETRY_ITERATORS_EVER_CIRCLING_ITERATOR_HPP

#include <boost/iterator.hpp>
#include <boost/iterator/iterator_adaptor.hpp>
#include <boost/iterator/iterator_categories.hpp>

#include <boost/geometry/iterators/base.hpp>

namespace boost { namespace geometry
{

/*!
    \brief Iterator which ever circles through a range
    \tparam Iterator iterator on which this class is based on
    \ingroup iterators
    \details If the iterator arrives at range.end() it restarts from the
     beginning. So it has to be stopped in another way.
    Don't call for(....; it++) because it will turn in an endless loop
    \note Name inspired on David Bowie's
    "Chant Of The Ever Circling Skeletal Family"
*/
template <typename Iterator>
struct ever_circling_iterator :
    public detail::iterators::iterator_base
    <
        ever_circling_iterator<Iterator>,
        Iterator
    >
{
    friend class boost::iterator_core_access;

    explicit inline ever_circling_iterator(Iterator begin, Iterator end,
            bool skip_first = false)
      : m_begin(begin)
      , m_end(end)
      , m_skip_first(skip_first)
    {
        this->base_reference() = begin;
    }

    explicit inline ever_circling_iterator(Iterator begin, Iterator end, Iterator start,
            bool skip_first = false)
      : m_begin(begin)
      , m_end(end)
      , m_skip_first(skip_first)
    {
        this->base_reference() = start;
    }

    /// Navigate to a certain position, should be in [start .. end], it at end
    /// it will circle again.
    inline void moveto(Iterator it)
    {
        this->base_reference() = it;
        check_end();
    }

private:

    inline void increment(bool possibly_skip = true)
    {
        (this->base_reference())++;
        check_end(possibly_skip);
    }

    inline void check_end(bool possibly_skip = true)
    {
        if (this->base() == this->m_end)
        {
            this->base_reference() = this->m_begin;
            if (m_skip_first && possibly_skip)
            {
                increment(false);
            }
        }
    }

    Iterator m_begin;
    Iterator m_end;
    bool m_skip_first;
};


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ITERATORS_EVER_CIRCLING_ITERATOR_HPP
