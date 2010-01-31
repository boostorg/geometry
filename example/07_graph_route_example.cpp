// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// Example showing GGL combined with Boost.Graph, calculating shortest routes
// input: two WKT's, provided in subfolder data
// output: text, + an SVG, displayable in e.g. Firefox)

#include <iostream>
#include <fstream>
#include <iomanip>
#include <limits>

#include <boost/tuple/tuple.hpp>
#include <boost/foreach.hpp>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>

#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/cartesian2d.hpp>


// Yes, this example currently uses some extensions:

    // For input:
    #include <boost/geometry/extensions/gis/io/wkt/read_wkt.hpp>

    // For output:
    #include <boost/geometry/extensions/io/svg/write_svg.hpp>

    // For distance-calculations over the Earth:
    #include <boost/geometry/extensions/gis/geographic/strategies/andoyer.hpp>



// Read an ASCII file containing WKT's, fill a vector of tuples
// The tuples consist of at least <0> a geometry and <1> an identifying string
template <typename Geometry, typename Tuple, typename Box>
void read_wkt(std::string const& filename, std::vector<Tuple>& tuples, Box& box)
{
    std::ifstream cpp_file(filename.c_str());
    if (cpp_file.is_open())
    {
        while (! cpp_file.eof() )
        {
            std::string line;
            std::getline(cpp_file, line);
            Geometry geometry;
            boost::trim(line);
            if (! line.empty() && ! boost::starts_with(line, "#"))
            {
                std::string name;

                // Split at ';', if any
                std::string::size_type pos = line.find(";");
                if (pos != std::string::npos)
                {
                    name = line.substr(pos + 1);
                    line.erase(pos);

                    boost::trim(line);
                    boost::trim(name);
                }

                Geometry geometry;
                boost::geometry::read_wkt(line, geometry);

                Tuple tuple(geometry, name);

                tuples.push_back(tuple);
                boost::geometry::combine(box, boost::geometry::make_envelope<Box>(geometry));
            }
        }
    }
}

// Boilerplate code to initialize the SVG XML.
// Note that this is (on purpose) not part of the library but of this sample.
// GGL itself only streams small pieces of SVG, in any coordinate system
void svg_header(std::ofstream& stream)
{
    stream << "<?xml version=\"1.0\" standalone=\"no\"?>" << std::endl;
    stream << "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\"" << std::endl;
    stream << "\"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">" << std::endl;

    stream << "<svg width=\"100%\" height=\"100%\" version=\"1.1\"" << std::endl;
    stream << "xmlns=\"http://www.w3.org/2000/svg\">" << std::endl;
}


// Code to define properties for Boost Graph's
enum vertex_ggl_property_t { vertex_ggl_property };
enum edge_ggl_property_t { edge_ggl_property };
namespace boost
{
    BOOST_INSTALL_PROPERTY(vertex, ggl_property);
    BOOST_INSTALL_PROPERTY(edge, ggl_property);
}

// Define properties for vertex
template <typename Point>
struct ggl_vertex_property
{
    ggl_vertex_property()
    {
        boost::geometry::assign_zero(location);
    }
    ggl_vertex_property(Point const& loc)
    {
        location = loc;
    }

    Point location;
};

// Define properties for edge
template <typename Linestring>
struct ggl_edge_property
{
    ggl_edge_property(Linestring const& line)
        : m_line(line)
    {
        m_length = boost::geometry::length(line);
    }

    inline operator double() const
    {
        return m_length;
    }

    inline Linestring const& line() const
    {
        return m_line;
    }

private :
    double m_length;
    Linestring m_line;
};

// Utility function to add a vertex to a graph. It might exist already. Then do not insert,
// but return vertex descriptor back. It might not exist. Then add it (and return).
// To efficiently handle this, a std::map is used.
template <typename M, typename K, typename G>
inline typename boost::graph_traits<G>::vertex_descriptor find_or_insert(M& map, K const& key, G& graph)
{
    typename M::const_iterator it = map.find(key);
    if (it == map.end())
    {
        // Add a vertex to the graph
        typename boost::graph_traits<G>::vertex_descriptor new_vertex
            = boost::add_vertex(graph);

        // Set the property (= location)
        boost::put(boost::get(vertex_ggl_property, graph), new_vertex,
            ggl_vertex_property<typename M::key_type>(key));

        // Add to the map, using POINT as key
        map[key] = new_vertex;
        return new_vertex;
    }
    return it->second;
}

