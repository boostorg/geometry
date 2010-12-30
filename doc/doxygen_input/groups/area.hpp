/*!
\defgroup area area: calculate area of a Geometry

\par Performance
2776 * 1000 area calculations are done in 0.11 seconds
(http://trac.osgeo.org/ggl/wiki/Performance#Area1)

\par Coordinate systems and strategies
Area calculation can be done in Cartesian and in spherical/geographic
coordinate systems.

\par Geometries
The area algorithm calculates the surface area of all geometries
having a surface: box, polygon, multi_polygon. The units are the square of
the units used for the points defining the surface. If the polygon is defined
in meters, the area is in square meters.

\par Example:
Example showing area calculation of polygons built
\dontinclude doxygen_1.cpp
\skip example_area_polygon()
\line {
\until }

*/
