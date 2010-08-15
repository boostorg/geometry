/*!
\defgroup transform transform: apply transformations on geometries
\brief Transforms from one geometry to another geometry, optionally using a strategy
\details The transform algorithm automatically transforms from one coordinate system to another coordinate system.
If the coordinate system of both geometries are the same, the geometry is copied. All point(s of the geometry)
are transformed.

There is a version without a strategy, transforming automatically, and there is a version with a strategy.

This function has a lot of appliances, for example
- transform from spherical coordinates to cartesian coordinates, and back
- transform from geographic coordinates to cartesian coordinates (projections) and back
- transform from degree to radian, and back
- transform from and to cartesian coordinates (mapping, translations, etc)

The automatic transformations look to the coordinate system family, and dimensions, of the point type and by this
apply the strategy (internally bounded by traits classes).

\par Examples:
The example below shows automatic transformations to go from one coordinate system to another one:
\dontinclude doxygen_2.cpp
\skip example_for_transform()
\skipline XYZ
\until endl;

The next example takes another approach and transforms from Cartesian to Cartesian:
\skipline XY
\until endl;

\note Not every possibility is yet worked out, e.g. polar coordinate system is ignored until now
\note This "transform" is broader then geodetic datum transformations, those are currently not worked out

*/
