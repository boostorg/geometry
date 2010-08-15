/*!
\defgroup intersection intersection: calculate new geometry
    containing geometries A and B
\details The intersection of two geometries A and B is the geometry containing
    all points of A also belonging to B, but no other elements. The so-called
    clip is an intersection of a geometry with a box.
\par Source description:
- OGC: Returns a geometric object that represents the Point set intersection of
    this geometric object with another Geometry.
\see http://en.wikipedia.org/wiki/Intersection_(set_theory)
\note Any intersection can result in no geometry at all

\par Performance
- 2776 counties of US are intersected with a 100-points ellipse in 1.1 seconds
(http://trac.osgeo.org/ggl/wiki/Performance#Interesection)
- 2776 counties of US are clipped in 0.2 seconds
(http://trac.osgeo.org/ggl/wiki/Performance#Clip)


\par Geometries:
- \b polygon + \b box (clip) -> \b polygon(s)
\image html svg_intersection_polygon_box.png
\image html svg_intersection_countries.png
- \b ring + \b box (clip) -> \b polygon(s)
\image html svg_intersection_ring_box.png
- \b ring + \b ring -> \b polygon(s)
\image html svg_intersection_ring_ring.png
- \b polygon + \b ring -> \b polygon(s)
\image html svg_intersection_polygon_ring.png
- combinations above -> \b ring(s).
    <i>If the output is an ouput iterator of rings, holes are omitted</i>
- \b linestring + \b box (clip)
\image html svg_intersection_roads.png

\par Example:
Example showing clipping of linestring with box
\dontinclude doxygen_1.cpp
\skip example_clip_linestring1
\line {
\until }
\par Example:
Example showing clipping of vector, outputting vectors, with box
\dontinclude doxygen_1.cpp
\skip example_clip_linestring2
\line {
\until }
\par Example:
Example showing clipping of polygon with box
\dontinclude doxygen_1.cpp
\skip example_intersection_polygon1
\line {
\until }
*/
