// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_GEOMETRIES_ADAPTED_BOOST_POLYGON_POLYGON_ITERATOR_HPP
#define BOOST_GEOMETRY_GEOMETRIES_ADAPTED_BOOST_POLYGON_POLYGON_ITERATOR_HPP

#include <boost/iterator.hpp>
#include <boost/iterator/iterator_facade.hpp>



// ring iterator -> iterating over holes, delivering bp_ring's instead of normal polygon_data
template <typename Polygon, typename Ring>
class bp_iterator
    : public boost::iterator_facade
        <
            bp_iterator<Polygon, Ring>,
            Ring, // its value type
            boost::random_access_traversal_tag,
            Ring // its reference type
        >
{
public :
    typedef typename boost::polygon::polygon_with_holes_traits
        <
            Polygon
        >::iterator_holes_type it_type;

    explicit inline bp_iterator(it_type& it)
        : m_base(it)
    {
    }

    explicit inline bp_iterator()
    {
        //throw std::string("Constructed by default!");
    }


private:
    friend class boost::iterator_core_access;

    inline Ring dereference() const
    {
        return Ring(*this->m_base);
    }


    inline void increment() { ++m_base; }
    inline void decrement() { --m_base; }
    inline void advance(difference_type n) { m_base += n; }

    inline bool equal(bp_iterator<Polygon, Ring> const& other) const
    {
        return this->m_base == other.m_base;
    }

    it_type m_base;

};

#endif // BOOST_GEOMETRY_GEOMETRIES_ADAPTED_BOOST_POLYGON_POLYGON_ITERATOR_HPP

