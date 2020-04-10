// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2020 Baidyanath Kundu, Haldia, India.

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_IO_PLOT_LY_PLOTTER_HPP
#define BOOST_GEOMETRY_EXTENSIONS_IO_PLOT_LY_PLOTTER_HPP

#include <ostream>

#include <boost/geometry/geometries/geometries.hpp>

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace json
{

template <typename Point, int Dimension>
struct json_point
{
    template <typename Char, typename Traits>
    static inline void apply(std::basic_ostream<Char, Traits>& os, 
        Point const& p, std::string const& color)
    {
        os << "{ \"mode\": \"lines\", \"type\": \"" 
           << (( Dimension > 2 ) ? "scatter3d" : "scatter" )
           << "\"" 
           << ", \"x\": [\"" << geometry::get<0>(p) << "\"]"
           << ", \"y\": [\"" << geometry::get<1>(p) << "\"]"
           << (( Dimension > 2 ) ? (", \"z\": [\"" << geometry::get<2>(p) << "\"]") : "" )
           << ", \"marker\": {\"color\": \"" << color << "\"}}";
    }
};

template <typename Segment, int Dimension>
struct json_segment
{
    template <typename Char, typename Traits>
    static inline void apply(std::basic_ostream<Char, Traits>& os,
        Segment const& segment, std::string const& color)
    {        
        os << "{ \"mode\": \"lines\", \"type\": \"" 
           << (( Dimension > 2 ) ? "scatter3d" : "scatter" )
           << "\"" 
           << ", \"x\": [\"" << geometry::get<0, 0>(segment) << "\", \"" << geometry::get<1, 0>(segment) <<"\"]"
           << ", \"y\": [\"" << geometry::get<0, 1>(segment) << "\", \"" << geometry::get<1, 1>(segment) <<"\"]";
        if( Dimension == 3 )
        {
            os << ", \"z\": [\"" << geometry::get<0, 2>(segment) << "\", \"" << geometry::get<1, 2>(segment) << "\"]";
        }
        os << ", \"line\": {\"dash\": \"solid\", \"color\": \"" << color << "\", \"width\": 2}";
    }
};

template <typename Range, int Dimension>
struct json_range
{
    template <typename Char, typename Traits>
    static inline void apply(std::basic_ostream<Char, Traits>& os,
        Range const& range, std::string const& color)
    {
        typedef typename boost::range_iterator<Range const>::type iterator;

        bool first = true;

        os << "{ \"mode\": \"lines\", \"type\": \"" 
           << (( Dimension > 2 ) ? "scatter3d" : "scatter" )
           << "\"";

        std::basic_stringstream<Char, Traits> x, y, z;

        x << ", \"x\": [";
        y << ", \"y\": [";
        z << ( Dimension > 2 ? ", \"z\": [" : "" );

        for (iterator it = boost::begin(range);
            it != boost::end(range);
            ++it, first = false)
        {
            x << (first ? "\"" : ", \"" )
              << geometry::get<0>(*it)
              << "\"";
            y << (first ? "\"" : ", \"" )
              << geometry::get<1>(*it)
              << "\"";
            if( Dimension > 2 ) 
            {
                z << (first ? "\"" : ", \"" )
                  << geometry::get<2>(*it)
                  << "\"";
            }
        }

        x << "]";
        y << "]";
        z << ( Dimension > 2 ? "]" : "" );
        
        os << x.rdbuf()
           << y.rdbuf() 
           << z.rdbuf()
           << ", \"line\": {\"dash\": \"solid\", \"color\": \"" << color << "\", \"width\": 2}";
    }
};


template <typename MultiGeometry, typename Policy>
struct json_multi
{
    template <typename Char, typename Traits>
    static inline void apply(std::basic_ostream<Char, Traits>& os,
        MultiGeometry const& multi, std::string const& color)
    {
        for (typename boost::range_iterator<MultiGeometry const>::type
                    it = boost::begin(multi);
            it != boost::end(multi);
            ++it)
        {
            Policy::apply(os, *it, color);
        }

    }

};

}} // namespace detail::json
#endif // DOXYGEN_NO_DETAIL

#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template 
<
    typename Geometry, 
    typename Tag = typename tag<Geometry>::type
