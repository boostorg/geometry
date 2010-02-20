// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_BUFFER_HPP
#define BOOST_GEOMETRY_ALGORITHMS_BUFFER_HPP

#include <cstddef>

#include <boost/numeric/conversion/cast.hpp>


#include <boost/geometry/algorithms/clear.hpp>
#include <boost/geometry/algorithms/detail/disjoint.hpp>
#include <boost/geometry/arithmetic/arithmetic.hpp>
#include <boost/geometry/geometries/concepts/check.hpp>
#include <boost/geometry/geometries/segment.hpp>
#include <boost/geometry/util/math.hpp>


/*!
\defgroup buffer buffer: calculate buffer of a geometry
\par Source description:
- OGC: Returns a geometric object that represents all Points whose distance
from this geometric object is less than or equal to distance. Calculations are in the spatial reference system of
this geometric object. Because of the limitations of linear interpolation, there will often be some relatively
small error in this distance, but it should be near the resolution of the coordinates used
\see http://en.wikipedia.org/wiki/Buffer_(GIS)
*/
namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace buffer
{

template <typename BoxIn, typename BoxOut, typename T, std::size_t C, std::size_t D, std::size_t N>
struct box_loop
{
    typedef typename coordinate_type<BoxOut>::type coordinate_type;

    static inline void apply(BoxIn const& box_in, T const& distance, BoxOut& box_out)
    {
        coordinate_type d = distance;
        set<C, D>(box_out, get<C, D>(box_in) + d);
        box_loop<BoxIn, BoxOut, T, C, D + 1, N>::apply(box_in, distance, box_out);
    }
};

template <typename BoxIn, typename BoxOut, typename T, std::size_t C, std::size_t N>
struct box_loop<BoxIn, BoxOut, T, C, N, N>
{
    static inline void apply(BoxIn const&, T const&, BoxOut&) {}
};

// Extends a box with the same amount in all directions
template<typename BoxIn, typename BoxOut, typename T>
inline void buffer_box(BoxIn const& box_in, T const& distance, BoxOut& box_out)
{
    assert_dimension_equal<BoxIn, BoxOut>();

    static const std::size_t N = dimension<BoxIn>::value;

    box_loop<BoxIn, BoxOut, T, min_corner, 0, N>::apply(box_in, -distance, box_out);
    box_loop<BoxIn, BoxOut, T, max_corner, 0, N>::apply(box_in, +distance, box_out);
}




template <typename RingInput, typename RingOutput, typename JoinStrategy>
struct ring_buffer
{
    template <typename Point, typename Line1, typename Line2 = Line1>
    struct line_line_intersection
    {
        template <typename A, typename B, typename C, typename D>
        static inline A det(A const& a, B const& b, C const& c, D const& d)
        {
            return a * d - b * c;
        }

        static inline bool apply(Line1 const& line1, Line2 const& line2, Point& p)
        {
            // See http://mathworld.wolfram.com/Line-LineIntersection.html
            typedef typename coordinate_type<Point>::type coordinate_type;
            coordinate_type x1 = get<0,0>(line1), y1 = get<0,1>(line1);
            coordinate_type x2 = get<1,0>(line1), y2 = get<1,1>(line1);
            coordinate_type x3 = get<0,0>(line2), y3 = get<0,1>(line2);
            coordinate_type x4 = get<1,0>(line2), y4 = get<1,1>(line2);

            coordinate_type denominator = det(x1 - x2, y1 - y2, x3 - x4, y3 - y4);

            // If denominator is zero, segments are parallel.
            // We have context information, so know that it should then
            // be the case that line1.p2 == line2.p1, and that is the
            // intersection point.
            if (geometry::math::equals(denominator, 0.0))
            {
                set<0>(p, x2);
                set<1>(p, y2);
                return true;
            }

            coordinate_type d1 = det(x1, y1, x2, y2);
            coordinate_type d2 = det(x3, y3, x4, y4);
            coordinate_type px = det(d1, x1 - x2, d2, x3 - x4) / denominator;
            coordinate_type py = det(d1, y1 - y2, d2, y3 - y4) / denominator;

            set<0>(p, px);
            set<1>(p, py);

            if (abs(denominator) < 1e-7)
            {
                std::cout << "small " << denominator << std::endl;
            }
            return abs(denominator) > 1e-7;
        }
    };



#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
    template <typename Mapper>
#endif
    static inline void apply(RingInput const& ring, RingOutput& buffered,
            double distance, // TODO: change coordinate type
            JoinStrategy const& join_strategy
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
            , Mapper& mapper
#endif
            )
    {
        typedef typename point_type<RingOutput>::type output_point_type;
        typedef segment<output_point_type const> segment_type;
        typedef typename boost::range_iterator
            <
                RingInput const
            >::type iterator_type;

        output_point_type previous_p1, previous_p2;
        output_point_type first_p1, first_p2;
        bool first = true;

        int index = 0;

        iterator_type it = boost::begin(ring);
        for (iterator_type prev = it++;
            it != boost::end(ring); ++it)
        {
            if (! detail::equals::equals_point_point(*prev, *it))
            {
                bool skip = false;

                // Generate a block along (int most cases to the left of) the segment
                typedef typename coordinate_type<output_point_type>::type coordinate_type;

                // Simulate a vector d (dx,dy)
                coordinate_type dx = get<0>(*it) - get<0>(*prev);
                coordinate_type dy = get<1>(*it) - get<1>(*prev);


                // For normalization [0,1] (=dot product d.d, sqrt)
                coordinate_type length = sqrt(dx * dx + dy * dy);

                // Because coordinates are not equal, length should not be zero
                BOOST_ASSERT((! geometry::math::equals(length, 0)));

                // Generate the normalized perpendicular p, to the left (ccw)
                coordinate_type px = -dy / length;
                coordinate_type py = dx / length;

                output_point_type p1, p2;

                coordinate_type d = distance;

                set<0>(p2, get<0>(*it) + px * d);
                set<1>(p2, get<1>(*it) + py * d);

                set<0>(p1, get<0>(*prev) + px * d);
                set<1>(p1, get<1>(*prev) + py * d);

                {
                    RingOutput block;
                    block.push_back(*prev);
                    block.push_back(*it);
                    block.push_back(p2);
                    block.push_back(p1);
                    block.push_back(*prev);

    #ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
                    mapper.map(block, "opacity:0.4;fill:rgb(0,255,0);stroke:rgb(0,0,0);stroke-width:1");
    #endif
                }

                if (! first)
                {
                    output_point_type p;
                    segment_type s1(p1, p2);
                    segment_type s2(previous_p1, previous_p2);
                    if (line_line_intersection<output_point_type, segment_type>::apply(s1, s2, p))
                    {
                        join_strategy.apply(p, *prev, previous_p2, p1, distance, buffered);
                        {
        #ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
                            mapper.map(p, "fill:rgb(0,0,0);", 3);

                            std::ostringstream out;
                            out << index++;
                            mapper.text(p, out.str(), "fill:rgb(0,0,0);font-family='Arial';", 5, 5);
        #endif
                        }
                    }
                    else
                    {
                        skip = false;
                    }
                }
                else
                {
                    first = false;
                    first_p1 = p1;
                    first_p2 = p2;
                }

                if (! skip)
                {
                    previous_p1 = p1;
                    previous_p2 = p2;
                    prev = it;
                }
            }
        }

        // Last one
        {
            output_point_type p;
            segment_type s1(previous_p1, previous_p2);
            segment_type s2(first_p1, first_p2);
            line_line_intersection<output_point_type, segment_type>::apply(s1, s2, p);

            join_strategy.apply(p, *boost::begin(ring), previous_p2, first_p1, distance, buffered);

#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
            mapper.map(p, "fill:rgb(0,0,0);", 3);
            std::ostringstream out;
            out << index++;
            mapper.text(p, out.str(), "fill:rgb(0,0,0);font-family='Arial';", 5, 5);
#endif
        }

        // Close the generated buffer
        {
            output_point_type p = *boost::begin(buffered);
            buffered.push_back(p);
        }
    }
};



template <typename PolygonInput, typename PolygonOutput, typename JoinStrategy>
struct polygon_buffer
{
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
    template <typename Mapper>
#endif
    static inline void apply(PolygonInput const& polygon, PolygonOutput& buffered,
            double distance, JoinStrategy const& join_strategy
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
            , Mapper& mapper
#endif
            )
    {
        boost::geometry::clear(buffered);

        typedef typename ring_type<PolygonInput>::type input_ring_type;
        typedef typename ring_type<PolygonOutput>::type output_ring_type;

        typedef ring_buffer<input_ring_type, output_ring_type, JoinStrategy> policy;
        policy::apply(exterior_ring(polygon), exterior_ring(buffered),
                distance, join_strategy
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
                , mapper
#endif
                );

        for (typename boost::range_iterator
                <
                    typename interior_type<PolygonInput>::type const
                >::type it = boost::begin(interior_rings(polygon));
             it != boost::end(interior_rings(polygon));
             ++it)
        {
            output_ring_type ring;
            policy::apply(*it, ring, distance, join_strategy
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
                , mapper
#endif
                );
            interior_rings(buffered).push_back(ring);
        }
    }
};



}} // namespace detail::buffer
#endif // DOXYGEN_NO_DETAIL

