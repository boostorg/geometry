// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_SEGMENT_RANGE_ITERATOR_HPP
#define BOOST_GEOMETRY_SEGMENT_RANGE_ITERATOR_HPP

#include <boost/assert.hpp>

#include <boost/iterator.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/iterator/iterator_categories.hpp>

#include <boost/geometry/core/point_type.hpp>
#include <boost/geometry/algorithms/assign.hpp>


namespace boost { namespace geometry
{

/*!
    \brief Iterator which adapts a segment (two points) as iterator
    \tparam Segment segment type on which this iterator is based on
    \note It is always const. We cannot dereference something non-const
        (at least not without doing tricks as returning assignables)
    \ingroup iterators
*/
template <typename Segment>
struct segment_range_iterator
    : public boost::iterator_facade
    <
        segment_range_iterator<Segment>,
        typename point_type<Segment>::type const,
        boost::random_access_traversal_tag
    >
{
    // Default constructor is required to check concept of Range
    // (used in checking linestring/ring concepts)
    inline segment_range_iterator()
        : m_index(-1)
        , m_segment_address(NULL)
    {
    }

    // Operator= is required to check concept of Range
    inline segment_range_iterator<Segment>& operator=(segment_range_iterator<Segment> const& source)
    {
        m_index = source.m_index;
        m_segment_address = source.m_segment_address;
        return *this;
    }


    explicit inline segment_range_iterator(Segment const& segment)
        : m_index(0)
        , m_segment_address(&segment)
    {
        init(segment);
    }

    // Constructor to indicate the end of a segment
    explicit inline segment_range_iterator(Segment const& segment, bool)
        : m_index(2)
        , m_segment_address(&segment)
    {
        init(segment);
    }

    typedef std::ptrdiff_t difference_type;

private:
    friend class boost::iterator_core_access;
    typedef typename point_type<Segment>::type point_type;

    inline point_type const& dereference() const
    {
        if (m_index >= 0 && m_index <= 1)
        {
            return m_points[m_index];
        }

        BOOST_ASSERT(!"Should not occur"); // Probably throw here.
        return m_points[0];
    }

    inline bool equal(segment_range_iterator<Segment> const& other) const
    {
        return m_segment_address == this->m_segment_address
            && other.m_index == this->m_index;
    }

    inline void increment()
    {
        m_index++;
    }

    inline void decrement()
    {
        m_index--;
    }

    inline difference_type distance_to(segment_range_iterator<Segment> const& other) const
    {
        return other.m_index - this->m_index;
    }

    inline void init(Segment const& segment)
    {
        assign_point_from_index<0>(segment, m_points[0]);
        assign_point_from_index<1>(segment, m_points[1]);
    }

    // We HAVE TO copy the points, because a segment does not need
    // to consist of two points,
    // and we are expected to return a point here
    point_type m_points[2];
    int m_index;
    Segment const* m_segment_address;
};


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_SEGMENT_RANGE_ITERATOR_HPP
