// Boost.Geometry

// Copyright (c) 2024 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_GIS_IO_GEOJSON_WRITER_HPP
#define BOOST_GEOMETRY_EXTENSIONS_GIS_IO_GEOJSON_WRITER_HPP

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/io/dsv/write.hpp>
#include <boost/geometry/core/static_assert.hpp>
#include <boost/geometry/util/constexpr.hpp>

#include <iomanip>
#include <ostream>
#include <string>
#include <type_traits>


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

    std::size_t feature_count = 0;
    std::size_t property_count = 0;

    template <typename T>
    void stream_quoted(T const& entry)
    {
        m_out << '"' << entry << '"';
    }

    void start_feature()
    {
        end_properties();
        end_feature();

        m_out << (feature_count > 0 ? ",\n" : "") <<  R"({"type": "Feature")";
        feature_count++;
    }

    void start_property()
    {
        // Write a comma, either after the "geometry" tag, or after the previous property
        // If there are no properties yet, start the list of properties
        m_out << "," << (property_count == 0 ? R"("properties": {)" : "") << '\n';
        property_count++;
    }

    void end_properties()
    {
        if (property_count > 0)
        {
            m_out << "}\n";
            property_count = 0;
        }
    }
    void end_feature()
    {
        if (feature_count > 0)
        {
            m_out << "}\n";
        }
    }

  public:
    explicit geojson_writer(std::ostream& out) : m_out(out)
    {
        m_out << R"({"type": "FeatureCollection", "features": [)" << '\n';
    }

    ~geojson_writer()
    {
        end_properties();
        end_feature();

        m_out << "]}";
    }

    // Set a property. It is expected that a feature is already started.
    template <typename T>
    void add_property(std::string const& name, T const& value)
    {
        constexpr bool needs_quotes 
            = std::is_same<T, std::string>::value
              || std::is_same<typename std::remove_extent<T>::type, char>::value;

        start_property();
        stream_quoted(name);
        m_out << ": ";
        if BOOST_GEOMETRY_CONSTEXPR(needs_quotes)
        {
            stream_quoted(value);
        }
        else
        {
            m_out << std::boolalpha << value;
        }
    }

    // The method "feature" should be called to start a feature.
    // After that, properties can be added, until a new "feature" is called,
    // or the instance is destructed
    template <typename Geometry>
    void feature(Geometry const& geometry)
    {
        using tag_t = typename tag<Geometry>::type;
        
        start_feature();

        // Write the comma after either the "Feature" tag
        m_out << ",\n";

        // Write the geometry
        m_out << R"("geometry": {"type":)";
        stream_quoted(dispatch::geojson_feature_type<tag_t>::apply());
        m_out << R"(, "coordinates": )";

        // A ring is modelled as a geojson polygon, and therefore the opening and closing
        // list marks should be duplicated to indicate empty interior rings.
        constexpr bool dup = std::is_same<tag_t, ring_tag>::value;
        const char* list_open = dup ? "[[" : "[";
        const char* list_close = dup ? "]]" : "]";

        // Indicate that dsv should close any ring automatically if its model is open
        constexpr bool close = geometry::closure<Geometry>::value == geometry::open;

        m_out << geometry::dsv(geometry, ", ", "[", "]", ", ", list_open,
                               list_close, ",", close) << "}\n";
    }

};

}} // namespace boost::geometry

#endif