template
<
    typename Line,
    typename Graph,
    typename RoadTupleVector,
    typename CityTupleVector
>
void add_roads_and_connect_cities(Graph& graph,
            RoadTupleVector const& roads,
            CityTupleVector& cities)
{
    typedef typename boost::geometry::point_type<Line>::type point_type;

    typedef typename boost::graph_traits<Graph>::vertex_descriptor vertex_type;

    // Define a map to be used during graph filling
    // Maps from point to vertex-id's
    typedef std::map<point_type, vertex_type, boost::geometry::less<point_type> > map_type;
    map_type map;


    // Fill the graph
    typedef typename boost::range_value<RoadTupleVector>::type road_type;
    BOOST_FOREACH(road_type const& road, roads)
    {
        // Find or add begin/end point of these line
        vertex_type from = find_or_insert(map, road.get<0>().front(), graph);
        vertex_type to = find_or_insert(map, road.get<0>().back(), graph);
        boost::add_edge(from, to, ggl_edge_property<Line>(road.get<0>()), graph);
    }

    // Find nearest graph vertex for each city, using the map
    typedef typename boost::range_value<CityTupleVector>::type city_type;
    BOOST_FOREACH(city_type& city, cities)
    {
        double min_distance = 1e300;
        for(typename map_type::const_iterator it = map.begin(); it != map.end(); ++it)
        {
            double dist = boost::geometry::distance(it->first, city.get<0>());
            if (dist < min_distance)
            {
                min_distance = dist;
                // Set the vertex
                city.get<2>() = it->second;
            }
        }
    }
}

template <typename Graph, typename Route>
inline void add_edge_to_route(Graph const& graph,
            typename boost::graph_traits<Graph>::vertex_descriptor vertex1,
            typename boost::graph_traits<Graph>::vertex_descriptor vertex2,
            Route& route)
{
    std::pair
        <
            typename boost::graph_traits<Graph>::edge_descriptor,
            bool
        > opt_edge = boost::edge(vertex1, vertex2, graph);
    if (opt_edge.second)
    {
        // Get properties of edge and of vertex
        ggl_edge_property<Route> const& edge_prop =
            boost::get(boost::get(edge_ggl_property, graph), opt_edge.first);

        ggl_vertex_property<typename boost::geometry::point_type<Route>::type> const& vertex_prop =
            boost::get(boost::get(vertex_ggl_property, graph), vertex2);

        // Depending on how edge connects to vertex, copy it forward or backward
        if (boost::geometry::equals(edge_prop.line().front(), vertex_prop.location))
        {
            std::copy(edge_prop.line().begin(), edge_prop.line().end(),
                std::back_inserter(route));
        }
        else
        {
            std::reverse_copy(edge_prop.line().begin(), edge_prop.line().end(),
                std::back_inserter(route));
        }
    }
}


template <typename Graph, typename Route>
inline void build_route(Graph const& graph,
            std::vector<typename boost::graph_traits<Graph>::vertex_descriptor> const& predecessors,
            typename boost::graph_traits<Graph>::vertex_descriptor vertex1,
            typename boost::graph_traits<Graph>::vertex_descriptor vertex2,
            Route& route)
{
    typedef typename boost::graph_traits<Graph>::vertex_descriptor vertex_type;
    vertex_type pred = predecessors[vertex2];

    add_edge_to_route(graph, vertex2, pred, route);
    while (pred != vertex1)
    {
        add_edge_to_route(graph, predecessors[pred], pred, route);
        pred = predecessors[pred];
    }
}


