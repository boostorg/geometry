// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_BUFFER_APPENDER_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_BUFFER_APPENDER_HPP


#include <cstddef>

#include <boost/range.hpp>

#include <boost/geometry/core/point_type.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace buffer
{

// Appends points to an output range (always a ring).
// On the way, special points can be marked, and marked points
// forming a hooklet, loop, curve, curl, or how you call it are checked on intersections.
template
    <
        typename Range
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
        , typename Mapper
#endif
    >
struct buffer_appender
{
    typedef Range range_type;

    typedef typename geometry::point_type<Range>::type point_type;
    typedef model::referring_segment<const point_type> segment_type;
    typedef strategy::intersection::relate_cartesian_segments
                <
                    policies::relate::segments_intersection_points
                        <
                            segment_type,
                            segment_type,
                            segment_intersection_points<point_type>
                        >
                > policy;


    int m_index_begin_join;
    int m_index_end_hooklet;
    int m_index_begin_hooklet;

#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
    Mapper const& m_mapper;
    inline buffer_appender(Range& r, Mapper const& mapper)
        : m_range(r)
        , m_mapper(mapper)
#else
    inline buffer_appender(Range& r)
        : m_range(r)
#endif

        , m_index_begin_join(-1)
        , m_index_end_hooklet(-1)
        , m_index_begin_hooklet(-1)
    {}

    inline void append(point_type const& point)
    {
        m_range.push_back(point);
        m_index_end_hooklet = -1;
        m_index_begin_hooklet = -1;
    }

    inline void append_end_hooklet(point_type const& point)
    {
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
        const_cast<Mapper&>(m_mapper).map(point, "fill:rgb(0,0,255);", 4);
#endif

        m_index_end_hooklet = boost::size(m_range);
        m_range.push_back(point);
    }

    inline void append_begin_hooklet(point_type const& point)
    {
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
        const_cast<Mapper&>(m_mapper).map(point, "fill:rgb(0,0,192);", 3);
#endif
        if (m_index_begin_hooklet > 0)
        {
            calculate(point, m_index_begin_hooklet - 1, boost::size(m_range) - 1);
        }

        m_index_begin_hooklet = boost::size(m_range);
        m_range.push_back(point);
    }

    inline void append_begin_join(point_type const& point)
    {
        if (m_index_begin_join >= 0 && m_index_end_hooklet >= 0)
        {
            calculate(point, m_index_begin_join, m_index_end_hooklet);
        }
        else
        {
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
            const_cast<Mapper&>(m_mapper).map(point, "fill:rgb(0,255,0);", 5);
#endif
        }

        m_index_begin_join = boost::size(m_range);
        m_range.push_back(point);
    }


private :
    Range& m_range;

    inline bool calculate(point_type const& point, int begin, int end)
    {
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
        const_cast<Mapper&>(m_mapper).map(point, "fill:rgb(255,0,0);", 4);
        for (int i = begin; i < end; i++)
        {
            //const_cast<Mapper&>(m_mapper).map(m_range[i], "fill:rgb(0,255,255);", 3);
        }
#endif

        segment_type segment1(m_range[end], point);

#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
        //const_cast<Mapper&>(m_mapper).map(segment1, "stroke:rgb(0,255,255);stroke-width:4");
#endif

        for (int i = begin; i < end - 1; i++)
        {
            segment_type segment2(m_range[i], m_range[i + 1]);
            segment_intersection_points<point_type> is = policy::apply(segment1, segment2);
            if (is.count > 0)
            {
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
                const_cast<Mapper&>(m_mapper).map(is.intersections[0], "fill:rgb(255,0,255);", 4);
#endif

                // Remove all points until this point, and add intersection point.
                m_range.resize(i + 1);
                m_range.push_back(is.intersections[0]);
                m_index_end_hooklet = -1;
                return true;
            }
        }
        return false;
    }
};


}} // namespace detail::buffer
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_BUFFER_APPENDER_HPP
