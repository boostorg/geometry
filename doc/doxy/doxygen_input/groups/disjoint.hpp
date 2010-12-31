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
