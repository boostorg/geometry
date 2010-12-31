/*!
\defgroup distance distance: calculate distance between two geometries
The distance algorithm returns the distance between two geometries.
\par Coordinate systems and strategies:
With help of strategies the distance function returns the appropriate distance.
If the input is in cartesian coordinates, the Euclidian distance (Pythagoras) is calculated.
If the input is in spherical coordinates (either degree or radian), the distance over the sphere is returned.
If the input is in geographic coordinates, distance is calculated over the globe and returned in meters.

\par Distance result:
Depending on calculation type the distance result is either a structure, convertable
to a double, or a double value. In case of Pythagoras it makes sense to not draw the square root in the
strategy itself. Taking a square root is relative expensive and is not necessary when comparing distances.

\par Geometries:
Currently implemented, for both cartesian and spherical/geographic:
- POINT - POINT
- POINT - SEGMENT and v.v.
- POINT - LINESTRING and v.v.

Not yet implemented:
- POINT - RING etc, note that it will return a zero if the point is anywhere within the ring

\par Example:
Example showing distance calculation of two points, in xy and in latlong coordinates
\dontinclude doxygen_1.cpp
\skip example_distance_point_point
\line {
\until }
*/
