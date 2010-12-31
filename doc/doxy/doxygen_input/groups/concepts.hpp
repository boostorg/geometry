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
