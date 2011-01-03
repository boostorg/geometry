// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2011, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// File defining groups for Doxygen.
// Note that group descriptions are (currently) not used.

/*!
\defgroup append append: append points to geometries
*/

/*!
\defgroup make make: construct geometries
*/

/*!
\defgroup assign assign: assign values to geometries
*/

/*!
\defgroup clear clear: clear geometries
*/

/*!
\defgroup correct correct: correct geometries
*/


/*!
\defgroup get get: get geometries
*/

/*!
\defgroup set set: set geometries
*/

/*!
\defgroup num_geometries number of geometries: calculate the number of geometries in a multi-geometry
*/

/*!
\defgroup num_interior_rings number of interior rings: calculate the number of interior rings
*/

/*!
\defgroup num_points number of points: calculate number of points of a geometry
*/

/*!
\defgroup exterior_ring exterior_ring: exterior_ring
*/

/*!
\defgroup interior_rings interior_rings: interior_rings
*/

/*!
\defgroup ranges ranges: ranges
*/

/*!
\defgroup strategies strategies: strategies
*/

// -------------------------------------------------------
// Former algorithms
// -------------------------------------------------------

/*!
\defgroup area area: calculate area of a Geometry

\par Performance
2776 * 1000 area calculations are done in 0.11 seconds
(http://trac.osgeo.org/ggl/wiki/Performance#Area1)

\par Coordinate systems and strategies
Area calculation can be done in Cartesian and in spherical/geographic
coordinate systems.

\par Geometries
The area algorithm calculates the surface area of all geometries
having a surface: box, polygon, multi_polygon. The units are the square of
the units used for the points defining the surface. If the polygon is defined
in meters, the area is in square meters.

\par Example:
Example showing area calculation of polygons built
\dontinclude doxygen_1.cpp
\skip example_area_polygon()
\line {
\until }

*/


/*!
\defgroup buffer buffer: calculate buffer of a geometry
\par Source description:
- OGC: Returns a geometric object that represents all Points whose distance
from this geometric object is less than or equal to distance. Calculations are in the spatial reference system of
this geometric object. Because of the limitations of linear interpolation, there will often be some relatively
small error in this distance, but it should be near the resolution of the coordinates used
\see http://en.wikipedia.org/wiki/Buffer_(GIS)
*/


/*!
\defgroup centroid centroid: calculate centroid (center of gravity) of a geometry
\par Source descriptions:
- OGC description: The mathematical centroid for this Surface as a Point. The
    result is not guaranteed to be on this Surface.
- From Wikipedia: Informally, it is the "average" of all points
\see http://en.wikipedia.org/wiki/Centroid
\note Polygon should be closed, and can be orientated either way
\note The "centroid" functions are taking a non const reference to the centroid.
    The "make_centroid" functions return the centroid, the type has to be
    specified.
\note Both of them have an overloaded version where
    a centroid calculation strategy can be specified
\exception centroid_exception if calculation is not successful,
    e.g. because polygon didn't contain points

\par Example:
Example showing centroid calculation
\dontinclude doxygen_1.cpp
\skip example_centroid_polygon
\line {
\until }

\par Performance
2776 * 1000 centroid calculations are done in 0.16 seconds
(http://trac.osgeo.org/ggl/wiki/Performance#Centroid1)

\par Geometries:
- RING: \image html centroid_ring.png
- BOX: the centroid of a 2D or 3D box is the center of the box
- POLYGON \image html centroid_polygon.png
- POINT: the point is the centroid
- LINESTRING: the average of the centers of its segments
- MULTIPOINT: the average of the points
*/


/*!
\defgroup combine combine: add a geometry to a bounding box
\par Geometries:
- \b box + \b box -> \b box: the box will be combined with the other box
    \image html combine_box_box.png
    \note Also if the input box is incorrect, the box will correctly updated
- \b box + \b point -> \b box: the box will combined with the point
    \image html combine_box_point.png
- \b box + \b segment -> \b box
*/

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


/*!
\defgroup difference difference: difference of two geometries
*/


/*!
\defgroup disjoint disjoint: detect if geometries are not spatially related
\details disjoint means spatially disjoint, there is no overlap of interiors
    and boundaries, the intersection of interiors or boundaries is empty.

\par Geometries:
- \b point + \b point (= ! equals)
- \b point + \b box (= not within or on border)
- \b box + \b box
- \b ring + \b box
- \b polygon + \b box
- \b polygon + \b ring
- \b polygon + \b polygon

*/


