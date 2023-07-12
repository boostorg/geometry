// Boost.Geometry

// Copyright (c) 2023 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_TEST_GEOJSON_WRITER_HPP
#define BOOST_GEOMETRY_TEST_GEOJSON_WRITER_HPP

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/io/dsv/write.hpp>

#include <ostream>
#include <string>

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template <typename GeometryTag>
struct geojson_feature_type
{
    BOOST_GEOMETRY_STATIC_ASSERT_FALSE("Not or not yet implemented for this Geometry type.",
                                       GeometryTag);
};

template <>
struct geojson_feature_type<point_tag> { static inline const char* apply() { return "Point"; } };

template <>
struct geojson_feature_type<segment_tag>  { static inline const char* apply() { return "LineString"; } };

template <>
struct geojson_feature_type<ring_tag>  { static inline const char* apply() { return "Polygon"; } };

template <>
struct geojson_feature_type<linestring_tag>  { static inline const char* apply() { return "LineString"; } };

template <>
struct geojson_feature_type<polygon_tag>  { static inline const char* apply() { return "Polygon"; } };

template <>
struct geojson_feature_type<multi_polygon_tag>  { static inline const char* apply() { return "MultiPolygon"; } };

} // namespace dispatch
#endif

/*!
\brief Helper class to create geojson output
*/
struct geojson_writer
{

  private:
    std::ostream& m_out;

    static const char* comma_space() { return ", "; }
    static const char* colon() { return ": "; }
    static const char* gtag() { return "\"geometry\""; }
    static const char* ttag() { return "\"type\""; }
    static const char* ctag() { return "\"coordinates\""; }
    static char comma() { return ','; }
    static char quote() { return '"'; }
    static char cbopen() { return '{'; }
    static char cbclose() { return '}'; }

    std::size_t feature_count = 0;
    std::size_t property_count = 0;

    void start_feature()
    {
        end_properties();
        end_feature();

        if (feature_count > 0)
        {
            m_out << comma() << std::endl;
        }
        feature_count++;

        m_out << cbopen() << std::endl << ttag() << colon() << quote() << "Feature" << quote();
    }

    void start_property()
    {
        // Write a comma, either after the "geometry" tag, or after the previous property
        m_out << comma() << std::endl;

        if (property_count == 0)
        {
            // No properties yet, start the list of properties
            m_out << quote() << "properties" << quote() << colon() << cbopen() << std::endl;
        }
        property_count++;
    }

    void end_properties()
    {
        if (property_count > 0)
        {
            m_out << cbclose() << std::endl;
            property_count = 0;
        }
    }
    void end_feature()
    {
        if (feature_count > 0)
        {
            m_out << cbclose() << std::endl;
        }
    }

  public:
    geojson_writer(std::ostream& out) : m_out(out)
    {
        m_out << cbopen() << ttag() << colon() << quote() << "FeatureCollection" << quote()
              << comma_space() << quote() << "features" << quote() << colon() << "[" << std::endl;
    }
    ~geojson_writer()
    {
        end_properties();
        end_feature();

        m_out << "]" << cbclose();
    }

    // Set a property. It is expected that a feature is already started.
    template <typename T>
    void add_property(const std::string& name, T const& value)
    {
        start_property();
        m_out << quote() << name << quote() << colon() << value;
    }

    // Overload to get it quoted
    void add_property(const std::string& name, std::string const& value)
    {
        start_property();
        m_out << quote() << name << quote() << colon() << quote() << value << quote();
    }

    // Overload to get it quoted
    void add_property(const std::string& name, const char* value)
    {
        start_property();
        m_out << quote() << name << quote() << colon() << quote() << value << quote();
    }

    // The method "feature" should be called to start a feature.
    // After that, properties can be added.
    template <typename Geometry>
    void feature(const Geometry& geometry)
    {
        start_feature();

        // Write the comma after either the "Feature" tag
        m_out << comma() << std::endl;

        // Write the geometry
        using tag_t = typename tag<Geometry>::type;
        m_out << gtag() << colon() << cbopen() << ttag() << colon() << quote()
              << dispatch::geojson_feature_type<tag_t>::apply() << quote() << comma_space()
              << ctag() << colon();

        // A ring is modelled as a geojson polygon, and therefore the opening and closing
        // list marks should be duplicated to indicate empty interior rings.
        const bool dup = std::is_same<tag_t, ring_tag>::value;
        const char* list_open = dup ? "[[" : "[";
        const char* list_close = dup ? "]]" : "]";
        m_out << geometry::dsv(geometry, comma_space(), "[", "]", comma_space(), list_open,
                               list_close) << cbclose() << std::endl;
    }

};

}} // namespace boost::geometry


#endif
