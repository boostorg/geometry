// Boost.Geometry

// Copyright (c) 2019, Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_EXTENSIONS_GIS_IO_SHAPEFILE_READ_HPP
#define BOOST_GEOMETRY_EXTENSIONS_GIS_IO_SHAPEFILE_READ_HPP

#include <algorithm>
#include <vector>

#include <boost/cstdint.hpp>
#include <boost/endian/conversion.hpp>
#include <boost/throw_exception.hpp>

#include <boost/geometry/core/exception.hpp>
#include <boost/geometry/core/exterior_ring.hpp>
#include <boost/geometry/core/interior_rings.hpp>
#include <boost/geometry/core/point_order.hpp>
#include <boost/geometry/core/point_type.hpp>
#include <boost/geometry/core/closure.hpp>
#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tags.hpp>
#include <boost/geometry/geometries/concepts/check.hpp>
#include <boost/geometry/util/range.hpp>

namespace boost { namespace geometry
{

class read_shapefile_exception : public geometry::exception
{
public:
    explicit read_shapefile_exception(const char * msg)
        : m_msg(msg)
    {}

    explicit read_shapefile_exception(std::string const& msg)
        : m_msg(msg)
    {}

    virtual char const* what() const throw()
    {
        //return "Shapefile read error";
        return m_msg.what();
    }

private:
    std::runtime_error m_msg;
};

namespace detail { namespace shapefile
{

template <typename IStream, typename T>
inline void read_native(IStream & is, T & v)
{
    is.read(reinterpret_cast<char*>(&v), sizeof(T));    
}

template <typename IStream, typename T>
inline void read_big(IStream & is, T & v)
{
    is.read(reinterpret_cast<char*>(&v), sizeof(T));    
    v = boost::endian::big_to_native(v);
}

template <typename IStream, typename T>
inline void read_little(IStream & is, T & v)
{
    is.read(reinterpret_cast<char*>(&v), sizeof(T));
    v = boost::endian::little_to_native(v);
}

template <typename IStream>
inline boost::int32_t reset_and_read_header(IStream & is)
{
    is.clear();
    is.seekg(0);

    boost::int32_t code = 0;
    read_big(is, code);

    if (code != 9994)
    {
        BOOST_THROW_EXCEPTION(read_shapefile_exception("Invalid header code"));
    }

    // 5 unused, length, version
    is.seekg(7 * sizeof(boost::int32_t), IStream::cur);
    
    boost::int32_t type = 0;
    read_little(is, type);

    // TODO: support filtering
    /*
    double min_x, min_y, max_x, max_y;
    double min_z, max_z;
    double min_m, max_m;
    read_little(is, min_x);
    read_little(is, min_y);
    read_little(is, max_x);
    read_little(is, max_y);
    read_little(is, min_z);
    read_little(is, max_z);
    read_little(is, min_m);
    read_little(is, max_m);
    */
    is.seekg(8 * sizeof(double), IStream::cur);

    if (!is.good())
    {
        BOOST_THROW_EXCEPTION(read_shapefile_exception("Unreadable header"));
    }

    return type;
}

template <typename IStream>
inline bool read_record_header(IStream & is)
{
    //read_big(is, number);
    //read_big(is, length);

    is.seekg(sizeof(boost::int32_t), IStream::cur);   
    // only to set flags
    boost::int32_t foo;
    read_native(is, foo);

    return is.good();
}

struct shape_type
{
    enum enum_t
    {
        null_shape = 0,
        point = 1,
        polyline = 3,
        polygon = 5,
        multipoint = 8
        // TODO: support the rest
    };
};

template <typename Range>
inline typename boost::range_reference<Range>::type
push_back(Range & rng, typename boost::range_value<Range>::type const& v)
{
    range::push_back(rng, v);
    return range::back(rng);
}

template <typename IStream, typename Range>
inline void read_and_push_back_point(IStream & is, Range & rng)
{
    typedef typename boost::range_value<Range>::type pt_type;

    double x, y;
    read_little(is, x);
    read_little(is, y);

    pt_type pt;
    geometry::set<0>(pt, x);
    geometry::set<1>(pt, y);

    range::push_back(rng, pt);
}

template <typename IStream, typename Range>
inline void read_and_set_point_at(IStream & is, Range & rng, std::size_t index)
{
    typedef typename boost::range_value<Range>::type pt_type;

    double x, y;
    read_little(is, x);
    read_little(is, y);

    pt_type & pt = range::at(rng, index);
    geometry::set<0>(pt, x);
    geometry::set<1>(pt, y);
}

template <typename IStream>
inline void read_parts(IStream & is,
                       std::vector<boost::int32_t> & parts,
                       boost::int32_t num_parts)
{
    parts.resize(num_parts);
    for (boost::int32_t i = 0 ; i < num_parts ; ++i)
    {
        read_little(is, parts[i]);
    }
}

// Range of Points
template <typename IStream, typename Points>
inline void read_point(IStream & is, Points & points)
{
    boost::int32_t type;
    read_little(is, type);
    if (type != shape_type::point)
    {
        BOOST_THROW_EXCEPTION(read_shapefile_exception("Point expected"));
    }

    read_and_push_back_point(is, points);

    if (!is.good())
    {
        BOOST_THROW_EXCEPTION(read_shapefile_exception("Read error"));
    }
}

// Range of Points
template <typename IStream, typename Points>
inline void read_multipoint(IStream & is, Points & points)
{
    typedef typename boost::range_value<Points>::type pt_type;

    boost::int32_t type;
    read_little(is, type);
    if (type != shape_type::multipoint)
    {
        BOOST_THROW_EXCEPTION(read_shapefile_exception("Multipoint expected"));
    }

    is.seekg(4 * sizeof(double), IStream::cur);

    boost::int32_t num_points;
    read_little(is, num_points);

    if (num_points < 0)
    {
        BOOST_THROW_EXCEPTION(read_shapefile_exception("Points number lesser than 0"));
    }

    std::size_t old_size = boost::size(points);
    range::resize(points, old_size + num_points);
    for (boost::int32_t i = 0; i < num_points; ++i)
    {
        read_and_set_point_at(is, points, old_size + i);
    }

    if (!is.good())
    {
        BOOST_THROW_EXCEPTION(read_shapefile_exception("Read error"));
    }
}

// Range of Linestrings
template <typename IStream, typename Linestrings>
inline void read_polyline(IStream &is, Linestrings & linestrings)
{
    typedef typename boost::range_value<Linestrings>::type ls_type;
    typedef typename boost::range_value<ls_type>::type pt_type;

    boost::int32_t type;
    //double min_x, min_y, max_x, max_y;
    boost::int32_t num_parts;
    boost::int32_t num_points;
    std::vector<boost::int32_t> parts;

    read_little(is, type);
    if (type != shape_type::polyline)
    {
        BOOST_THROW_EXCEPTION(read_shapefile_exception("Polyline expected"));
    }

    // TODO: support filtering
    //read_little(is, min_x);
    //read_little(is, min_y);
    //read_little(is, max_x);
    //read_little(is, max_y);
    is.seekg(4 * sizeof(double), IStream::cur);
    read_little(is, num_parts);
    read_little(is, num_points);

    if (num_parts < 0 || num_points < 0)
    {
        BOOST_THROW_EXCEPTION(read_shapefile_exception("Parts or points number lesser than 0"));
    }

    read_parts(is, parts, num_parts);
    
    for (boost::int32_t i = 0; i < num_parts; ++i)
    {
        boost::int32_t f = parts[i];
        boost::int32_t l = (i + 1) < num_parts ? parts[i + 1] : num_points;

        if (f >= num_points || l > num_points || f > l)
        {
            BOOST_THROW_EXCEPTION(read_shapefile_exception("Invalid part number"));
        }

        ls_type & ls = push_back(linestrings, ls_type());

        std::size_t ls_size = l - f;
        
        range::resize(ls, ls_size);
        
        for (std::size_t j = 0; j < ls_size; ++j)
        {
            read_and_set_point_at(is, ls, j);
        }
    }

    if (!is.good())
    {
        BOOST_THROW_EXCEPTION(read_shapefile_exception("Read error"));
    }
}

// Range of Polygons
template <typename IStream, typename Polygons>
inline void read_polygon(IStream &is, Polygons & polygons)
{
    typedef typename boost::range_value<Polygons>::type poly_type;
    typedef typename geometry::point_type<poly_type>::type pt_type;
    typedef typename geometry::ring_type<poly_type>::type ring_type;

    static const bool is_ccw = geometry::point_order<poly_type>::value == geometry::counterclockwise;
    static const bool is_open = geometry::closure<poly_type>::value == geometry::open;

    boost::int32_t type;
    //double min_x, min_y, max_x, max_y;
    boost::int32_t num_parts;
    boost::int32_t num_points;
    std::vector<boost::int32_t> parts;

    read_little(is, type);
    if (type != shape_type::polygon)
    {
        BOOST_THROW_EXCEPTION(read_shapefile_exception("Polygon expected"));
    }

    // TODO: support filtering
    //read_little(is, min_x);
    //read_little(is, min_y);
    //read_little(is, max_x);
    //read_little(is, max_y);
    is.seekg(4 * sizeof(double), IStream::cur);
    read_little(is, num_parts);
    read_little(is, num_points);

    if (num_parts < 0 || num_points < 0)
    {
        BOOST_THROW_EXCEPTION(read_shapefile_exception("Parts or points number lesser than 0"));
    }

    read_parts(is, parts, num_parts);

    poly_type & poly = push_back(polygons, poly_type());

    for (boost::int32_t i = 0; i < num_parts; ++i)
    {
        boost::int32_t f = parts[i];
        boost::int32_t l = (i + 1) < num_parts ? parts[i + 1] : num_points;

        if (f >= num_points || l > num_points || f > l)
        {
            BOOST_THROW_EXCEPTION(read_shapefile_exception("Invalid part number"));
        }

        ring_type & ring = (i == 0)
                         ? geometry::exterior_ring(poly)
                         : push_back(geometry::interior_rings(poly), ring_type());

        std::size_t ring_size = l - f - (is_open ? 1 : 0);
        
        range::resize(ring, ring_size);

        for (std::size_t j = 0; j < ring_size; ++j)
        {
            read_and_set_point_at(is, ring, j);
        }

        // if ring is open ignore the last point
        if (is_open)
        {
            is.seekg(2 * sizeof(double), IStream::cur);
        }

        // if ring is ccw reverse leaving the first point untouched
        if (is_ccw)
        {
            typename boost::range_iterator<ring_type>::type
                b = boost::begin(ring),
                e = boost::end(ring);
            std::reverse(++b, is_open ? e : (--e));
        }
    }

    if (!is.good())
    {
        BOOST_THROW_EXCEPTION(read_shapefile_exception("Read error"));
    }
}

}} // namespace detail::shapefile

namespace dispatch
{

template
<
    typename Geometry,
    typename Tag = typename geometry::tag<Geometry>::type
>
struct read_shapefile
    : not_implemented<Tag>
{};

template <typename Geometry>
struct read_shapefile<Geometry, point_tag>
{
    template <typename IStream, typename Points>
    static inline void apply(IStream &is, Points & points)
    {
        boost::int32_t const type = detail::shapefile::reset_and_read_header(is);

        if (type == detail::shapefile::shape_type::point)
        {
            for (;;)
            {
                if (! detail::shapefile::read_record_header(is))
                    break;

                detail::shapefile::read_point(is, points);
            }
        }
        else if (type == detail::shapefile::shape_type::multipoint)
        {
            for (;;)
            {
                if (! detail::shapefile::read_record_header(is))
                    break;

                detail::shapefile::read_multipoint(is, points);
            }
        }
    }
};

template <typename Geometry>
struct read_shapefile<Geometry, multi_point_tag>
{
    template <typename IStream, typename MultiPoints>
    static inline void apply(IStream &is, MultiPoints & multi_points)
    {
        typedef typename boost::range_value<MultiPoints>::type mpt_type;

        boost::int32_t const type = detail::shapefile::reset_and_read_header(is);

        if (type == detail::shapefile::shape_type::point)
        {
            mpt_type & mpt = detail::shapefile::push_back(multi_points, mpt_type());
            
            for (;;)
            {
                if (! detail::shapefile::read_record_header(is))
                    break;

                detail::shapefile::read_point(is, mpt);
            }
        }
        else if (type == detail::shapefile::shape_type::multipoint)
        {
            for (;;)
            {
                if (! detail::shapefile::read_record_header(is))
                    break;

                mpt_type & mpt = detail::shapefile::push_back(multi_points, mpt_type());

                detail::shapefile::read_multipoint(is, mpt);
            }
        }
    }
};

template <typename Geometry>
struct read_shapefile<Geometry, linestring_tag>
{
    template <typename IStream, typename Linestrings>
    static inline void apply(IStream &is, Linestrings & linestrings)
    {
        boost::int32_t const type = detail::shapefile::reset_and_read_header(is);

        if (type == detail::shapefile::shape_type::polyline)
        {
            for (;;)
            {
                if (! detail::shapefile::read_record_header(is))
                    break;

                detail::shapefile::read_polyline(is, linestrings);
            }
        }
    }
};

template <typename Geometry>
struct read_shapefile<Geometry, multi_linestring_tag>
{
    template <typename IStream, typename MultiLinestrings>
    static inline void apply(IStream &is, MultiLinestrings & multi_linestrings)
    {
        typedef typename boost::range_value<MultiLinestrings>::type mls_type;

        boost::int32_t const type = detail::shapefile::reset_and_read_header(is);

        if (type == detail::shapefile::shape_type::polyline)
        {
            for (;;)
            {
                if (! detail::shapefile::read_record_header(is))
                    break;

                mls_type & mls = detail::shapefile::push_back(multi_linestrings, mls_type());

                detail::shapefile::read_polyline(is, mls);
            }
        }
    }
};

template <typename Geometry>
struct read_shapefile<Geometry, polygon_tag>
{
    template <typename IStream, typename Polygons>
    static inline void apply(IStream &is, Polygons & polygons)
    {
        boost::int32_t const type = detail::shapefile::reset_and_read_header(is);

        if (type == detail::shapefile::shape_type::polygon)
        {
            for (;;)
            {
                if (! detail::shapefile::read_record_header(is))
                    break;

                detail::shapefile::read_polygon(is, polygons);
            }
        }
    }
};

template <typename Geometry>
struct read_shapefile<Geometry, multi_polygon_tag>
{
    template <typename IStream, typename MultiPolygons>
    static inline void apply(IStream &is, MultiPolygons & multi_polygons)
    {
        typedef typename boost::range_value<MultiPolygons>::type mpoly_type;

        boost::int32_t const type = detail::shapefile::reset_and_read_header(is);

        if (type == detail::shapefile::shape_type::polygon)
        {
            for (;;)
            {
                if (! detail::shapefile::read_record_header(is))
                    break;

                mpoly_type & mpoly = detail::shapefile::push_back(multi_polygons, mpoly_type());

                detail::shapefile::read_polygon(is, mpoly);
            }
        }
    }
};


} // namespace dispatch


template <typename IStream, typename RangeOfGeometries>
inline void read_shapefile(IStream &is, RangeOfGeometries & range_of_geometries)
{
    typedef typename boost::range_value<RangeOfGeometries>::type geometry_type;
    geometry::concepts::check<geometry_type>();
    dispatch::read_shapefile<geometry_type>::apply(is, range_of_geometries);
}


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_EXTENSIONS_GIS_IO_SHAPEFILE_READ_HPP
