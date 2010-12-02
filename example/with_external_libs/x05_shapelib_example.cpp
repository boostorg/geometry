// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// SHAPELIB example

// Shapelib is a well-known and often used library to read (and write) shapefiles by Frank Warmerdam

// To build and run this example:
// 1) download shapelib from http://shapelib.maptools.org/
// 2) extract and put the source "shpopen.cpp" in project or makefile
// 3) download a shapefile, for example world countries from http://aprsworld.net/gisdata/world
// Alternativelly, install Shapelib using OSGeo4W installer from http://trac.osgeo.org/osgeo4w/
// that provides Windows binary packages

#include "shapefil.h"

#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/cartesian2d.hpp>
#include <boost/geometry/extensions/gis/io/wkt/stream_wkt.hpp>

using namespace boost::geometry;

template <typename T, typename F>
void read_shapefile(const std::string& filename, std::vector<T>& polygons, F functor)
{
    try
    {
        SHPHandle handle = SHPOpen(filename.c_str(), "rb");
        if (handle <= 0)
        {
            throw std::string("File " + filename + " not found");
        }

        int nShapeType, nEntities;
        double adfMinBound[4], adfMaxBound[4];
        SHPGetInfo(handle, &nEntities, &nShapeType, adfMinBound, adfMaxBound );

        for (int i = 0; i < nEntities; i++)
        {
            SHPObject* psShape = SHPReadObject(handle, i );

            // Read only polygons, and only those without holes
            if (psShape->nSHPType == SHPT_POLYGON && psShape->nParts == 1)
            {
                T polygon;
                functor(psShape, polygon);
                polygons.push_back(polygon);
            }
            SHPDestroyObject( psShape );
        }
        SHPClose(handle);
    }
    catch(const std::string& s)
    {
        throw s;
    }
    catch(...)
    {
        throw std::string("Other exception");
    }
}


template <typename T>
void convert(SHPObject* psShape, T& polygon)
{
    double* x = psShape->padfX;
    double* y = psShape->padfY;
    for (int v = 0; v < psShape->nVertices; v++)
    {
        typename point_type<T>::type point;
        assign(point, x[v], y[v]);
        append(polygon, point);
    }
}


int main()
{
    std::string filename = "c:/data/spatial/shape/world_free/world.shp";

    std::vector<model::d2::polygon> polygons;

    try
    {
        read_shapefile(filename, polygons, convert<model::d2::polygon>);
    }
    catch(const std::string& s)
    {
        std::cout << s << std::endl;
        return 1;
    }

    // Do something with the polygons, for example simplify them
    for (std::vector<model::d2::polygon>::iterator it = polygons.begin(); it != polygons.end(); it++)
    {
        model::d2::polygon p;
        simplify(*it, p, 0.01);
        std::cout << it->outer().size() << "," << p.outer().size() << std::endl;
        *it = p;
    }
    std::cout << "Simplified " << polygons.size() << std::endl;

    double sum = 0;
    for (std::vector<model::d2::polygon>::const_iterator it = polygons.begin(); it != polygons.end(); it++)
    {
        sum += area(*it);
    }
    std::cout << "Total area of " << polygons.size() << " polygons, total: " << sum << std::endl;

    return 0;
}
