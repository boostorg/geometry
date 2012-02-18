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
template<typename Range>
class buffer_appender
{
public :
    typedef Range range_type;
    typedef typename geometry::point_type<Range>::type point_type;

    inline buffer_appender(Range& r)
        : m_range(r)
    {}

    inline void append(point_type const& point)
    {
        check(point);

        do_append(point);
    }

    inline void append_begin_join(point_type const& point)
    {
        DEBUG("begin join");
        check(point);

        cleanup();

        int index = do_append(point);
        m_pieces.push_back(piece('J', index));
    }

    inline void append_end_join(point_type const& point)
    {
        clean_split_offs();

        DEBUG("end join");
        do_append(point);
    }

    inline void append_begin_hooklet(point_type const& point)
    {
        DEBUG("begin hooklet");

        check(point);

        int index = do_append(point);
        if (!m_pieces.empty() && m_pieces.back().end == -1)
        {
            m_pieces.back().end = index;
        }
    }

    inline void append_end_hooklet(point_type const& point)
    {
        DEBUG("end hooklet");

        do_append(point);
    }


private :

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

    struct piece
    {
        char type; // For DEBUG, this will either go or changed into enum
        int begin, end;

        Range split_off;

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
        int result = boost::size(m_range);
        m_range.push_back(point);
        m_previous_point = point;
        return result;
    }

    inline void check(point_type const& point)
    {
        for (typename std::deque<piece>::const_reverse_iterator rit 
                    = m_pieces.rbegin();
            rit != m_pieces.rend();
            ++rit)
        {
            if (rit->end >= rit->begin
                && calculate_ip(point, *rit))
                {
                    // We HAVE to leave here 
                    // because the deque is cleared in between
                    return;
                }
        }

        // Second loop to check for intersections on intersected pieces
        for (typename std::deque<piece>::const_reverse_iterator rit 
                    = m_pieces.rbegin();
            rit != m_pieces.rend();
            ++rit)
        {
            if (rit->end >= rit->begin)
            {
                if (calculate_ip2(point, *rit))
                {
                    return;
                }
            }
        }

    }

    inline bool calculate_ip(point_type const& point, piece const& the_piece)
    {
        int const n = boost::size(m_range);
        if (the_piece.end >= n)
        {
            return false;
        }

        segment_type segment1(m_previous_point, point);

        // Walk backwards through list (chance is higher to have IP at the end)
        for (int i = the_piece.end - 1; i >= the_piece.begin; i--)
        {
            segment_type segment2(m_range[i], m_range[i + 1]);
            segment_intersection_points<point_type> is 
                                    = policy::apply(segment1, segment2);
            if (is.count == 1)
            {
                Range split_off;
                if (get_valid_split(is.intersections[0], i + 1, split_off))
                {
                    add_ip(is.intersections[0], i + 1, the_piece, split_off);
                }

                return true;
            }
        }

        return false;
    }

    inline bool calculate_ip2(point_type const& point, piece const& the_piece)
    {
        segment_type segment1(m_previous_point, point);

        // No IP found. Check if it is in the split off
        if (! the_piece.split_off.empty() && the_piece.type == 'I')
        {
            //typedef typename boost::reverse_iterator<Range const>::type ritt;
            typedef typename Range::const_reverse_iterator ritt;
            ritt previous = the_piece.split_off.rbegin();
            for (ritt rit = previous++; rit != the_piece.split_off.rend(); ++rit)
            {
                segment_type segment2(*rit, *previous);
                segment_intersection_points<point_type> is 
                                        = policy::apply(segment1, segment2);
                if (is.count == 1)
                {
                    Range split_off;
                    if (get_valid_split(is.intersections[0], the_piece.begin + 1, split_off))
                    {
                        DEBUG("split off from splitted off");

                        add_ip(is.intersections[0], the_piece.begin + 1, the_piece, split_off);

                        return true;
                    }

                }
                previous = rit;
            }
        }
        return false;
    }

    inline bool get_valid_split(point_type const& ip, int index, Range& split_off)
    {
        int const n = boost::size(m_range);
        split_off.push_back(ip);
        for (int j = index; j < n; j++)
        {
            split_off.push_back(m_range[j]);
        }
        split_off.push_back(ip);

        typename default_area_result<Range>::type area = geometry::area(split_off);
        if (area <= 0)
        {
            m_pieces.resize(0);
            return false;
        }
        return true;
    }

    inline void add_ip(point_type const& ip, int index,
                piece const& the_piece, Range const& split_off)
    {
        // Remove all points until this point, and add intersection point.
        m_range.resize(index);
        int ip_index = do_append(ip);

        // We first clear the piece list
        m_pieces.resize(0);

        // Add piece-with-intersection again (e.g. for #bowls >= 6 in unit tests)
        m_pieces.push_back(piece('F', the_piece.begin, ip_index));

        // Add IP as new starting point and include the cut-off piece
        // (we might intersect with that as well)
        m_pieces.push_back(piece('I', ip_index));
        m_pieces.back().split_off = split_off;
    }

    inline void cleanup()
    {
        if (m_pieces.size() > 0 && m_pieces.back().end == -1)
        {
            m_pieces.resize(0);
        }
    }

    inline void clean_split_offs()
    {
        for (typename std::deque<piece>::iterator it = m_pieces.begin();
            it != m_pieces.end();
            ++it)
        {
            it->split_off.resize(0);
        }
    }

    inline void DEBUG(std::string const& caption)
    {
#ifdef BOOST_GEOMETRY_DEBUG_BUFFER_APPENDER
        std::cout << "  " << caption;
        for (typename std::deque<piece>::iterator it 
                    = m_pieces.begin();
            it != m_pieces.end();
            ++it)
        {
            std::cout << " " << it->type << " " << it->begin << "-" << it->end 
                << " " << it->split_off.size();
        }
        std::cout << std::endl;
#endif
    }
};


}} // namespace detail::buffer
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_BUFFER_APPENDER_HPP
