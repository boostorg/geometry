
/*!
\defgroup projections projections: Projections
\brief Projections are algorithms to transform earth coordinates (latlong coordinates, sphere coordinates,
coordinates in latitude longitude) to a cartesian system. Algorithms here are converted from PROJ4 (http://trac.osgeo.org/proj)

\details Almost all projection sources are converted from PROJ4, from C to a C++ template structure.
PROJ4 is originally written by Gerald Evenden (then of the USGS).

Projections can be used in combination with the Generic Geometry Library.
Because it does not use the Generic Geometry Library,
it might also be used as a standalone map projection library.
\par Projection parameters
(This list is adapted from the PROJ4 documentation.)
- \b proj is required for selection of the cartographic transformation
function and where name is an acronym for the desired projection.
- \b R specifies that the projection should be computed as a spherical Earth
with radius R.
- \b ellps The \b ellps option allows selection of standard, predefined
ellipsoid figures. For spherical only projections, the major axis is used as the
radius.
- \b a specifies an elliptical Earth’s major axis a.
- \b es defines the elliptical Earth’s squared eccentricity. Optionally,
    either \b b, \b e, \b rf or \b f may be used where \b b, \b e and \b f are respective minor axis,
    eccentricity and flattening and \b rf = 1/f
- \b R_A must be used with elliptical Earth parameters. It determines that spherical
computations be used with the radius of a sphere that has a surface area
equivalent to the selected ellipsoid.
- \b R_V can be used in a similar manner for sphere radius of an ellipse with equivalent volume.
- \b R_a must be used with elliptical Earth parameters. Spherical radius of the
arithmetic mean of the major and minor axis is used.
- \b R_g and \b R_h can be used for equivalent geometric and harmonic means of major and minor axis.
- \b R_lat_a must be used with elliptical Earth parameters. Spherical radius
of the arithmetic mean of the principle radii of the ellipsoid at latitude is used.
- \b R_lat_g can be used for equivalent geometric mean of the principle radii.
- \b x_0 false easting; added to x value of the cartesian coordinate. Used in
grid systems to avoid negative grid coordinates.
- \b y_0 false northing; added to y value of the cartesian coordinate. See x_0.
- \b lon_0 central meridian. Along with lat_0, normally determines the
geographic origin of the projection.
- \b lat_0 central parallel. See lon_0.
- \b units selects conversion of cartesian values to units specified by name.
When used, other + metric parameters must be in meters.
- \b geoc data geographic coordinates are to be treated as geocentric when this
option specified.
- \b over inhibit reduction of input longitude values to a range within ±180 of the central meridian.

\note
- Note that many projections have also their own parameters, additionally to the list above
- Note also that some of the parameters above are required for some projections, others are always optionally

\par Original copyright of PROJ4:
- Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:
- The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.
- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/


/*!
\defgroup projection projection: Projection struct's, classes
\brief Projection struct's, classes

\details
*/


/*
- +init=file:key names a file containing cartographic control parameters associated
with the keyword key.
*/
