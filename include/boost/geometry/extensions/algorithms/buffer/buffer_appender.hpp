// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_BUFFER_APPENDER_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_BUFFER_APPENDER_HPP


#include <cstddef>
#include <deque>

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



#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
    Mapper const& m_mapper;
    inline buffer_appender(Range& r, Mapper const& mapper)
        : m_range(r)
        , m_mapper(mapper)
#else
    inline buffer_appender(Range& r)
        : m_range(r)
#endif

    {}

    inline void append(point_type const& point)
    {
        if (! m_pieces.empty())
        {
            check(point);
        }
        do_append(point);
    }

    inline void append_begin_join(point_type const& point)
    {
        if (! check(point))
        {
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
            const_cast<Mapper&>(m_mapper).map(point, "fill:rgb(0,255,0);", 5);
#endif
        }

        cleanup();

        int index = do_append(point);
        m_pieces.push_back(piece('J', index));
    }

    inline void append_end_join(point_type const& point)
    {
        do_append(point);
    }

    inline void append_begin_hooklet(point_type const& point)
    {
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
        const_cast<Mapper&>(m_mapper).map(point, "fill:rgb(0,0,192);", 3);
#endif

        check(point);

        int index = do_append(point);
        if (!m_pieces.empty() && m_pieces.back().end == -1)
        {
            m_pieces.back().end = index;
        }
    }

    inline void append_end_hooklet(point_type const& point)
    {
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
        const_cast<Mapper&>(m_mapper).map(point, "fill:rgb(0,0,255);", 4);
#endif

        do_append(point);
    }


private :

    struct piece
    {
        char type; // For DEBUG, this will either go or changed into enum
        int begin, end;

        inline piece(char t = '\0', int b = -1, int e = -1)
            : type(t)
            , begin(b)
            , end(e)
        {}
    };

    Range& m_range;
    point_type m_previous_point;
    std::deque<piece> m_pieces;

    inline int do_append(point_type const& point)
    {
        //check(point);
        int result = boost::size(m_range);
        m_range.push_back(point);
        m_previous_point = point;
        return result;
    }

    inline bool check(point_type const& point)
    {
        for (typename std::deque<piece>::const_reverse_iterator rit 
                    = m_pieces.rbegin();
            rit != m_pieces.rend();
            ++rit)
        {
            if (rit->end >= rit->begin)
            {
                if (calculate(point, *rit))
                {
                    // We HAVE to leave here 
                    // because the deque is cleared in between
                    return true;
                }
            }
        }
        return false;
    }

    inline bool calculate(point_type const& point, piece const& the_piece)
    {
        int const n = boost::size(m_range);

#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
        const_cast<Mapper&>(m_mapper).map(point, "fill:rgb(255,0,0);", 4);
        for (int i = the_piece.begin; i <= the_piece.end && i < n; i++)
        {
            //const_cast<Mapper&>(m_mapper).map(m_range[i], "fill:rgb(0,255,255);", 3);
        }
#endif

        segment_type segment1(m_previous_point, point);

        for (int i = the_piece.begin;
            i < the_piece.end && i + 1 < n;
            i++)
        {
            segment_type segment2(m_range[i], m_range[i + 1]);
            segment_intersection_points<point_type> is 
                = policy::apply(segment1, segment2);
            if (is.count == 1)
            {
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
                const_cast<Mapper&>(m_mapper).map(is.intersections[0], "fill:rgb(255,0,255);", 4);

                // Draw red line of cut-off piece
                model::linestring<point_type> temp;
                temp.push_back(is.intersections[0]);
                for (int j = i + 1; j < n; j++)
                {
                    temp.push_back(m_range[j]);
                }
                temp.push_back(is.intersections[0]);
                const_cast<Mapper&>(m_mapper).map(temp, "fill:none;stroke:rgb(255,0,0);stroke-width:2");

#endif

                // Remove all points until this point, and add intersection point.
                m_range.resize(i + 1);

                // Add this IP also as first point on the deque.
                // We clear the deque - the indexes might be invalidated anyway
                int is_index = do_append(is.intersections[0]);

                // For many the index of intersection point is OK, but
                // for bowls >= 6 (see test-program) we need to intersect with the same segment again:
                int begin_index = the_piece.begin;

                m_pieces.resize(0);
                m_pieces.push_back(piece('F', begin_index, is_index));
                m_pieces.push_back(piece('I', is_index));

                return true;
            }
        }
        return false;
    }

    void cleanup()
    {
        if (m_pieces.size() > 0 && m_pieces.back().end == -1)
        {
            m_pieces.resize(0);
        }
    }
};


}} // namespace detail::buffer
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_BUFFER_APPENDER_HPP
