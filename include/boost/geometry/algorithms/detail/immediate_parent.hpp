// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_IMMEDIATE_PARENT_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_IMMEDIATE_PARENT_HPP

namespace boost { namespace geometry { namespace detail
{

// Strict containment of a family of nested or disjoint boundaries. Return
// count if none contains child. Candidate eligibility belongs to contains.
template <typename Index, typename Contains>
Index immediate_parent(Index count, Index child, Contains const& contains)
{
    Index parent = count;
    for (Index candidate = 0; candidate < count; ++candidate)
        if (candidate != child && contains(child, candidate)
            && (parent == count || contains(candidate, parent)))
            parent = candidate;
    return parent;
}

}}} // namespace boost::geometry::detail

#endif
