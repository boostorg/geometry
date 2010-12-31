/*!
\defgroup convex_hull convex hull: calculate the convex hull of a geometry
\par Source descriptions:
- OGC description: Returns a geometric object that represents the convex hull of
    this geometric object. Convex hulls, being dependent on straight lines, can
    be accurately represented in linear interpolations
    for any geometry restricted to linear interpolations.
\see http://en.wikipedia.org/wiki/Convex_hull

\par Performance
2776 counties of US are "hulled" in 0.9 seconds
(http://trac.osgeo.org/ggl/wiki/Performance#Convexhull1)

\note The convex hull is always a ring, holes are not possible. Therefore it is
    can also be used in combination with an output iterator.

\par Geometries supported:
In the images below the convex hull is painted in red.

- \b point: will not compile

- \b linestring:

- \b polygon: will deliver a polygon without holes
    \image html svg_convex_hull_country.png

- \b multi_point:
    \image html svg_convex_hull_cities.png

- \b multi_linestring:

- \b multi_polygon:

\par Output geometries supported:

- \b polygon

- \b ring

- inserter version (with output iterator) can output to any array supporting
    points of same type as the input geometry type

*/
