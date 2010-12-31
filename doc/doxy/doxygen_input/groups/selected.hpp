/*!
\defgroup selected selection: check if a geometry is "selected" by a point

Checks if one geometry is selected by a point lying within or in the neighborhood of that geometry

\par Geometries:
- POINT: checks if points are CLOSE TO each other (< search_radius)
- LINESTRING: checks if selection point is CLOSE TO linestring (< search_radius)
- RING: checks if selection point is INSIDE the ring, search radius is ignored
- POLYGON: checks if selection point is INSIDE the polygon, but not inside any of its holes

*/
