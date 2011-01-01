// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_RANGES_SEGMENT_RANGE_HPP
#define BOOST_GEOMETRY_RANGES_SEGMENT_RANGE_HPP


#include <boost/range.hpp>

#include <boost/geometry/iterators/segment_range_iterator.hpp>


namespace boost { namespace geometry
{


/*!
\brief Range, walking over the two points of a segment
\tparam Segment segment type
\ingroup ranges
*/
template <typename Segment>
class segment_range
{
public :
    typedef segment_range_iterator<Segment const> const_iterator;
    typedef segment_range_iterator<Segment const> iterator; // must be defined

    explicit segment_range(Segment const& segment)
        : m_begin(const_iterator(segment))
        , m_end(const_iterator(segment, true))
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


#ifndef DOXYGEN_NO_TRAITS_SPECIALIZATIONS

// All segment ranges can be handled as linestrings
namespace traits
{
    template<typename Segment>
    struct tag<segment_range<Segment> >
    {
        typedef linestring_tag type;
    };
}

#endif // DOXYGEN_NO_TRAITS_SPECIALIZATIONS



}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_RANGES_SEGMENT_RANGE_HPP
