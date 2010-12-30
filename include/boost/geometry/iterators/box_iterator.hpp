// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_BOX_ITERATOR_HPP
#define BOOST_GEOMETRY_BOX_ITERATOR_HPP

#include <boost/iterator.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/iterator/iterator_categories.hpp>

#include <boost/geometry/core/point_type.hpp>
#include <boost/geometry/algorithms/assign.hpp>


namespace boost { namespace geometry
{

/*!
    \brief Iterator which adapts a box (four points + closing) as iterator
    \tparam Box box type on which this iterator is based on
    \note It is always const. We cannot dereference something non-const
        (at least not without doing tricks as returning assignables)
    \ingroup iterators
*/
template <typename Box>
struct box_iterator
    : public boost::iterator_facade
    <
        box_iterator<Box>,
        typename point_type<Box>::type const,
        boost::random_access_traversal_tag
    >
{
    explicit inline box_iterator(Box const& box)
        : m_index(0)
        , m_box_address(&box)
    {
        init(box);
    }

    // Constructor to indicate the end of a box
    explicit inline box_iterator(Box const& box, bool)
        : m_index(5)
        , m_box_address(&box)
    {
        init(box);
    }

    typedef std::ptrdiff_t difference_type;

private:
    friend class boost::iterator_core_access;
    typedef typename point_type<Box>::type point_type;

    inline point_type const& dereference() const
    {
        if (m_index >= 0 && m_index <= 3)
        {
            return m_points[m_index];
        }
        // If it index is 4, or other, return first point
        return m_points[0];
    }

    inline bool equal(box_iterator<Box> const& other) const
    {
        return m_box_address == this->m_box_address
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

    inline difference_type distance_to(box_iterator<Box> const& other) const
    {
        return other.m_index - this->m_index;
    }

    inline void init(Box const& box)
    {
        // asb -> lower_left, lower_right, upper_left, upper_right
        // we want: clockwise
        assign_box_corners(box, m_points[0], m_points[3], m_points[1], m_points[2]);
    }

    // Copy points here - box might define them otherwise
    point_type m_points[4];
    int m_index;
    Box const* const m_box_address;
};


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_BOX_ITERATOR_HPP
