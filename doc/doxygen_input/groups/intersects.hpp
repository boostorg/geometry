/*!
\defgroup intersects intersects: detect if a geometry self-intersects or if two geometries intersect
\par Source descriptions:
- OGC description: Returns 1 (TRUE) if  this geometric object spatially
    intersects anotherGeometry.
- OGC: a.Intersects(b) <=> ! a.Disjoint(b)
\note There are two overloaded versions:
- with one geometry, detecting self-intersections
- with two geometries, deferring to disjoint, returning !disjoint

\par Geometries:
- \b ring
- \b polygon
- for two geometries: same is disjoint

\note if one geometry is completely within another geometry, it "intersects"

*/