#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template <typename TagIn, typename TagOut, typename Input, typename T, typename Output>
struct buffer {};


template <typename BoxIn, typename T, typename BoxOut>
struct buffer<box_tag, box_tag, BoxIn, T, BoxOut>
{
    static inline void apply(BoxIn const& box_in, T const& distance,
                T const& chord_length, BoxIn& box_out)
    {
        detail::buffer::buffer_box(box_in, distance, box_out);
    }
};

// Many things to do. Point is easy, other geometries require self intersections
// For point, note that it should output as a polygon (like the rest). Buffers
// of a set of geometries are often lateron combined using a "dissolve" operation.
// Two points close to each other get a combined kidney shaped buffer then.

} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


/*!
    \brief Calculate buffer (= new geometry) around specified distance of geometry
    \ingroup buffer
    \param geometry_in input geometry
    \param distance the distance used in buffer
    \param chord_length length of the chord's in the generated arcs around points or bends
    \param geometry_out buffered geometry
    \note Currently only implemented for box, the trivial case, but still useful
    \par Use case:
        BOX + distance -> BOX: it is allowed that "geometry_out" the same object as "geometry_in"
 */
template <typename Input, typename Output, typename T>
inline void buffer(Input const& geometry_in, Output& geometry_out,
            T const& distance, T const& chord_length = -1)
{
    concept::check<const Input>();
    concept::check<Output>();

    dispatch::buffer
        <
            typename tag<Input>::type,
            typename tag<Output>::type,
            Input,
            T,
            Output
        >::apply(geometry_in, distance, chord_length, geometry_out);
}

/*!
    \brief Calculate and return buffer (= new geometry) around specified distance of geometry
    \ingroup buffer
    \param geometry input geometry
    \param distance the distance used in buffer
    \param chord_length length of the chord's in the generated arcs around points or bends
    \return the buffered geometry
    \note See also: buffer
 */
template <typename Output, typename Input, typename T>
Output make_buffer(Input const& geometry, T const& distance, T const& chord_length = -1)
{
    concept::check<const Input>();
    concept::check<Output>();

    Output geometry_out;

    dispatch::buffer
        <
            typename tag<Input>::type,
            typename tag<Output>::type,
            Input,
            T,
            Output
        >::apply(geometry, distance, chord_length, geometry_out);

    return geometry_out;
}

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_BUFFER_HPP
