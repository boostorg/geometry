// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.SpatialIndex - nearest neighbour filter implementation
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_FILTERS_NEAREST_FILTER_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_FILTERS_NEAREST_FILTER_HPP

namespace boost { namespace geometry { namespace index { namespace filters {

template <typename SpacialIndex>
class nearest_filter
{
public:
    typedef int* iterator;
    typedef const int* const_iterator;

    template <typename Point>
    nearest_filter(
        SpacialIndex const&,
        Point const&,
        typename traits::coordinate_type<Point>::type const&
    )
    {}

    iterator begin() { return 0; }
    iterator end() { return 0; }
    const_iterator begin() const { return 0; }
    const_iterator end() const { return 0; }
};

namespace detail {

template<typename Point>
class nearest_filtered
{
public:
    explicit nearest_filtered(
        Point const& p,
        typename traits::coordinate_type<Point>::type const& distance)
        : m_point(p), m_distance(distance) {}

    Point const& point() const { return m_point; }

    typename traits::coordinate_type<Point>::type const&
    distance() const { return m_distance; }

private:
    Point m_point;
    typename traits::coordinate_type<Point>::type m_distance;
};

} // namespace detail

template <typename Point>
detail::nearest_filtered<Point> nearest_filtered(
    Point const& p,
    typename traits::coordinate_type<Point>::type const& distance)
{
    return detail::nearest_filtered<Point>(p, distance);
}

}}}} // namespace boost::geometry::index::filters

template<typename SpacialIndex, typename Point>
boost::geometry::index::filters::nearest_filter<SpacialIndex>
    operator|(
    SpacialIndex const& si,
    boost::geometry::index::filters::detail::nearest_filtered<Point> const& f)
{
    return boost::geometry::index::filters::nearest_filter<SpacialIndex>(si, f.point(), f.distance());
}

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_FILTERS_NEAREST_FILTER_HPP
