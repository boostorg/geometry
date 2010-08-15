/*!
\defgroup convert convert: convert geometries from one type to another
\details Convert from one geometry type to another type,
    for example from BOX to POLYGON
\par Geometries:
- \b point to \b box -> a zero-area box of a point
- \b box to \b ring -> a rectangular ring
- \b box to \b polygon -> a rectangular polygon
- \b ring to \b polygon -> polygon with an exterior ring (the input ring)
- \b polygon to \b ring -> ring, interior rings (if any) are ignored
*/

