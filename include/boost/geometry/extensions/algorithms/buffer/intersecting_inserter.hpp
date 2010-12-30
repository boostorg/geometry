// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_INTERSECTING_INSERTER_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_INTERSECTING_INSERTER_HPP


#include <cstddef>

#include <boost/range.hpp>

#include <boost/geometry/core/point_type.hpp>
#include <boost/geometry/core/ring_type.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace buffer
{


template
<
    typename Collection  // usually collection of rings
>
struct intersecting_inserter
{
    typedef typename boost::range_value<Collection>::type item_type;
    typedef typename geometry::ring_type<item_type>::type ring_type;
    typedef typename geometry::point_type<ring_type>::type point_type;

    intersecting_inserter(Collection& c)
        : m_collection(c)
        , m_index(0)
    {}

    inline void start_ring()
    {
        // clear current ring
        m_ring.clear();
        m_index = 0;
    }

    inline ring_type& get_ring()
    {
        return m_ring;
    }


    inline void insert(point_type const& point)
    {
        m_ring.push_back(point);
    }


    inline void close_and_insert_ring()
    {
        if (boost::size(m_ring) > 0)
        {
            // Close the ring
            point_type p = m_ring.front();
            insert(p);

            item_type poly;
            poly.outer() = m_ring;
            m_collection.push_back(poly);
        }
    }


private :
    Collection& m_collection;
    ring_type m_ring;
    std::size_t m_index;
};


}} // namespace detail::buffer
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_INTERSECTING_INSERTER_HPP
