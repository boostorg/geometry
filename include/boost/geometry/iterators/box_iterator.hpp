// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2011 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2011 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2011 Mateusz Loskot, London, UK.

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

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
    // Default constructor is required to check concept of Range
    inline box_iterator()
        : m_index(-1)
        , m_box_address(NULL)
    {
    }
    
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

    // Operator= is required to check concept of Range
    inline box_iterator<Box>& operator=(box_iterator<Box> const& source)
    {
        m_index = source.m_index;
        m_box_address = source.m_box_address;
        return *this;
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
        detail::assign_box_corners_oriented<false>(box, m_points);
    }

    // Copy points here - box might define them otherwise
    point_type m_points[4];
    int m_index;
    Box const* m_box_address;
};


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_BOX_ITERATOR_HPP
