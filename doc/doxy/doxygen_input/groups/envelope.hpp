/*!
\defgroup envelope envelope: calculate envelope (minimum bounding rectangle) of a geometry
\par Source descriptions:
- OGC: Envelope (): Geometry - The minimum bounding rectangle (MBR) for this
    Geometry,
returned as a Geometry. The polygon is defined by the corner points of the
    bounding box
    [(MINX, MINY), (MAXX, MINY), (MAXX, MAXY), (MINX, MAXY), (MINX, MINY)].

\note Implemented in the Generic Geometry Library: The minimum bounding box,
    always as a box, having min <= max

The envelope algorithm calculates the bounding box, or envelope, of a geometry.
There are two versions:
- envelope, taking a reference to a box as second parameter
- make_envelope, returning a newly constructed box (type as a template parameter
    in the function call)

\par Geometries:
- \b point: a box with zero area, the maximum and the minimum point of the box are
set to the point itself.
- \b linestring, \b ring or \b range is the smallest box that contains all points of the
    specified point sequence.
If the linestring is empty, the envelope is the inverse infinite box, that is,
    the minimum point is very large (max infinite) and the maximum point is
    very small (min infinite).
- \b polygon, the envelope of the outer ring
\image html envelope_polygon.png

\par Example:
Example showing envelope calculation
\dontinclude doxygen_1.cpp
\skip example_envelope_linestring
\line {
\until }
*/

