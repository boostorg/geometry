---
title: "Boost.Geometry: A Generic C++ Geometry Library"
tags:
  - C++
  - computational geometry
  - GIS
  - geodesy
  - spatial indexing
  - robotics
  - generic programming
authors:
  - name: Barend Gehrels
    orcid: 0000-0000-0000-0000
    affiliation: 1
  - name: Bruno Lalande
    orcid: 0000-0000-0000-0000
    affiliation: 1
  - name: Mateusz Łoskot
    orcid: 0000-0000-0000-0000
    affiliation: 1
  - name: Adam Wulkiewicz
    orcid: 0000-0000-0000-0000
    affiliation: 1
  - name: Menelaos Karavelas
    orcid: 0000-0000-0000-0000
    affiliation: 1
  - name: Vissarion Fisikopoulos
    orcid: 0000-0002-0780-666X
    affiliation: 2
affiliations:
  - name: Boost C++ Libraries
    index: 1
  - name: National Technical University of Athens
    index: 2
date: 2025-12-04
bibliography: paper.bib
---

# Summary

Computational geometry underpins numerous scientific and engineering domains, including GIS, robotics, computer graphics, and CAD. Foundational algorithms such as convex hulls, intersection tests, and point-in-polygon queries are covered in standard texts [@deBerg:2008], but robust and efficient implementations are crucial for real-world applications where numerical robustness and performance matter.

**Boost.Geometry** is an open-source C++ library providing generic, extensible, and standards-aligned geometric computation. It defines geometry *concepts* and implements algorithms in a dimension- and coordinate-system–agnostic manner. Since its inclusion in Boost (2011), it has become a widely used component in C++ scientific and industrial software. For example, MySQL uses Boost.Geometry as the geometry engine for spatial operations [@MySQLGIS:2014].


# Statement of Need

Boost.Geometry fulfills the need for a general-purpose, high-performance geometry engine in C++ with:

- A generic programming architecture via concepts, type traits, and template metaprogramming
- Support for Cartesian, spherical, and geographic coordinate systems
- Strategy-based dispatch enabling coordinate-system-specific algorithms
- OGC-compliant geometry types and spatial predicates [@OGC:2011]
- Header-only distribution, simplifying deployment and integration

A similar library is GEOS [@GEOS:2025], a C++ port of the Java Topology Suite (JTS) that implements the OGC Simple Features standard for GIS applications and serves as the core geometry engine for major geospatial tools like PostGIS [@PostGIS:2025].

CGAL [@Fabri:2000] is a comprehensive C++ library covering a broader range of geometric algorithms (triangulations, Voronoi diagrams, mesh generation, geometry processing) with a strong emphasis on robustness via exact geometric predicates.

Coordinate transformations and CRS handling are commonly performed using PROJ [@PROJ:2024], which users integrate alongside geometry libraries for projections and datum transformations. For spatial indexing, specialized libraries such as libspatialindex [@libspatialindex:2024] are frequently used; Boost.Geometry provides a header-only R-tree as a lightweight alternative.

GeographicLib [@GeographicLib:2013] and Karney's work on geodesics [@Karney:2013] provide state-of-the-art algorithms for ellipsoidal geodesic problems. 

# Design and Features

Boost.Geometry provides:

- **Geometry models:** points, boxes, rings, polygons, multi-geometries
- **Algorithms:** area, distance, length, centroid, convex hull, within, intersects, union, intersection, simplify, buffer, transformations and projections
- **Spatial indexing:** R-tree implementation supporting spatial and nearest-neighbor queries

Boost.Geometry is a generic, header-only C++ library built around stateless free functions plus template-based dispatch via concepts, traits, and geometry tags, so users can adapt their own geometry types to the Boost.Geometry interfaces (e.g., via registration macros) without changing their data structures.

Algorithms are structured as coordinate-system-agnostic core logic combined with coordinate-system-specific strategies selected from the input geometry’s declared coordinate-system tag (e.g., Cartesian vs. spherical vs. geographic). In this model, spherical computations treat Earth as a sphere (great-circle style calculations), while geographic computations use an ellipsoidal Earth model with more accurate geodetic calculations.
Boost.Geometry provides a range of geographic strategies that trade accuracy for speed: fast, efficient formulas for common use cases, and more accurate series-expansion methods based on Karney's geodesic algorithms [@Karney:2013; @isprs-archives-XLII-4-W14-45-2019].
Numerical robustness can cause failures in standard GIS algorithms [@kettner2004classroom; @isprs-archives-XLVI-4-W2-2021-1-2021]; Boost.Geometry mitigates these problems using filtered predicates that perform fast approximate tests and fall back to slower, exact evaluations when needed to ensure correct topology and predicate results.

The source code is distributed under the Boost Software License. Detailed documentation and reference material are available on the Boost website [@BoostGeomDocs]. 
A set of resources ranging from contributors guidelines and documentation generation to talks and videos about Boost Geometry is available in  [a wiki page](https://github.com/boostorg/geometry/wiki).
The repository's `test/` and `example/` directories contain unit and regression tests and usage examples demonstrating API usage and performance tests. 
Continuous integration on the Boost project provides broad platform and compiler coverage; Boost.Geometry also runs project-specific CI on GitHub Actions and CircleCI.


# References