/*!
\defgroup distance distance: calculate distance between two geometries
The distance algorithm returns the distance between two geometries.
\par Coordinate systems and strategies:
With help of strategies the distance function returns the appropriate distance.
If the input is in cartesian coordinates, the Euclidian distance (Pythagoras) is calculated.
If the input is in spherical coordinates (either degree or radian), the distance over the sphere is returned.
If the input is in geographic coordinates, distance is calculated over the globe and returned in meters.

\par Distance result:
Depending on calculation type the distance result is either a structure, convertable
to a double, or a double value. In case of Pythagoras it makes sense to not draw the square root in the
strategy itself. Taking a square root is relative expensive and is not necessary when comparing distances.

\par Geometries:
Currently implemented, for both cartesian and spherical/geographic:
- POINT - POINT
- POINT - SEGMENT and v.v.
- POINT - LINESTRING and v.v.

Not yet implemented:
- POINT - RING etc, note that it will return a zero if the point is anywhere within the ring

\par Example:
Example showing distance calculation of two points, in xy and in latlong coordinates
\dontinclude doxygen_1.cpp
\skip example_distance_point_point
\line {
\until }
*/


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

/*!
\defgroup for_each for_each: apply a functor to each point or segment of a geometry
\details There are two algorithms provided which walk through the points or segments
of linestrings and polygons. They are called for_each_point, for_each_segment,
after the standard library
\note For both for_each algorithms there is a \b const and a non-const version provided.
*/


/*!
\defgroup intersection intersection: calculate new geometry
    containing geometries A and B
\details The intersection of two geometries A and B is the geometry containing
    all points of A also belonging to B, but no other elements. The so-called
    clip is an intersection of a geometry with a box.
\par Source description:
- OGC: Returns a geometric object that represents the Point set intersection of
    this geometric object with another Geometry.
\see http://en.wikipedia.org/wiki/Intersection_(set_theory)
\note Any intersection can result in no geometry at all

\par Performance
- 2776 counties of US are intersected with a 100-points ellipse in 1.1 seconds
(http://trac.osgeo.org/ggl/wiki/Performance#Interesection)
- 2776 counties of US are clipped in 0.2 seconds
(http://trac.osgeo.org/ggl/wiki/Performance#Clip)


\par Geometries:
- \b polygon + \b box (clip) -> \b polygon(s)
\image html svg_intersection_polygon_box.png
\image html svg_intersection_countries.png
- \b ring + \b box (clip) -> \b polygon(s)
\image html svg_intersection_ring_box.png
- \b ring + \b ring -> \b polygon(s)
\image html svg_intersection_ring_ring.png
- \b polygon + \b ring -> \b polygon(s)
\image html svg_intersection_polygon_ring.png
- combinations above -> \b ring(s).
    <i>If the output is an ouput iterator of rings, holes are omitted</i>
- \b linestring + \b box (clip)
\image html svg_intersection_roads.png

\par Example:
Example showing clipping of linestring with box
\dontinclude doxygen_1.cpp
\skip example_clip_linestring1
\line {
\until }
\par Example:
Example showing clipping of vector, outputting vectors, with box
\dontinclude doxygen_1.cpp
\skip example_clip_linestring2
\line {
\until }
\par Example:
Example showing clipping of polygon with box
\dontinclude doxygen_1.cpp
\skip example_intersection_polygon1
\line {
\until }
*/


/*!
\defgroup intersects intersects: detect if a geometry self-intersects or if two geometries intersect
\par Source descriptions:
- OGC description: Returns 1 (TRUE) if  this geometric object spatially
    intersects anotherGeometry.
- OGC: a.Intersects(b) <=> ! a.Disjoint(b)
\note There are two overloaded versions:
- with one geometry, detecting self-intersections
- with two geometries, deferring to disjoint, returning !disjoint

\par Geometries:
- \b ring
- \b polygon
- for two geometries: same is disjoint

\note if one geometry is completely within another geometry, it "intersects"

*/

/*!
\defgroup length length: calculate length of a linear geometry
The length algorithm is implemented for the linestring and the multi_linestring
geometry and results in the length of the linestring. If the points of
a linestring have coordinates expressed in kilometers,
the length of the line is expressed in kilometers as well.
\par Example:
Example showing length calculation
\dontinclude doxygen_1.cpp
\skip example_length_linestring_iterators1
\line {
\until }
*/





/*!
\defgroup overlaps overlaps: detect overlap between two geometries
\par Source descriptions:
- Egenhofer: Two objects overlap if they have common interior faces and the bounding faces have common parts
with the opposite interior faces.

\par Geometries:
- \b box + \b box

*/


/*!
\defgroup perimeter perimeter: calculate perimeter of a geometry
\par Geometries:
- \b polygon
- \b box
- \b linear_ring
- \b multi_polygon
*/

/*!
\defgroup reverse reverse: reverse a geometry
    This is functionally equivalent to the std::reverse algorithm.
    For a linestring or a linear ring, it is exactly the same as calling the std::reverse algorithm.
    For a polygon or a multi-geometry, all its rings or elements are reversed.

    No check on order is applied. So a clockwise polygon (having positive area)
    will be made counterclockwise (having negative area).

    The first and last points are reversed as well, even if they are closed and the same.
*/



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



/*!
\defgroup sym_difference sym_difference: sym_difference of two geometries
*/
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

