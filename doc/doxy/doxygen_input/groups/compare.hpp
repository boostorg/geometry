/*!
\defgroup compare compare: define compare functors for points
\details The compare policies enable to compare points in a way that they can
be  compared in many functions of the standard library.

The functors do have an optional second template argument, \b dimension.
If dimension is specified, comparison is only done on that dimension.

This is useful for (a.o.):
- std::sort  (use geometry::less<P> or geometry::greater<P> or geometry::less<P, 1>)
- std::map (use geometry::less<P>)
- std::unique_copy (use geometry::equal_to<P>)

\par Geometries:
- \b point


\par Example:
Example showing how geometry::less can be used
\dontinclude doxygen_3.cpp
\skip example_less()
\line {
\until }


\note There is a boolean function \ref equals "equals" as well, which returns
true or false if a geometry is spatially equal to another geometry. That one
is defined for OGC compatibility, while these ones are defined for
compatibility with the std:: library. These ones are functors, operating on
the same geometry type (currently only the point-type), the equals function is
a free function operating on different point types or even different geometry
types (a linestring can be spatially equal to a multi-linestring).
*/