int main()
{
    // Define a point in the Geographic coordinate system
    typedef boost::geometry::point<double, 2, boost::geometry::cs::geographic<boost::geometry::degree> > point_type;

    typedef boost::geometry::linestring<point_type> line_type;

    // Define the graph, lateron containing the road network
    typedef boost::adjacency_list
        <
            boost::vecS, boost::vecS, boost::undirectedS
            , boost::property<vertex_ggl_property_t, ggl_vertex_property<point_type> >
            , boost::property<edge_ggl_property_t, ggl_edge_property<line_type> >
        > graph_type;

    typedef boost::graph_traits<graph_type>::vertex_descriptor vertex_type;


    // Init a bounding box, lateron used to define SVG map
    boost::geometry::box_2d box;
    boost::geometry::assign_inverse(box);

    // Read the cities
    typedef boost::tuple<point_type, std::string, vertex_type> city_type;
    std::vector<city_type> cities;
    read_wkt<point_type>("data/cities.wkt", cities, box);

    // Read the road network
    typedef boost::tuple<line_type, std::string> road_type;
    std::vector<road_type> roads;
    read_wkt<line_type>("data/roads.wkt", roads, box);


    graph_type graph;

    // Add roads and connect cities
    add_roads_and_connect_cities<line_type>(graph, roads, cities);

    double const km = 1000.0;
    std::cout << "distances, all in KM" << std::endl
        << std::fixed << std::setprecision(0);

    // Main functionality: calculate shortest routes from/to all cities

    // For the first one, the complete route is stored as a linestring
    bool first = true;
    line_type route;

    int const n = boost::num_vertices(graph);
    BOOST_FOREACH(city_type const& city1, cities)
    {
        std::vector<vertex_type> predecessors(n);
        std::vector<double> costs(n);

        // Call Dijkstra (without named-parameter to be compatible with all VC)
        boost::dijkstra_shortest_paths(graph, city1.get<2>(),
                &predecessors[0], &costs[0],
                boost::get(edge_ggl_property, graph),
                boost::get(boost::vertex_index, graph),
                std::less<double>(), std::plus<double>(),
                (std::numeric_limits<double>::max)(), double(),
                boost::dijkstra_visitor<boost::null_visitor>());

        BOOST_FOREACH(city_type const& city2, cities)
        {
            if (! boost::equals(city1.get<1>(), city2.get<1>()))
            {
                double distance = costs[city2.get<2>()] / km;
                double acof = boost::geometry::distance(city1.get<0>(), city2.get<0>()) / km;

                std::cout
                    << std::setiosflags (std::ios_base::left) << std::setw(15)
                        << city1.get<1>() << " - "
                    << std::setiosflags (std::ios_base::left) << std::setw(15)
                        << city2.get<1>()
                    << " -> through the air: " << std::setw(4) << acof
                    << " , over the road: " << std::setw(4) << distance
                    << std::endl;

                if (first)
                {
                    build_route(graph, predecessors,
                            city1.get<2>(), city2.get<2>(),
                            route);
                    first = false;
                }
            }
        }
    }

    // Create the SVG
    typedef boost::geometry::point_xy<int> svg_point_type;
    std::ofstream stream("routes.svg");
    svg_header(stream);

    boost::geometry::strategy::transform::map_transformer
        <
            point_type,
            svg_point_type, true, true
        > matrix(box, 1000, 800);

    // Map roads
    BOOST_FOREACH(road_type const& road, roads)
    {
        boost::geometry::linestring<svg_point_type> line;
        boost::geometry::transform(road.get<0>(), line, matrix);
        stream << boost::geometry::svg(line, "stroke:rgb(128,128,128);stroke-width:1") << std::endl;
    }

    // Map the calculated route as thicker green transparent markation
    {
        boost::geometry::linestring<svg_point_type> line;
        boost::geometry::transform(route, line, matrix);
        stream << boost::geometry::svg(line, "stroke:rgb(0, 255, 0);stroke-width:6;opacity:0.5") << std::endl;
    }

    // Map cities
    BOOST_FOREACH(city_type const& city, cities)
    {
        svg_point_type point;
        boost::geometry::transform(city.get<0>(), point, matrix);
        stream << boost::geometry::svg(point, "fill:rgb(255,255,0);stroke:rgb(0,0,0);stroke-width:1") << std::endl;
    }

    stream << "</svg>" << std::endl;

    return 0;
}
