/*!
\defgroup within within: detect if a geometry is inside another geometry, a.o. point-in-polygon

\par Source descriptions:
- OGC: Returns 1 (TRUE) if this geometric object is "spatially within"
        another Geometry.

\par Performance
- 2776 within determinations using bounding box and polygon are done
    in 0.05 seconds (http://trac.osgeo.org/ggl/wiki/Performance#Within1)
- note that using another strategy the performance can be increased:
  - winding            : 0.093 s
  - franklin           : 0.062 s
  - Crossings-multiply : 0.047 s
- but note also that the last two do not detect point-on-border cases

\par Geometries:
- \b point + \b polygon: The well-known point-in-polygon, returning true if
    a point falls within a polygon (and not
within one of its holes) \image html within_polygon.png
- \b point + \b ring: returns true if point is completely within
    a ring \image html within_ring.png
- \b point + \b box
- \b box + \b box

\par Example:
The within algorithm is used as following:
\dontinclude doxygen_1.cpp
\skip example_within
\line {
\until }

*/
