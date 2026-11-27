// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_OUTPUT_POLICY_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_OUTPUT_POLICY_HPP


namespace boost { namespace geometry { namespace strategies { namespace relate
{

// Emit the approximation already stored at the output boundary. Predicates
// remain exact, but emission neither refines nor certifies these coordinates.
struct approximate_output {};

// Root-bound intersections store the nearest binary64 at construction. Float
// emission rounds the exact coordinate directly to binary32.
// Integral output follows BG numeric_cast conversion under every policy.
struct nearest_output {};

}}}} // namespace boost::geometry::strategies::relate

#endif
