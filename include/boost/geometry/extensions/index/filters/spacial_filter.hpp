// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.SpatialIndex - box query filter implementation
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_FILTERS_SPACIAL_FILTER_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_FILTERS_SPACIAL_FILTER_HPP

namespace boost { namespace geometry { namespace index { namespace filters {

template <typename SpacialIndex>
class spatial_filter
{
public:
    typedef int* iterator;
    typedef const int* const_iterator;

    template <typename Geometry>
    spatial_filter(SpacialIndex const&, Geometry const&) {}

    iterator begin() { return 0; }
    iterator end() { return 0; }
    const_iterator begin() const { return 0; }
    const_iterator end() const { return 0; }
};

namespace detail {

template<typename Geometry>
class spatially_filtered
{
public:
    explicit spatially_filtered(Geometry const& geom) : m_geom(geom) {}
    Geometry const& geometry() const { return m_geom; }

private:
    Geometry const& m_geom;
};

} // namespace detail

template <typename Geometry>
detail::spatially_filtered<Geometry> spatially_filtered(Geometry const& geom)
{
    return detail::spatially_filtered<Geometry>(geom);
}

}}}} // namespace boost::geometry::index::filters

template<typename SpacialIndex, typename Geometry>
boost::geometry::index::filters::spatial_filter<SpacialIndex>
operator|(
    SpacialIndex const& si,
    boost::geometry::index::filters::detail::spatially_filtered<Geometry> const& f)
{
    return boost::geometry::index::filters::spatial_filter<SpacialIndex>(si, f.geometry());
}

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_FILTERS_SPACIAL_FILTER_HPP
