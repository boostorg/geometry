// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include "shapefil.h"

#include <boost/noncopyable.hpp>
#include <boost/geometry/geometry.hpp>
#include <boost/geometry/multi/multi.hpp>

#include <boost/geometry/extensions/gis/io/shapelib/shape_creator.hpp>
#include <boost/geometry/extensions/gis/io/shapelib/shp_create_object.hpp>
#include <boost/geometry/extensions/gis/io/shapelib/shp_create_object_multi.hpp>
#include <boost/geometry/extensions/gis/io/shapelib/dbf_write_attribute.hpp>

#include <boost/geometry/extensions/gis/io/wkt/wkt.hpp>


// Writing shapefiles has never been easier.

// Small derived class to be able to write shapes by WKT
template<typename Geometry>
class shape_creator : public boost::geometry::shape_creator<Geometry>
{
public :
	shape_creator(std::string const& name)
		: boost::geometry::shape_creator<Geometry>(name)
	{}

    // Name inspired on OGC "STGeomFromText"
	int AddGeomFromText(std::string const& wkt)
    {
        Geometry geometry;
        boost::geometry::read_wkt(wkt, geometry);
        return AddShape(geometry);
    }
};


// Next ~80 lines write 8 shapefiles including one or two features
// complete with a DBF with one attribute
int main()
{
    namespace bg = boost::geometry;
	typedef bg::point<double, 2, bg::cs::cartesian> point_type;
    try
    {
	    {
		    shape_creator<bg::linear_ring<point_type> > sc("out/pol");
		    sc.AddField<int>("dummy", 10);
		    int r = sc.AddGeomFromText("POLYGON((0 0,0 1,1 1,1 0,0 0))");
		    sc.WriteField(r, 0, 10);
	    }

	    {
		    shape_creator<bg::polygon<point_type> > sc("out/donut");
		    sc.AddField<int>("dummy", 10);
		    int r = sc.AddGeomFromText("POLYGON((0 0,0 7,4 2,2 0,0 0),(1 1,2 1,1 2,1 1))");
		    sc.WriteField(r, 0, 10);
	    }

	    {
		    shape_creator<bg::linestring<point_type> > sc("out/ls");
		    sc.AddField<int>("dummy", 10);
		    int r = sc.AddGeomFromText("LINESTRING(0 0,3 3,5 4,7 1)");
		    sc.WriteField(r, 0, 10);
	    }

	    {
		    shape_creator<point_type> sc("out/pnt");
		    sc.AddField<int>("dummy", 10);
            // Write three points including attribute
		    int r = sc.AddGeomFromText("POINT(2 2)"); sc.WriteField(r, 0, 22);
		    r = sc.AddGeomFromText("POINT(3 2)"); sc.WriteField(r, 0, 32);
		    r = sc.AddGeomFromText("POINT(2 3)"); sc.WriteField(r, 0, 23);
	    }

	    {
		    shape_creator<bg::box<point_type> > sc("out/box");
		    sc.AddField<int>("dummy", 10);
		    int r = sc.AddGeomFromText("POLYGON((1 1,2 2))");
	    }

	    {
		    shape_creator<bg::segment<point_type const> > sc("out/seg");
		    sc.AddField<int>("dummy", 10);

            // This time, write to shape as geometry and not as WKT
            // (because bg::segment is currently const -> no WKT support)
            point_type p1, p2;
            bg::read_wkt("POINT(1 1)", p1);
            bg::read_wkt("POINT(2 2)", p2);
            bg::segment<point_type const> s(p1, p2);

            int r = sc.AddShape(s);
		    sc.WriteField(r, 0, 10);
        }

	    {
		    shape_creator<bg::multi_point<point_type> > sc("out/mpnt");
		    sc.AddField<int>("dummy", 10);
		    int r = sc.AddGeomFromText("MULTIPOINT((0 0),(1 1),(5 2),(7 3))");
		    sc.WriteField(r, 0, 10);
	    }

	    {
		    shape_creator<bg::multi_linestring<bg::linestring<point_type> > > sc("out/ml");
		    sc.AddField<int>("dummy", 10);
		    int r = sc.AddGeomFromText("MULTILINESTRING((0 0,1 1,2 0,3 1),(4 4,5 3,6 5))");
		    sc.WriteField(r, 0, 10);
	    }

	    {
		    shape_creator<bg::multi_polygon<bg::polygon<point_type> > > sc("out/mp");
		    sc.AddField<std::string>("dummy", 10);
		    int r = sc.AddGeomFromText("MULTIPOLYGON(((0 0,0 7,4 2,2 0,0 0),(1 1,2 1,1 2,1 1)),((10 10,10 11,11 11,11 10,10 10)))");
		    sc.WriteField(r, 0, "test");
	    }
    }
    catch(std::exception const& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}