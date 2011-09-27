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

namespace boost { namespace geometry { namespace index {

template <typename Index>
class nearest_filter
{
    BOOST_MPL_ASSERT_MSG(
        (false),
        NOT_IMPLEMENTED_FOR_THIS_INDEX,
        (nearest_filter));

    typedef int* iterator;
    typedef const int* const_iterator;

    template <typename DistancesPredicates, typename Predicates>
    inline nearest_filter(
        Index const&,
        DistancesPredicates const&,
        size_t,
        Predicates const&
    )
    {}

    inline iterator begin() { return 0; }
    inline iterator end() { return 0; }
    inline const_iterator begin() const { return 0; }
    inline const_iterator end() const { return 0; }
};

namespace detail {

template<typename DistancesPredicates, typename Predicates>
struct nearest_filtered
{
    inline nearest_filtered(
        DistancesPredicates const& dpred,
        size_t k,
        Predicates const& pred
    )
        : distances_predicates(dpred)
        , count(k)
        , predicates(pred)
    {}

    DistancesPredicates const& distances_predicates;
    size_t count;
    Predicates const& predicates;
};

} // namespace detail

template <typename DistancesPredicates, typename Predicates>
detail::nearest_filtered<DistancesPredicates, Predicates> nearest_filtered(
    DistancesPredicates const& dpred,
    size_t k,
    Predicates const& pred = detail::empty())
{
    return detail::nearest_filtered<DistancesPredicates, Predicates>(dpred, k, pred);
}

template<typename Index, typename DistancesPredicates, typename Predicates>
index::nearest_filter<Index>
operator|(
    Index const& si,
    detail::nearest_filtered<DistancesPredicates, Predicates> const& f)
{
    return index::nearest_filter<Index>(si, f.distances_predicates, f.count, f.predicates);
}

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_FILTERS_NEAREST_FILTER_HPP
