/*!
\defgroup equals equals: detect if two geometries are spatially equal
\details Equals returns true if geometries are spatially equal. Spatially equal
means including the same point-set. A linestring can be spatially equal to
another linestring, even if both do not have the same amount of points.
A polygon can be spatially equal to a multi-polygon (which then has
only one element).

\par Geometries:
- \b point + \b point
- \b box + \b box

\note There is a functor \ref compare "compare" as well, which can be used
for std:: library compatibility.

*/
