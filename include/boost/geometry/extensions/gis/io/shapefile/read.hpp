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

#include <boost/geometry/algorithms/detail/calculate_point_order.hpp>
#include <boost/geometry/algorithms/detail/within/point_in_geometry.hpp>
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

// TEMP - only here for convenience, for now
#include <boost/geometry/strategies/cartesian/io.hpp>
#include <boost/geometry/strategies/geographic/io.hpp>
#include <boost/geometry/strategies/spherical/io.hpp>


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

struct read_point_policy
{
    template <typename IStream, typename Points, typename Strategy>
    static inline void apply(IStream & is, Points & points, Strategy const&)
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
};

struct read_multipoint_policy
{
    template <typename IStream, typename Points, typename Strategy>
    static inline void apply(IStream & is, Points & points, Strategy const&)
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
};

struct read_polyline_policy
{
    template <typename IStream, typename Linestrings, typename Strategy>
    static inline void apply(IStream &is, Linestrings & linestrings, Strategy const&)
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

            range::push_back(linestrings, ls_type());
            ls_type & ls = range::back(linestrings);

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
};

struct read_polygon_policy
{
    template <typename IStream, typename Polygons, typename Strategy>
    static inline void apply(IStream &is, Polygons & polygons, Strategy const& strategy)
    {
        typedef typename boost::range_value<Polygons>::type poly_type;
        typedef typename geometry::point_type<poly_type>::type pt_type;
        typedef typename geometry::ring_type<poly_type>::type ring_type;

        static const bool is_ccw = geometry::point_order<poly_type>::value == geometry::counterclockwise;
        static const bool is_open = geometry::closure<poly_type>::value == geometry::open;

        typename Strategy::point_order_strategy_type
            order_strategy = strategy.get_point_order_strategy();
        typename Strategy::template point_in_geometry_strategy<ring_type, ring_type>::type
            within_strategy = strategy.template get_point_in_geometry_strategy<ring_type, ring_type>();

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

        std::vector<ring_type> outer_rings;
        std::vector<ring_type> inner_rings;

        for (boost::int32_t i = 0; i < num_parts; ++i)
        {
            boost::int32_t f = parts[i];
            boost::int32_t l = (i + 1) < num_parts ? parts[i + 1] : num_points;

            if (f >= num_points || l > num_points || f > l)
            {
                BOOST_THROW_EXCEPTION(read_shapefile_exception("Invalid part number"));
            }

            ring_type ring;

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

            // TODO: support rval references in range::push_back()
            //       and/or implement range::emplace_back()
            //       implement and call move_or_copy(ring)

            // assume outer ring
            if (num_parts == 1)
                range::push_back(outer_rings, ring); // order could be checked here too
            // check order
            else if ( is_outer_ring(ring, order_strategy) )
                range::push_back(outer_rings, ring);
            else
                range::push_back(inner_rings, ring);
        }

        if (inner_rings.empty()) // no inner rings
        {
            for (size_t i = 0; i < outer_rings.size(); ++i)
            {
                poly_type poly;
                geometry::exterior_ring(poly) = outer_rings[i]; // TODO: move
                range::push_back(polygons, poly); // TODO: move
            }
        }
        else if (! outer_rings.empty()) // outer and inner rings
        {
            std::vector<bool> assigned(inner_rings.size(), false);
            std::size_t assigned_count = 0;
            for (size_t i = 0; i < outer_rings.size(); ++i)
            {
                poly_type poly;
                geometry::exterior_ring(poly) = outer_rings[i]; // TODO: move
                for (size_t j = 0; j < inner_rings.size(); ++j)
                {
                    if (! assigned[j])
                    {
                        if (is_inner_ring(inner_rings[j],
                                          geometry::exterior_ring(poly),
                                          within_strategy))
                        {
                            range::push_back(geometry::interior_rings(poly), inner_rings[j]); // TODO: move
                            ++assigned_count;
                            assigned[j] = true;
                        }
                        // just in case, ignore empty
                        else if (boost::empty(inner_rings[j]))
                        {
                            ++assigned_count;
                            assigned[j] = true;
                        }
                    }
                }
                range::push_back(polygons, poly); // TODO: move
            }

            // check if all interior rings were assigned
            if (assigned_count != inner_rings.size())
            {
                BOOST_THROW_EXCEPTION(read_shapefile_exception("Not all interior rings were assigned to polygons."));
            }
        }
        else // inner rings but no outer rings
        {
            // unexpected, file corrupted, bug or numerical error
            BOOST_THROW_EXCEPTION(read_shapefile_exception("Exterior ring expected"));
        }
        
        if (!is.good())
        {
            BOOST_THROW_EXCEPTION(read_shapefile_exception("Read error"));
        }
    }

    template <typename Ring, typename Strategy>
    static inline bool is_outer_ring(Ring const& ring, Strategy const& order_strategy)
    {
        geometry::order_selector result = detail::calculate_point_order(ring, order_strategy);
        return result == geometry::point_order<Ring>::value
            || result == geometry::order_undetermined;
    }

    template <typename InnerRing, typename OuterRing, typename Strategy>
    static inline bool is_inner_ring(InnerRing const& inner_ring,
                                     OuterRing const& outer_ring,
                                     Strategy const& within_strategy)
    {
        // NOTE: The worst case complexity is O(N^2) and best O(N)
        //       R-tree or partition could be used (~O(NlogN))
        //       but in most cases this version should be faster.
        typedef typename boost::range_iterator<InnerRing const>::type iter_type;
        for (iter_type it = boost::begin(inner_ring); it != boost::end(inner_ring); ++it)
        {
            if (detail::within::within_point_geometry(*it, outer_ring, within_strategy))
            {
                // at least one point of potentially interior ring found in the interior of exterior ring
                return true;
            }
        }
        return false;
    }
};

