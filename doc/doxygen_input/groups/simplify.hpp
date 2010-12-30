/*!
\defgroup simplify simplify: remove points from a geometry, keeping shape (simplification or generalization)
\par Source description:
- Wikipedia: given a 'curve' composed of line segments to find a curve
    not too dissimilar but that has fewer points

\see http://en.wikipedia.org/wiki/Ramer-Douglas-Peucker_algorithm

\par Performance
- Performance is measured on simplification of a collection of rings,
    such that 10% of the points is kept.
- 2776 counties of US are simplified in 0.7 seconds
(http://trac.osgeo.org/ggl/wiki/Performance#Simplify1)

\par Geometries
- \b linestring:
\image html svg_simplify_road.png
This US Road originally contained 34 points, the simplified version contains 7 points

- \b polygon:
\image html svg_simplify_country.png
This country (Belgium) originally contained 55 points, the simplified version contains 24 points

\note simplifying a valid simple polygon (which never intersects itself)
    might result in an invalid polygon, where the simplified rings intersect
    themselves or one of the other outer or inner rings.
Efficient simplification of a ring/polygon is still an "Open Problem"
(http://maven.smith.edu/~orourke/TOPP/P24.html#Problem.24)

- \b multi_linestring
- \b multi_polygon


*/


