/*!
\defgroup reverse reverse: reverse a geometry
    This is functionally equivalent to the std::reverse algorithm.
    For a linestring or a linear ring, it is exactly the same as calling the std::reverse algorithm.
    For a polygon or a multi-geometry, all its rings or elements are reversed.

    No check on order is applied. So a clockwise polygon (having positive area)
    will be made counterclockwise (having negative area).

    The first and last points are reversed as well, even if they are closed and the same.
*/