template <typename Policy, typename IStream, typename Range, typename Strategy>
inline void add_records(IStream & is, Range & rng, Strategy const& strategy)
{
    while (read_record_header(is))
    {
        Policy::apply(is, rng, strategy);
    }
}

template <typename Policy, typename IStream, typename Range, typename Strategy>
inline void add_records_as_new_element(IStream & is, Range & rng, Strategy const& strategy)
{
    typedef typename boost::range_value<Range>::type val_type;

    if (! read_record_header(is))
    {
        return;
    }

    range::push_back(rng, val_type());
    val_type & elem = range::back(rng);

    do
    {
        Policy::apply(is, elem, strategy);
    }
    while (read_record_header(is));
}

template <typename Policy, typename IStream, typename Range, typename Strategy>
inline void add_records_as_new_elements(IStream & is, Range & rng, Strategy const& strategy)
{
    typedef typename boost::range_value<Range>::type val_type;

    while (read_record_header(is))
    {
        range::push_back(rng, val_type());
        val_type & elem = range::back(rng);

        Policy::apply(is, elem, strategy);
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
    template <typename IStream, typename Points, typename Strategy>
    static inline void apply(IStream &is, Points & points, Strategy const& strategy)
    {
        namespace shp = detail::shapefile;
        
        boost::int32_t const type = shp::reset_and_read_header(is);

        if (type == shp::shape_type::point)
        {
            shp::add_records<shp::read_point_policy>(is, points, strategy);
        }
        else if (type == detail::shapefile::shape_type::multipoint)
        {
            shp::add_records<shp::read_multipoint_policy>(is, points, strategy);
        }
    }
};

template <typename Geometry>
struct read_shapefile<Geometry, multi_point_tag>
{
    template <typename IStream, typename MultiPoints, typename Strategy>
    static inline void apply(IStream &is, MultiPoints & multi_points, Strategy const& strategy)
    {
        namespace shp = detail::shapefile;
        
        boost::int32_t const type = shp::reset_and_read_header(is);

        if (type == shp::shape_type::point)
        {
            shp::add_records_as_new_element<shp::read_point_policy>(is, multi_points, strategy);
        }
        else if (type == detail::shapefile::shape_type::multipoint)
        {
            shp::add_records_as_new_elements<shp::read_multipoint_policy>(is, multi_points, strategy);
        }
    }
};

template <typename Geometry>
struct read_shapefile<Geometry, linestring_tag>
{
    template <typename IStream, typename Linestrings, typename Strategy>
    static inline void apply(IStream &is, Linestrings & linestrings, Strategy const& strategy)
    {
        namespace shp = detail::shapefile;

        boost::int32_t const type = shp::reset_and_read_header(is);

        if (type == shp::shape_type::polyline)
        {
            shp::add_records<shp::read_polyline_policy>(is, linestrings, strategy);
        }
    }
};

template <typename Geometry>
struct read_shapefile<Geometry, multi_linestring_tag>
{
    template <typename IStream, typename MultiLinestrings, typename Strategy>
    static inline void apply(IStream &is, MultiLinestrings & multi_linestrings, Strategy const& strategy)
    {
        namespace shp = detail::shapefile;

        boost::int32_t const type = shp::reset_and_read_header(is);

        if (type == shp::shape_type::polyline)
        {
            shp::add_records_as_new_elements<shp::read_polyline_policy>(is, multi_linestrings, strategy);
        }
    }
};

template <typename Geometry>
struct read_shapefile<Geometry, polygon_tag>
{
    template <typename IStream, typename Polygons, typename Strategy>
    static inline void apply(IStream &is, Polygons & polygons, Strategy const& strategy)
    {
        namespace shp = detail::shapefile;

        boost::int32_t const type = shp::reset_and_read_header(is);

        if (type == shp::shape_type::polygon)
        {
            shp::add_records<shp::read_polygon_policy>(is, polygons, strategy);
        }
    }
};

template <typename Geometry>
struct read_shapefile<Geometry, multi_polygon_tag>
{
    template <typename IStream, typename MultiPolygons, typename Strategy>
    static inline void apply(IStream &is, MultiPolygons & multi_polygons, Strategy const& strategy)
    {
        namespace shp = detail::shapefile;

        boost::int32_t const type = shp::reset_and_read_header(is);

        if (type == shp::shape_type::polygon)
        {
            shp::add_records_as_new_elements<shp::read_polygon_policy>(is, multi_polygons, strategy);
        }
    }
};


} // namespace dispatch


// Note: if an exception is thrown the output range may contain partial data
template <typename IStream, typename RangeOfGeometries, typename Strategy>
inline void read_shapefile(IStream &is, RangeOfGeometries & range_of_geometries,
                           Strategy const& strategy)
{
    typedef typename boost::range_value<RangeOfGeometries>::type geometry_type;

    geometry::concepts::check<geometry_type>();

    dispatch::read_shapefile<geometry_type>::apply(is, range_of_geometries,
                                                   strategy);
}

template <typename IStream, typename RangeOfGeometries>
inline void read_shapefile(IStream &is, RangeOfGeometries & range_of_geometries)
{
    typedef typename boost::range_value<RangeOfGeometries>::type geometry_type;
    typedef typename strategy::io::services::default_strategy
        <
            typename cs_tag<geometry_type>::type
        >::type strategy_type;

    geometry::concepts::check<geometry_type>();

    dispatch::read_shapefile<geometry_type>::apply(is, range_of_geometries,
                                                   strategy_type());
}


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_EXTENSIONS_GIS_IO_SHAPEFILE_READ_HPP
