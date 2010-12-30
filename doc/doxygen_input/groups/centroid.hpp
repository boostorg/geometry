/*!
\defgroup centroid centroid: calculate centroid (center of gravity) of a geometry
\par Source descriptions:
- OGC description: The mathematical centroid for this Surface as a Point. The
    result is not guaranteed to be on this Surface.
- From Wikipedia: Informally, it is the "average" of all points
\see http://en.wikipedia.org/wiki/Centroid
\note Polygon should be closed, and can be orientated either way
\note The "centroid" functions are taking a non const reference to the centroid.
    The "make_centroid" functions return the centroid, the type has to be
    specified.
\note Both of them have an overloaded version where
    a centroid calculation strategy can be specified
\exception centroid_exception if calculation is not successful,
    e.g. because polygon didn't contain points

\par Example:
Example showing centroid calculation
\dontinclude doxygen_1.cpp
\skip example_centroid_polygon
\line {
\until }

\par Performance
2776 * 1000 centroid calculations are done in 0.16 seconds
(http://trac.osgeo.org/ggl/wiki/Performance#Centroid1)

\par Geometries:
- RING: \image html centroid_ring.png
- BOX: the centroid of a 2D or 3D box is the center of the box
- POLYGON \image html centroid_polygon.png
- POINT: the point is the centroid
- LINESTRING: the average of the centers of its segments
- MULTIPOINT: the average of the points
*/
