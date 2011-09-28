// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.SpatialIndex - query filter implementation
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_FILTERS_QUERY_FILTER_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_FILTERS_QUERY_FILTER_HPP

namespace boost { namespace geometry { namespace index {

template <typename Index>
class query_filter
{
    BOOST_MPL_ASSERT_MSG(
        (false),
        NOT_IMPLEMENTED_FOR_THIS_INDEX,
        (query_filter));

    typedef int* iterator;
    typedef const int* const_iterator;

    template <typename Predicates>
    inline query_filter(
        Index const&,
        Predicates const&
    )
    {}

    inline iterator begin() { return 0; }
    inline iterator end() { return 0; }
    inline const_iterator begin() const { return 0; }
    inline const_iterator end() const { return 0; }
};

namespace detail {

// TODO: awulkiew - consider removing reference from predicates

template<typename Predicates>
struct query_filtered
{
    inline explicit query_filtered(Predicates const& pred)
        : predicates(pred)
    {}

    Predicates const& predicates;
};

} // namespace detail

template <typename Predicates>
detail::query_filtered<Predicates> query_filtered(Predicates const& pred)
{
    return detail::query_filtered<Predicates>(pred);
}

template<typename Index, typename Predicates>
index::query_filter<Index>
operator|(
    Index const& si,
    index::detail::query_filtered<Predicates> const& f)
{
    return index::query_filter<Index>(si, f.predicates);
}

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_FILTERS_QUERY_FILTER_HPP
