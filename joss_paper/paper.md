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
  - name: Tinko Sebastian Bartels
    orcid: 0000-0002-0084-7100
    affiliation: 3
  - name: Vissarion Fisikopoulos
    orcid: 0000-0002-0780-666X
    affiliation: 2
  - name: Barend Gehrels
    affiliation: 1
  - name: Menelaos Karavelas
    affiliation: 1
  - name: Bruno Lalande
    affiliation: 1
  - name: Mateusz Łoskot
    affiliation: 1
  - name: Adam Wulkiewicz
    affiliation: 1
affiliations:
  - name: Boost C++ Libraries
    index: 1
  - name: National Technical University of Athens, Greece
    index: 2
  - name: The Chinese University of Hong Kong, Shenzhen
    index: 3
date: 2026-02-18
bibliography: paper.bib
---

# Summary

Computational geometry underpins numerous scientific and engineering domains, including GIS, robotics, computer graphics, CAD, and astronomy. Foundational algorithms such as convex hulls, intersection tests, and point-in-polygon queries are covered in standard texts [@deBerg:2008]; however, robust and efficient implementations are crucial for real-world applications where numerical robustness and performance matter.

`Boost.Geometry`[^*] is an open-source C++ library providing generic, extensible, and standards-compliant geometric computation. It defines geometry *concepts* and implements algorithms in a dimension- and coordinate-system-agnostic manner. Since its inclusion in Boost (2011), it has become a widely used component in C++ scientific and industrial software.

[^*]: Authors are listed in alphabetical order by surname.

# Statement of need

`Boost.Geometry` fulfills the need for a general-purpose, high-performance geometry engine in C++ with:

- A generic programming architecture via concepts, type traits, and template metaprogramming
- Strategy-based dispatch enabling coordinate-system-specific algorithms
- Support for Cartesian, spherical, and geographic coordinate systems, including CRS transformations, projections, and spatial indexing
- OGC-compliant geometry types and spatial predicates [@OGC:2011]
- Header-only distribution, simplifying deployment and integration
- Support for adapting user-defined geometry types

# State of the field 

A similar library is GEOS [@GEOS:2025], a C++ port of the Java Topology Suite (JTS) that implements the OGC Simple Features standard for GIS applications and serves as the core geometry engine for major geospatial tools like PostGIS [@PostGIS:2025].

CGAL [@Fabri:2000] is a comprehensive C++ library covering a broader range of geometric algorithms (triangulations, Voronoi diagrams, mesh generation, geometry processing) with a strong emphasis on robustness via exact geometric predicates and constructions.

Coordinate transformations and CRS handling are commonly performed using PROJ [@PROJ:2024], which users integrate alongside geometry libraries for projections and datum transformations. For spatial indexing, specialized libraries such as libspatialindex [@libspatialindex:2024] are frequently used.

GeographicLib [@GeographicLib:2013] and Karney's work on geodesics [@Karney:2013] provide state-of-the-art algorithms for ellipsoidal geodesic problems.

# Software design

`Boost.Geometry` provides:

- **Geometry models:** points, boxes, rings, polygons, multi-geometries
- **Algorithms:** area, distance, length, centroid, convex hull, within, intersects, union, intersection, simplify, buffer, transformations and projections
- **Spatial indexing:** R-tree implementation supporting spatial and nearest-neighbor queries

`Boost.Geometry` is a generic, header-only C++14 library built around stateless free functions with template-based dispatch via concepts, traits, and geometry tags, so users can adapt their own geometry types to the `Boost.Geometry` interfaces (e.g., via registration macros) without changing their data structures and with no runtime overhead.

Algorithms are structured as coordinate-system-agnostic core logic combined with coordinate-system-specific strategies selected from the input geometry's declared coordinate-system tag (e.g., Cartesian vs. spherical vs. geographic). In the spherical model, computations follow great-circle arcs; in the geographic model, an ellipsoidal Earth model is used for more accurate geodetic results. Notably, algorithms such as `buffer` operate directly in the geographic or spherical domain without requiring a prior projection to a planar coordinate system, preserving geodetic accuracy throughout.

`Boost.Geometry` provides a range of geographic strategies that trade accuracy for speed: fast, efficient closed-form formulas for common use cases, and more accurate series-expansion methods based on Karney's geodesic algorithms [@Karney:2013; @isprs-archives-XLII-4-W14-45-2019].

Numerical robustness can cause silent failures in standard GIS algorithms [@kettner2004classroom; @isprs-archives-XLVI-4-W2-2021-1-2021]. `Boost.Geometry` mitigates these problems using filtered predicates [@devillers:inria-00344517] that perform fast approximate tests and fall back to exact evaluations when needed, ensuring correct topology and predicate results.

The source code is distributed under the Boost Software License. Detailed documentation and reference material are available on the Boost website [@BoostGeomDocs]. Contributor guidelines, documentation tooling, talks, and videos are collected on the [project wiki](https://github.com/boostorg/geometry/wiki). The repository's `test/` and `example/` directories contain an extensive suite of unit and regression tests alongside usage examples. Continuous integration on the Boost project provides broad platform and compiler coverage; `Boost.Geometry` also runs project-specific CI on GitHub Actions and CircleCI.

# Research impact statement

`Boost.Geometry` has demonstrated significant adoption across scientific and industrial domains since its inclusion in Boost (2011).

MySQL uses `Boost.Geometry` as the geometry engine for spatial SQL operations [@MySQLGIS:2014], exposing it to millions of database deployments worldwide.

In spatial data management research, Hecatoncheir [@Georgiadis:2025], a distributed in-memory spatial data management library, uses `Boost.Geometry` for geometry comparisons and reports orders-of-magnitude speedups over Apache Sedona, showing that `Boost.Geometry` can serve as the geometry engine in high-performance distributed systems.

In scientific computing, the lifex finite-element library [@Bucelli:2024], uses `Boost.Geometry`'s R-tree implementation for efficient nearest-neighbor searches in computational physics simulations, demonstrating applicability in numerical scientific computing beyond GIS.

The library has also been used in terrain analysis in real-time strategy video games [@Richoux:2022] for simplifying obstacle contours and pruning Voronoi diagrams, utilizing the simplify and R-tree implementations respectively.

In crowd simulation research, @Vermeulen:2017 use and evaluate `Boost.Geometry`'s R-tree for k-nearest-neighbour searching.

In robotics, @Ashtekar:2023 used `Boost.Geometry` for geometrical computations in a push-recovery controller for a bipedal robot balancing on offset planes.

In naval and polar engineering, @Metrikin:2014 integrated `Boost.Geometry` into a simulation framework for stationkeeping of a vessel in floating sea ice, leveraging its polygon geometry operations such as intersection and 
difference. A related study by @Yang:2021 used `Boost.Geometry` in a numerical model to investigate the effect of ice floe shape on ship resistance under low-concentration broken ice condition.


# Acknowledgements

We acknowledge contributions from the many developers who have submitted new features, bug reports, fixes, and feature requests to `Boost.Geometry` over the years, and support from the Boost C++ Libraries community.

# AI usage disclosure

No generative AI tools were used in the development of this software, the writing
of this manuscript, or the preparation of supporting materials.

# References



