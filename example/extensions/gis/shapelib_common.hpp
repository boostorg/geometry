#ifndef SHAPELIB_COMMON_HPP
#define SHAPELIB_COMMON_HPP

// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "shapefil.h"
#include <string>
#include <vector>

// A slightly different implementation of reading shapefile with shapelib.
// See also 08_shapefile_example
template <typename T>
void read_shapefile(const std::string& filename, std::vector<T>& rings)
{
    typedef typename boost::geometry::point_type<T>::type P;

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

        if (psShape->nSHPType == SHPT_POLYGON)
        {
            // Read all polygon parts. Holes are ignored, handled as normal parts. Add them as rings
            int v = 0;
            for (int p = 0; p < psShape->nParts && v < psShape->nVertices; p++)
            {
                T ring;
                int n = p == psShape->nParts - 1 ?psShape->nVertices : psShape->panPartStart[p + 1];
                while(v < n)
                {
                    P point;
                    boost::geometry::assign(point, psShape->padfX[v], psShape->padfY[v]);
                    ring.push_back(point);
                    v++;
                }
                rings.push_back(ring);
            }
        }

        SHPDestroyObject( psShape );
    }
    SHPClose(handle);
}

#endif
