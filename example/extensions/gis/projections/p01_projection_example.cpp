// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// Projection example 1, direct

#include <boost/geometry/geometry.hpp>
#include <boost/geometry/algorithms/parse.hpp>
#include <boost/geometry/geometries/cartesian2d.hpp>
#include <boost/geometry/extensions/gis/io/wkt/stream_wkt.hpp>
#include <boost/geometry/extensions/gis/latlong/latlong.hpp>
#include <boost/geometry/extensions/gis/projections/parameters.hpp>
#include <boost/geometry/extensions/gis/projections/proj/robin.hpp>

int main()
{
    using namespace boost::geometry;

    // Initialize projection parameters
    projection::parameters par = projection::init("+ellps=WGS84 +units=m");

    // Construct a Robinson projection, using specified point types
    // (This delivers a projection without virtual methods. Note that in p02 example
    //  the projection is created using a factory, which delivers a projection with virtual methods)
    projection::robin_spheroid<model::point_ll_deg, model::d2::point> prj(par);

    // Define Amsterdam / Barcelona in decimal degrees / degrees/minutes
    model::point_ll_deg amsterdam = parse<model::point_ll_deg>("52.4N", "5.9E");
    model::point_ll_deg barcelona = parse<model::point_ll_deg>("41 23'N", "2 11'E");

    model::d2::point pa, pb;

    // Now do the projection. "Forward" means from latlong to meters.
    // (Note that a map projection might fail. This is not 'exceptional'.
    // Therefore the forward function does not throw but returns false)
    if (prj.forward(amsterdam, pa) && prj.forward(barcelona, pb))
    {
        std::cout << "Amsterdam: " << pa << std::endl << "Barcelona: " << pb << std::endl;

        std::cout << "Distance (unprojected):" << distance(amsterdam, barcelona) / 1000.0 << " km" << std::endl;
        std::cout << "Distance (  projected):" << distance(pa, pb) / 1000.0 << " km" << std::endl;

        // Do the inverse projection. "Inverse" means from meters to latlong
        // It also might fail or might not exist, not all projections
        // have their inverse implemented
        model::point_ll_deg a1;
        if (prj.inverse(pa, a1))
        {
            std::cout << "Amsterdam (original): " << amsterdam  << std::endl
                << "Amsterdam (projected, and back):" << a1 << std::endl;
            std::cout << "Distance a-a': " << distance(amsterdam, a1) << " meter" << std::endl;
        }
    }

    return 0;
}
