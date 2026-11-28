// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_BOUNDARY_CYCLES_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_BOUNDARY_CYCLES_HPP

namespace boost { namespace geometry { namespace detail
{

template <typename Position, typename Step, typename Equal>
bool walk_boundary_cycle(Position const& start, Step const& step, Equal const& equal)
{
    auto current = start;
    do
    {
        if (!step(current)) return false;
    }
    while (!equal(start, current));
    return true;
}

}}} // namespace boost::geometry::detail

#endif
