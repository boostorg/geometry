// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2009-2010, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_IO_SVG_MAPPER_HPP
#define BOOST_GEOMETRY_IO_SVG_MAPPER_HPP

#include <cstdio>

//#include <boost/geometry/geometry.hpp>
#include <boost/geometry/algorithms/envelope.hpp>
#include <boost/geometry/algorithms/transform.hpp>
#include <boost/geometry/algorithms/num_points.hpp>
#include <boost/geometry/strategies/transform.hpp>

#include <boost/geometry/strategies/transform/map_transformer.hpp>

#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/linestring.hpp>
#include <boost/geometry/geometries/linear_ring.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/point_xy.hpp>



#include <boost/geometry/multi/core/tags.hpp>

#include <boost/geometry/extensions/io/svg/write_svg.hpp>


namespace boost { namespace geometry
{



#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template <typename GeometryTag, bool IsMulti, typename Geometry>
struct svg_map
{
};


template <typename Point>
struct svg_map<boost::geometry::point_tag, false, Point>
{
    template <typename TransformStrategy>
    static inline void apply(std::ostream& stream,
                    std::string const& style, int size,
                    Point const& point, TransformStrategy const& strategy)
    {
        boost::geometry::point_xy<int> p;
        boost::geometry::transform(point, p, strategy);
        stream << boost::geometry::svg(p, style, size) << std::endl;
    }
};

template <typename Box>
struct svg_map<boost::geometry::box_tag, false, Box>
{
    template <typename TransformStrategy>
    static inline void apply(std::ostream& stream,
                    std::string const& style, int size,
                    Box const& box, TransformStrategy const& strategy)
    {

        typename boost::geometry::point_type<Box>::type p1, p2;
        boost::geometry::set<0>(p1, boost::geometry::get<boost::geometry::min_corner, 0>(box));
        boost::geometry::set<1>(p1, boost::geometry::get<boost::geometry::min_corner, 1>(box));
        boost::geometry::set<0>(p2, boost::geometry::get<boost::geometry::max_corner, 0>(box));
        boost::geometry::set<1>(p2, boost::geometry::get<boost::geometry::max_corner, 1>(box));

        boost::geometry::box<boost::geometry::point_xy<int> > ibox;
        boost::geometry::transform(box, ibox, strategy);

        stream << boost::geometry::svg(ibox, style, size) << std::endl;
    }
};


template <typename Range1, typename Range2>
struct svg_map_range
{
    template <typename TransformStrategy>
    static inline void apply(std::ostream& stream,
                std::string const& style, int size,
                Range1 const& range, TransformStrategy const& strategy)
    {
        Range2 irange;
        boost::geometry::transform(range, irange, strategy);
        stream << boost::geometry::svg(irange, style, size) << std::endl;
    }
};



template <typename Ring>
struct svg_map<boost::geometry::ring_tag, false, Ring>
    : svg_map_range<Ring, boost::geometry::linear_ring<boost::geometry::point_xy<int> > >
{};

template <typename Linestring>
struct svg_map<boost::geometry::linestring_tag, false, Linestring>
    : svg_map_range<Linestring, boost::geometry::linestring<boost::geometry::point_xy<int> > >
{};


template <typename Polygon>
struct svg_map<boost::geometry::polygon_tag, false, Polygon>
{
    template <typename TransformStrategy>
    static inline void apply(std::ostream& stream,
                    std::string const& style, int size,
                    Polygon const& polygon, TransformStrategy const& strategy)
    {
        boost::geometry::polygon<boost::geometry::point_xy<int> > ipoly;
        boost::geometry::transform(polygon, ipoly, strategy);
        stream << boost::geometry::svg(ipoly, style, size) << std::endl;
    }
};

template <typename Tag, typename Multi>
struct svg_map<Tag, true, Multi>
{
    template <typename TransformStrategy>
    static inline void apply(std::ostream& stream,
                    std::string const& style, int size,
                    Multi const& multi, TransformStrategy const& strategy)
    {
        for (typename boost::range_const_iterator<Multi>::type it
            = boost::begin(multi);
            it != boost::end(multi);
            ++it)
        {
            svg_map
                <
                    typename boost::geometry::single_tag<Tag>::type,
                    false,
                    typename boost::range_value<Multi>::type
                >::apply(stream, style, size, *it, strategy);
        }
    }
};



} // namespace dispatch
#endif



template <typename Geometry, typename TransformStrategy>
inline void svg_map(std::ostream& stream,
            std::string const& style, int size,
            Geometry const& geometry, TransformStrategy const& strategy)
{
    dispatch::svg_map
        <
            typename boost::geometry::tag<Geometry>::type,
            boost::geometry::is_multi<Geometry>::type::value,
            typename boost::remove_const<Geometry>::type
        >::apply(stream, style, size, geometry, strategy);
}


template <typename P>
class svg_mapper
{
    typedef boost::geometry::strategy::transform::map_transformer
        <
            P,
            boost::geometry::point_xy<int>,
            true,
            true
        > transformer_type;

    boost::geometry::box<P> bbox;
    transformer_type* matrix;
    std::ostream& stream;
    int width, height;

    void init_matrix()
    {
        if (! matrix)
        {
            matrix = new transformer_type(bbox, width, height);

            stream << "<?xml version=\"1.0\" standalone=\"no\"?>" << std::endl;
            stream << "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\"" << std::endl;
            stream << "\"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">" << std::endl;

            stream << "<svg width=\"100%\" height=\"100%\" version=\"1.1\"" << std::endl;
            stream << "xmlns=\"http://www.w3.org/2000/svg\">" << std::endl;
        }
    }

public :
    svg_mapper(std::ostream& s, int w, int h)
        : matrix(NULL)
        , stream(s)
        , width(w)
        , height(h)
    {
        boost::geometry::assign_inverse(bbox);
    }

    virtual ~svg_mapper()
    {
        stream << "</svg>" << std::endl;
        if (matrix) delete matrix;
    }

    template <typename Geometry>
    void add(Geometry const& geometry)
    {
        if (boost::geometry::num_points(geometry) > 0)
        {
            boost::geometry::combine(bbox, boost::geometry::make_envelope<boost::geometry::box<P> >(geometry));
        }
    }

    template <typename Geometry>
    void map(Geometry const& geometry, std::string const& style, int size = -1)
    {
        init_matrix();
        svg_map(stream, style, size, geometry, *matrix);
    }

    template <typename Point>
    void text(Point const& point, std::string const& s, std::string const& style,
            int offset_x = 0, int offset_y = 0)
    {
        init_matrix();
        boost::geometry::point_xy<int> p;
        boost::geometry::transform(point, p, *matrix);
        stream << "<text x=\"" << boost::geometry::get<0>(p) + offset_x
            << "\" y=\"" << boost::geometry::get<1>(p) + offset_y
            << "\" style=\"" << style << "\">"
            << s << "</text>";

    }

};

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_IO_SVG_MAPPER_HPP
