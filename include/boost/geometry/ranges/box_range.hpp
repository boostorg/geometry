// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_RANGES_BOX_RANGE_HPP
#define BOOST_GEOMETRY_RANGES_BOX_RANGE_HPP


#include <boost/range.hpp>

#include <boost/geometry/iterators/box_iterator.hpp>


namespace boost { namespace geometry
{


template <typename Box>
class box_range
{
public :
    typedef box_iterator<Box const> const_iterator;
    typedef box_iterator<Box const> iterator; // must be defined

    explicit box_range(Box const& box)
        : m_begin(const_iterator(box))
        , m_end(const_iterator(box, true))
    {
    }

    const_iterator begin() const { return m_begin; }
    const_iterator end() const { return m_end; }

    // It may not be used non-const, so comment this:
    //iterator begin() { return m_begin; }
    //iterator end() { return m_end; }

private :
    const_iterator m_begin, m_end;
};


// All box ranges can be handled as rings
namespace traits
{
    template<typename Box>
    struct tag<box_range<Box> >
    {
        typedef ring_tag type;
    };
}



}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_RANGES_BOX_RANGE_HPP