/*!
\defgroup unique unique: make a geometry unique w.r.t. points,
        so no duplicate consecutive points

*/



/*!
\defgroup within within: detect if a geometry is inside another geometry, a.o. point-in-polygon

\par Source descriptions:
- OGC: Returns 1 (TRUE) if this geometric object is "spatially within"
        another Geometry.

\par Performance
- 2776 within determinations using bounding box and polygon are done
    in 0.05 seconds (http://trac.osgeo.org/ggl/wiki/Performance#Within1)
- note that using another strategy the performance can be increased:
  - winding            : 0.093 s
  - franklin           : 0.062 s
  - Crossings-multiply : 0.047 s
- but note also that the last two do not detect point-on-border cases

\par Geometries:
- \b point + \b polygon: The well-known point-in-polygon, returning true if
    a point falls within a polygon (and not
within one of its holes) \image html within_polygon.png
- \b point + \b ring: returns true if point is completely within
    a ring \image html within_ring.png
- \b point + \b box
- \b box + \b box

\par Example:
The within algorithm is used as following:
\dontinclude doxygen_1.cpp
\skip example_within
\line {
\until }

*/











/*!
\defgroup iterators iterators: iterators
*/

/*!
\defgroup core core: meta-functions for geometry types
*/
/*!
\defgroup arithmetic arithmetic: arithmetic operations on points
*/

/*!
\defgroup svg x Extension svg: Stream SVG (Scalable Vector Graphics)
*/


/*!
\defgroup register register: Macros for registration
\details Registration of custom geometry types.
*/

/*!
\defgroup traits traits: adapt geometries
\brief Traits classes are small classes or structs to adapt geometries
such that they are recognized by the Generic Geometry Library
*/

/*!
\defgroup geometries geometries: geometries provided by default
\details The GGL can be used with the geometry classes provided by the library,
and with custom geometries registered by traits classes or registration
macros.

This documentation page refers to the geometry classes provided by the library.

*/

/*!
\defgroup wkt wkt: parse and stream WKT (Well-Known Text)
The wkt classes stream the specified geometry as \ref OGC Well Known Text (\ref WKT). It is defined for OGC geometries.
It is therefore not defined for all geometries (e.g. not for circle)
\note The implementation is independant from point type, point_xy and point_ll are supported,
as well as points with more than two coordinates.
*/

/*!
\defgroup utility utility: utility meta-functions and functions
\details The utilities, mostly in folder util, contain several headerfiles
not fitting in one of the other folders.

The following meta-functions are general and do not relate to GGL:
- add_const_if_c
- select_most_precise

They might fit into boost as a separate trait or utility, or there might
be a Boost equivalent which is yet unnoticed by the authors.


*/

/*!
\defgroup selected selection: check if a geometry is "selected" by a point

Checks if one geometry is selected by a point lying within or in the neighborhood of that geometry

\par Geometries:
- POINT: checks if points are CLOSE TO each other (< search_radius)
- LINESTRING: checks if selection point is CLOSE TO linestring (< search_radius)
- RING: checks if selection point is INSIDE the ring, search radius is ignored
- POLYGON: checks if selection point is INSIDE the polygon, but not inside any of its holes

*/


/*!
\defgroup cs coordinate systems
\brief Defines coordinate systems
\details Coordinate systems are essential for any point in the Generic Geometry Library. Many
algorithms such as distance or transform use coordinate systems to select the strategy to use.
*/




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


/*!
\defgroup concepts geometry concepts: defines and checks concepts for geometries

All GGL algorithms use concepts for their geometries. This means that all
algorithms work on the GGL-provided geometries (point, linestring, etc) but
also on custom geometries.

By declaring registration macro's or by specializating traits classes it is
possible to adapt custom or legacy geometries to fulfil  the GGL geometry
concepts.

GGL algorithms check the concepts of the input geometries.
Concept checking is done using BCCL (Boost Concept Check Library).

This means that geometries provided by library users, or legacy geometries, or
plain arrays, or boost tuples, all can be handled by the Generic Geometry
Library. Also std::vector of points, or tuples can be handled either as a
linestring or as a linear ring (polygon without holes).

There are concepts for
- points
- segment
- box
- linestring
- (linear) ring
- polygon
- multi point
- multi linestring
- multi polygon

The Generic Geometry Library uses the Boost Range Library to iterate through
standard containers, boost arrays, c-arrays.
So:
- linestring
- (linear) ring

are all just ranges. So a std::vector, std::deque, boost::array,
iterator pair or c-array will be accepted as such.

Also the multi geometries:
- multi point
- multi linestring
- multi polygon
are considered as ranges of respectively points, linestrings and polygons.

*/





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
- \b a specifies an elliptical Earth's major axis a.
- \b es defines the elliptical Earth's squared eccentricity. Optionally,
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
- \b over inhibit reduction of input longitude values to a range within ca. 180 of the central meridian.

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
