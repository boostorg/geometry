// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_IO_SVG_WRITE_SVG_HPP
#define BOOST_GEOMETRY_IO_SVG_WRITE_SVG_HPP

#include <ostream>
#include <string>

#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>

#include <boost/geometry/core/exterior_ring.hpp>
#include <boost/geometry/core/interior_rings.hpp>
#include <boost/geometry/core/ring_type.hpp>

#include <boost/geometry/geometries/concepts/check.hpp>

/*!
\defgroup svg x Extension svg: Stream SVG (Scalable Vector Graphics)
*/

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace svg {


template <typename Point>
struct svg_point
{
    template <typename Char, typename Traits>
    static inline void apply(std::basic_ostream<Char, Traits>& os,
                Point const& p, std::string const& style, int size)
    {
        os << "<circle cx=\"" << p.x()
            << "\" cy=\"" << p.y()
            << "\" r=\"" << (size < 0 ? 5 : size)
            << "\" style=\"" << style << "\"/>";
    }
};


template <typename Box>
struct svg_box
{
    template <typename Char, typename Traits>
    static inline void apply(std::basic_ostream<Char, Traits>& os,
                Box const& box, std::string const& style, int size)
    {
        typedef typename coordinate_type<Box>::type coord_type;
        coord_type x = geometry::get<geometry::min_corner, 0>(box);
        coord_type y = geometry::get<geometry::min_corner, 1>(box);
        coord_type width = std::abs(geometry::get<geometry::max_corner, 0>(box) - x);
        coord_type height = std::abs(geometry::get<geometry::max_corner, 1>(box) - y);

        os << "<rect x=\"" << static_cast<int>(x)
           << "\" y=\"" << static_cast<int>(y)
           << "\" width=\"" << static_cast<int>(width)
           << "\" height=\"" << static_cast<int>(height)
           << "\" style=\"" << style << "\"/>";
    }
};


/*!
\brief Stream ranges as SVG
\note policy is used to select type (polyline/polygon)
*/
template <typename Range, typename Policy>
struct svg_range
{
    template <typename Char, typename Traits>
    static inline void apply(std::basic_ostream<Char, Traits>& os,
        Range const& range, std::string const& style, int size)
    {
        typedef typename boost::range_const_iterator<Range>::type iterator;

        bool first = true;

        os << "<" << Policy::prefix() << " points=\"";

        for (iterator it = boost::begin(range);
            it != boost::end(range);
            ++it, first = false)
        {
            os << (first ? "" : " " ) << it->x() << "," << it->y();
        }
        os << "\" style=\"" << style << Policy::style() << "\"/>";
    }
};



template <typename Polygon>
struct svg_poly
{
    template <typename Char, typename Traits>
    static inline void apply(std::basic_ostream<Char, Traits>& os,
        Polygon const& polygon, std::string const& style, int size)
    {
        typedef typename geometry::ring_type<Polygon>::type ring_type;
        typedef typename boost::range_const_iterator<ring_type>::type iterator_type;

        bool first = true;
        os << "<g fill-rule=\"evenodd\"><path d=\"";

        ring_type const& ring = geometry::exterior_ring(polygon);
        for (iterator_type it = boost::begin(ring);
            it != boost::end(ring);
            ++it, first = false)
        {
            os << (first ? "M" : " L") << " " << it->x() << "," << it->y();
        }

        // Inner rings:
        {
            typedef typename boost::range_const_iterator
                <
                    typename geometry::interior_type<Polygon>::type
                >::type ring_iterator_type;
            for (ring_iterator_type rit = boost::begin(interior_rings(polygon));
                 rit != boost::end(interior_rings(polygon));
                 ++rit)
            {
                first = true;
                for (iterator_type it = boost::begin(*rit);
                    it != boost::end(*rit);
                    ++it, first = false)
                {
                    os << (first ? "M" : " L") << " " << it->x() << "," << it->y();
                }
            }
        }
        os << " z \" style=\"" << style << "\"/></g>";

    }
};



struct prefix_linestring
{
    static inline const char* prefix() { return "polyline"; }
    static inline const char* style() { return ";fill:none"; }
};


struct prefix_ring
{
    static inline const char* prefix() { return "polygon"; }
    static inline const char* style() { return ""; }
};



}} // namespace detail::svg
#endif // DOXYGEN_NO_DETAIL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch {

/*!
\brief Dispatching base struct for SVG streaming, specialized below per geometry type
\details Specializations should implement a static method "stream" to stream a geometry
The static method should have the signature:

template <typename Char, typename Traits>
static inline void apply(std::basic_ostream<Char, Traits>& os, const G& geometry)
*/
template <typename GeometryTag, typename Geometry>
struct svg {};

template <typename Point>
struct svg<point_tag, Point> : detail::svg::svg_point<Point> {};

template <typename Box>
struct svg<box_tag, Box> : detail::svg::svg_box<Box> {};

template <typename Linestring>
struct svg<linestring_tag, Linestring>
    : detail::svg::svg_range<Linestring, detail::svg::prefix_linestring> {};

template <typename Ring>
struct svg<ring_tag, Ring>
    : detail::svg::svg_range<Ring, detail::svg::prefix_ring> {};

template <typename Polygon>
struct svg<polygon_tag, Polygon>
    : detail::svg::svg_poly<Polygon> {};

} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


/*!
\brief Generic geometry template manipulator class, takes corresponding output class from traits class
\ingroup svg
\details Stream manipulator, streams geometry classes as Virtual Earth shape
*/
template <typename G>
class svg_manipulator
{
public:

    inline svg_manipulator(G const& g, std::string const& style, int size)
        : m_geometry(g)
        , m_style(style)
        , m_size(size)
    {}

    template <typename Char, typename Traits>
    inline friend std::basic_ostream<Char, Traits>& operator<<(
                    std::basic_ostream<Char, Traits>& os, svg_manipulator const& m)
    {
        dispatch::svg
            <
                typename tag<G>::type, G
            >::apply(os, m.m_geometry, m.m_style, m.m_size);
        os.flush();
        return os;
    }

private:
    G const& m_geometry;
    std::string const& m_style;
    int m_size;
};

/*!
\brief Main svg function to stream geometries as SVG
\ingroup svg
*/
template <typename Geometry>
inline svg_manipulator<Geometry> svg(Geometry const& t, std::string const& style, int size = -1)
{
    concept::check<const Geometry>();

    return svg_manipulator<Geometry>(t, style, size);
}

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_IO_SVG_WRITE_SVG_HPP
