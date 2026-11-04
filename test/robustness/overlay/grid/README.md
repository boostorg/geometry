# Random Grid Tests

The four line families are `x = 20i`, `y = 15j`, `3x + 4y = 120k`
and `3x - 4y = 120k`, for integer indices. Every intersection lies at
`(20i, 15j)`. Each 20-by-15 rectangle has two triangles: a rising diagonal
when `i+j` is even, a falling one otherwise. The pattern repeats every two
columns and rows; `--width` and `--height` count rectangles, not pairs.
Interior vertices alternate between degree eight and degree four.

The diagonal spacing is twice that of the original three-direction grid.
This avoids additional intersections at rectangle centres.
Scaling the original grid by two embeds all of its lines in this grid, so its
geometries remain representable as unions of the new cells and edges.
The miter-buffer distance is fixed at 120: it shifts columns by 6, rows by 8,
and diagonal supports by 5 line indices. There is no `--max-distance` option.

## Buffer

Buffer uses standard straight sides, miter joins (limit 5), flat linestring
ends and square point buffers. The sharpest grid corner has miter length
`sqrt(10)*distance`, below the limit. The grid margin accommodates coordinate
displacements up to three times the maximum distance.

The reference uses fixed cell-index helpers: rectangles for square point
buffers and axis-aligned sides/corners, and seven row-mask tables for diagonal
sides/corners. Translation and reflection use only index arithmetic. No points,
normals, offsets or geometric predicates are constructed or evaluated by the
reference. At distance 120 no cell sample lies on a helper's boundary, so their
cell masks combine with ordinary bitset OR; sector coverage is unnecessary.

Polygon boundary edges come from incident cell bits. Line and ring vertex paths
are recorded during geometry assembly, before collinear vertices are removed.
They preserve which edges actually join within a member, including where a hole
touches an outer ring: miter buffering does not distribute over primitive unions.
Ring closure follows the ring traits. Collinear joins contribute nothing, and
closed linestrings retain their explicit end caps.

The linear buffer's margin mask enables only horizontal and vertical edges,
before geometry creation. Diagonal edges are disabled throughout each member,
not just at its ends: exposed internal concave connectors could otherwise
introduce perpendicular directions and noninteger intersections. Axis-aligned
linear buffering stays on grid lines. Polygon buffering and the set-operation
and relate tests retain both diagonal directions. No endpoint trimming is needed.
As before, comparison checks only strict membership at cell test points,
not output area or grid alignment. The algorithm under test retains its
double-coordinate buffer model, despite the mathematically integer construction.

Build from the Boost root:

```sh
./b2 -j3 libs/geometry/test/robustness/overlay/grid variant=release
```

All three executables accept `--svg DIRECTORY` to write SVGs on failure. The
directory must already exist. For example, with the executable on `PATH`:

```sh
mkdir -p /tmp/grid-failures
random_grid_relate --width 10 --height 10 --seed 2027090710 --count 4 \
    --svg /tmp/grid-failures
```

`--svg-limit N` limits output to the first N failure reports (default 20);
`--svg-limit 0` writes every failure. Without `--svg`, no SVGs are written.

Each filename identifies the test, operation, input types, tile dimensions,
seed and zero-based iteration. Buffer filenames also identify the fixed
distance. Repeating the same case overwrites its SVG.

Set-operation SVGs show inputs, expected output and raw actual output. Sample
markers identify missing or unexpected membership; length, area and validity
diagnostics appear underneath. Relate SVGs show inputs and both DE-9IM matrices,
with differing entries highlighted. Buffer SVGs show expected cells directly,
without another union operation. Construction failures are drawn too; invalid
geometries are not queried for sample membership.

The SVG includes replay options and input/output WKT in its descriptions.
These are the original failures, not automatically reduced cases. Drawing uses
floating-point screen coordinates only; test arithmetic is unchanged.

## Exact inputs

The set-operation and relate tests accept `--point1`, `--point2`, `--line1`,
`--line2`, `--area1`, and `--area2`, using the printed `{decimal chunks}` format.
Each chunk is an unsigned 64-bit integer; chunk zero contains bits 0 through 63,
with bit zero least significant. Missing trailing chunks are zero. `{}` is empty.
Too many chunks, nonzero padding bits, malformed input, and overflow are rejected.

Supplying any pattern selects exact mode: unspecified inputs are empty and the
default iteration count is one. `--count` can explicitly repeat the case. The
seed does not affect exact inputs. Without any patterns, randomized behavior
and seed sequences are unchanged.

`--line-vertices1`, `--line-vertices2`, `--area-vertices1`, and `--area-vertices2`
specify which collinear vertices to retain. They use the grid's vertex indices;
an omitted pattern in exact mode removes all removable collinear vertices.
Failures print a complete one-iteration replay including these patterns.
Exact-input SVGs also store the supplied options in their description.

## Issue 1490

This is the regression for the nested, vertex-touching hole defect, fixed on
this branch:

```sh
random_grid_set_operations --width 4 --height 5 \
    --area1 '{261990907903}' --area2 '{339491840}' --verbose
```

A selects 35 cells; B selects 10. Cell 21 is A's triangular hole, with vertices
`(40,30)`, `(40,45)`, `(60,45)`. B contains it and touches it only at `(60,45)`;
B is strictly inside A's outer ring. Both inputs are valid.

Difference now has area 3900 and valid output. Symmetric difference preserves
the triangular island and has area 4050 in both operand orders. Before the
ring-selection fix these areas were 3750 and 3900, respectively. Relate also
passes. Retaining all collinear vertices covered the same defect.

The patterns are minimal for this one-cell-hole, single-contact construction:
the noncontact hole vertices require all their incident cells in B, and B's
vertices require all their incident cells inside A's outer ring. These force
10 and 36 cells respectively, with the hole removed from A. Enumerating these
mandatory neighborhoods in every grid of at most 20 tiles finds no smaller
grid, and four configurations at 4x5/5x4, two of which failed. This is not a claim
of global minimality among arbitrary polygons or different failure topologies.

Controls: replacing A by `{261992480767}` horizontally reflects the hole and
passes; filling the hole (`--area1 '{261993005055}'`) also passes. A simpler
rectangular-outer-ring version uses A `{1099509530623}` and B `{1010580480}` on
the same grid; its difference now has the expected area 4200 (formerly 4050).
