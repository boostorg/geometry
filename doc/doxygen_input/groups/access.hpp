/*!
\defgroup access access: get/set coordinate values, make objects, clear geometries, append point(s)
\details There are many ways to edit geometries. It is possible to:

\li use the geometries themselves, so access point.x(). This is not done
inside the library because it is agnostic to geometry type. However,
library users can use this as it is intuitive.
\li use the standard library, so use .push_back(point) or use inserters.
This is also avoided inside the library. However, library users can use
it if they are used to the standard library
\li use the functionality provided in this geometry library. These are
the functions in this module.

The library provides the following functions to edit geometries:
\li set to set one coordinate value
\li assign to set two or more coordinate values
\li make to construct and return geometries with specified coordinates.
\li append to append one or more points to a geometry
\li clear to remove all points from a geometry

For getting coordinates it is similar:
\li get to get a coordinate value
\li or use the standard library
\li or use the geometries themselves
*/