>
struct json_plot
{
    BOOST_MPL_ASSERT_MSG
        (
            false, NOT_OR_NOT_YET_IMPLEMENTED_FOR_THIS_GEOMETRY_TYPE
            , (Geometry)
        );
};

template <typename Point>
struct json_plot<Point, point_tag> : detail::json::json_point<Point, dimension<Point>::value> {};

template <typename Segment>
struct json_plot<Segment, segment_tag> : detail::json::json_segment<Segment, dimension<Segment>::value> {};

template <typename Linestring>
struct json_plot<Linestring, linestring_tag>
    : detail::json::json_range<Linestring,  dimension<Linestring>::value> {};

template <typename MultiPoint>
struct json_plot<MultiPoint, multi_point_tag>
    : detail::json::json_multi
        <
            MultiPoint,
            detail::json::json_point
                <
                    typename boost::range_value<MultiPoint>::type,
                    dimension<MultiPoint>::value
                >
        >
{};

template <typename MultiLinestring>
struct json_plot<MultiLinestring, multi_linestring_tag>
    : detail::json::json_multi
        <
            MultiLinestring,
            detail::json::json_range
                <
                    typename boost::range_value<MultiLinestring>::type,
                    dimension<MultiLinestring>::value
                >

        >
{};

} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH

template <typename Geometry>
inline void json_plot(std::ostream& stream, Geometry const& geometry, std::string const& color)
{
    dispatch::json_plot<Geometry>::apply(stream, geometry, color);
}


class json_plotter : boost::noncopyable
{
    std::ostream& m_stream;
    bool first;

public:

    json_plotter(std::ostream& stream)
        : m_stream(stream)
    {
        m_stream << "{"
                << std::endl
                << "    data: [";
        first = true;
    }

