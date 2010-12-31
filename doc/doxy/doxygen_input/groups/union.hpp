
/*!
\defgroup union union: calculate new geometry containing geometries A or B
\details The union of two geometries A and B is the geometry containing
    all points belong to either of A or B, but no other elements.
\par Source description:
- OGC: Returns a geometric object that represents the Point set union of
    this geometric object with another Geometry.
\see http://en.wikipedia.org/wiki/Union_(set_theory)
\note A union of two rings can result in a polygon having a hole

\par Geometries:
- \b polygon + \b box -> \b polygon(s)
\image html svg_union_polygon_box.png
- \b ring + \b box -> \b polygon(s)
\image html svg_union_ring_box.png
- \b ring + \b ring -> \b polygon(s)
\image html svg_union_ring_ring.png
- \b polygon + \b ring -> \b polygon(s)
\image html svg_union_polygon_ring.png
- combinations above -> \b ring(s).
    <i>If the output is an ouput iterator of rings, holes are omitted</i>

*/