    virtual ~json_plotter()
    {
        m_stream << "}], "
                << std::endl
                << "    layout: {\"scene\": {\"camera\": {\"up\": {\"x\": 0, \"y\": 0, \"z\": 1}, \"eye\": {\"x\": 0.2829440102575065, \"y\": 0.05402592432697305, \"z\": 2.14581543627592}, \"center\": {\"x\": 0, \"y\": 0, \"z\": 0}, \"projection\": {\"type\": \"perspective\"}}, \"aspectmode\": \"auto\", \"aspectratio\": {\"x\": 1, \"y\": 1, \"z\": 1}}, \"xaxis\": {\"range\": [-1, 6], \"autorange\": true}, \"yaxis\": {\"range\": [-1, 4], \"autorange\": true}, \"autosize\": true, \"template\": {\"data\": {\"bar\": [{\"type\": \"bar\", \"marker\": {\"colorbar\": {\"ticks\": \"\", \"outlinewidth\": 0}}}], \"table\": [{\"type\": \"table\", \"cells\": {\"fill\": {\"color\": \"#EBF0F8\"}, \"line\": {\"color\": \"white\"}}, \"header\": {\"fill\": {\"color\": \"#C8D4E3\"}, \"line\": {\"color\": \"white\"}}}], \"carpet\": [{\"type\": \"carpet\", \"aaxis\": {\"gridcolor\": \"#C8D4E3\", \"linecolor\": \"#C8D4E3\", \"endlinecolor\": \"#2a3f5f\", \"minorgridcolor\": \"#C8D4E3\", \"startlinecolor\": \"#2a3f5f\"}, \"baxis\": {\"gridcolor\": \"#C8D4E3\", \"linecolor\": \"#C8D4E3\", \"endlinecolor\": \"#2a3f5f\", \"minorgridcolor\": \"#C8D4E3\", \"startlinecolor\": \"#2a3f5f\"}}], \"mesh3d\": [{\"type\": \"mesh3d\", \"colorbar\": {\"ticks\": \"\", \"outlinewidth\": 0}}], \"contour\": [{\"type\": \"contour\", \"colorbar\": {\"ticks\": \"\", \"outlinewidth\": 0}, \"autocolorscale\": true}], \"heatmap\": [{\"type\": \"heatmap\", \"colorbar\": {\"ticks\": \"\", \"outlinewidth\": 0}, \"autocolorscale\": true}], \"scatter\": [{\"type\": \"scatter\", \"marker\": {\"colorbar\": {\"ticks\": \"\", \"outlinewidth\": 0}}}], \"surface\": [{\"type\": \"surface\", \"colorbar\": {\"ticks\": \"\", \"outlinewidth\": 0}}], \"heatmapgl\": [{\"type\": \"heatmapgl\", \"colorbar\": {\"ticks\": \"\", \"outlinewidth\": 0}}], \"histogram\": [{\"type\": \"histogram\", \"marker\": {\"colorbar\": {\"ticks\": \"\", \"outlinewidth\": 0}}}], \"parcoords\": [{\"line\": {\"colorbar\": {\"ticks\": \"\", \"outlinewidth\": 0}}, \"type\": \"parcoords\"}], \"scatter3d\": [{\"type\": \"scatter3d\", \"marker\": {\"colorbar\": {\"ticks\": \"\", \"outlinewidth\": 0}}}], \"scattergl\": [{\"type\": \"scattergl\", \"marker\": {\"colorbar\": {\"ticks\": \"\", \"outlinewidth\": 0}}}], \"choropleth\": [{\"type\": \"choropleth\", \"colorbar\": {\"ticks\": \"\", \"outlinewidth\": 0}}], \"scattergeo\": [{\"type\": \"scattergeo\", \"marker\": {\"colorbar\": {\"ticks\": \"\", \"outlinewidth\": 0}}}], \"histogram2d\": [{\"type\": \"histogram2d\", \"colorbar\": {\"ticks\": \"\", \"outlinewidth\": 0}, \"autocolorscale\": true}], \"scatterpolar\": [{\"type\": \"scatterpolar\", \"marker\": {\"colorbar\": {\"ticks\": \"\", \"outlinewidth\": 0}}}], \"contourcarpet\": [{\"type\": \"contourcarpet\", \"colorbar\": {\"ticks\": \"\", \"outlinewidth\": 0}}], \"scattercarpet\": [{\"type\": \"scattercarpet\", \"marker\": {\"colorbar\": {\"ticks\": \"\", \"outlinewidth\": 0}}}], \"scattermapbox\": [{\"type\": \"scattermapbox\", \"marker\": {\"colorbar\": {\"ticks\": \"\", \"outlinewidth\": 0}}}], \"scatterpolargl\": [{\"type\": \"scatterpolargl\", \"marker\": {\"colorbar\": {\"ticks\": \"\", \"outlinewidth\": 0}}}], \"scatterternary\": [{\"type\": \"scatterternary\", \"marker\": {\"colorbar\": {\"ticks\": \"\", \"outlinewidth\": 0}}}], \"histogram2dcontour\": [{\"type\": \"histogram2dcontour\", \"colorbar\": {\"ticks\": \"\", \"outlinewidth\": 0}, \"autocolorscale\": true}]}, \"layout\": {\"geo\": {\"bgcolor\": \"white\", \"showland\": true, \"lakecolor\": \"white\", \"landcolor\": \"white\", \"showlakes\": true, \"subunitcolor\": \"#C8D4E3\"}, \"font\": {\"color\": \"#2a3f5f\"}, \"polar\": {\"bgcolor\": \"white\", \"radialaxis\": {\"ticks\": \"\", \"gridcolor\": \"#EBF0F8\", \"linecolor\": \"#EBF0F8\"}, \"angularaxis\": {\"ticks\": \"\", \"gridcolor\": \"#EBF0F8\", \"linecolor\": \"#EBF0F8\"}}, \"scene\": {\"xaxis\": {\"ticks\": \"\", \"gridcolor\": \"#DFE8F3\", \"gridwidth\": 2, \"linecolor\": \"#EBF0F8\", \"zerolinecolor\": \"#EBF0F8\", \"showbackground\": true, \"backgroundcolor\": \"white\"}, \"yaxis\": {\"ticks\": \"\", \"gridcolor\": \"#DFE8F3\", \"gridwidth\": 2, \"linecolor\": \"#EBF0F8\", \"zerolinecolor\": \"#EBF0F8\", \"showbackground\": true, \"backgroundcolor\": \"white\"}, \"zaxis\": {\"ticks\": \"\", \"gridcolor\": \"#DFE8F3\", \"gridwidth\": 2, \"linecolor\": \"#EBF0F8\", \"zerolinecolor\": \"#EBF0F8\", \"showbackground\": true, \"backgroundcolor\": \"white\"}}, \"title\": {\"x\": 0.05}, \"xaxis\": {\"ticks\": \"\", \"gridcolor\": \"#EBF0F8\", \"linecolor\": \"#EBF0F8\", \"automargin\": true, \"zerolinecolor\": \"#EBF0F8\", \"zerolinewidth\": 2}, \"yaxis\": {\"ticks\": \"\", \"gridcolor\": \"#EBF0F8\", \"linecolor\": \"#EBF0F8\", \"automargin\": true, \"zerolinecolor\": \"#EBF0F8\", \"zerolinewidth\": 2}, \"ternary\": {\"aaxis\": {\"ticks\": \"\", \"gridcolor\": \"#DFE8F3\", \"linecolor\": \"#A2B1C6\"}, \"baxis\": {\"ticks\": \"\", \"gridcolor\": \"#DFE8F3\", \"linecolor\": \"#A2B1C6\"}, \"caxis\": {\"ticks\": \"\", \"gridcolor\": \"#DFE8F3\", \"linecolor\": \"#A2B1C6\"}, \"bgcolor\": \"white\"}, \"colorway\": [\"#636efa\", \"#EF553B\", \"#00cc96\", \"#ab63fa\", \"#19d3f3\", \"#e763fa\", \"#fecb52\", \"#ffa15a\", \"#ff6692\", \"#b6e880\"], \"hovermode\": \"closest\", \"colorscale\": {\"diverging\": [[0, \"#8e0152\"], [0.1, \"#c51b7d\"], [0.2, \"#de77ae\"], [0.3, \"#f1b6da\"], [0.4, \"#fde0ef\"], [0.5, \"#f7f7f7\"], [0.6, \"#e6f5d0\"], [0.7, \"#b8e186\"], [0.8, \"#7fbc41\"], [0.9, \"#4d9221\"], [1, \"#276419\"]], \"sequential\": [[0, \"#0508b8\"], [0.0893854748603352, \"#1910d8\"], [0.1787709497206704, \"#3c19f0\"], [0.2681564245810056, \"#6b1cfb\"], [0.3575418994413408, \"#981cfd\"], [0.44692737430167595, \"#bf1cfd\"], [0.5363128491620112, \"#dd2bfd\"], [0.6256983240223464, \"#f246fe\"], [0.7150837988826816, \"#fc67fd\"], [0.8044692737430168, \"#fe88fc\"], [0.8938547486033519, \"#fea5fd\"], [0.9832402234636871, \"#febefe\"], [1, \"#fec3fe\"]], \"sequentialminus\": [[0, \"#0508b8\"], [0.0893854748603352, \"#1910d8\"], [0.1787709497206704, \"#3c19f0\"], [0.2681564245810056, \"#6b1cfb\"], [0.3575418994413408, \"#981cfd\"], [0.44692737430167595, \"#bf1cfd\"], [0.5363128491620112, \"#dd2bfd\"], [0.6256983240223464, \"#f246fe\"], [0.7150837988826816, \"#fc67fd\"], [0.8044692737430168, \"#fe88fc\"], [0.8938547486033519, \"#fea5fd\"], [0.9832402234636871, \"#febefe\"], [1, \"#fec3fe\"]]}, \"plot_bgcolor\": \"white\", \"paper_bgcolor\": \"white\", \"shapedefaults\": {\"line\": {\"width\": 0}, \"opacity\": 0.4, \"fillcolor\": \"#506784\"}, \"annotationdefaults\": {\"arrowhead\": 0, \"arrowcolor\": \"#506784\", \"arrowwidth\": 1}}, \"themeRef\": \"PLOTLY_WHITE\"}},"
                << std::endl
                << "    frames: [], "
                << std::endl
                << "    config: {\"showLink\": true, \"linkText\": \"Export to plotly.com\", \"mapboxAccessToken\": \"pk.eyJ1IjoiY2hyaWRkeXAiLCJhIjoiY2lxMnVvdm5iMDA4dnhsbTQ5aHJzcGs0MyJ9.X9o_rzNLNesDxdra4neC_A\"}"
                << std::endl
                << "}";
    }

    template <typename Geometry>
    void plot(Geometry const& geometry, std::string const& color)
    {
        m_stream << ( first ? "" : ", " );
        first = false;
        json_plot(m_stream, geometry, color);
    }
};

}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_EXTENSIONS_IO_PLOT_LY_PLOTTER_HPP
