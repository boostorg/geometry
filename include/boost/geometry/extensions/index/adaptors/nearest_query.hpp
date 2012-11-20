// Boost.Geometry Index
//
// Nearest neighbour query range adaptor
//
// Copyright (c) 2011-2012 Adam Wulkiewicz, Lodz, Poland.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_ADAPTORS_NEAREST_QUERY_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_ADAPTORS_NEAREST_QUERY_HPP

namespace boost { namespace geometry { namespace index {

namespace adaptors {

template <typename Index>
class nearest_query_range
{
    BOOST_MPL_ASSERT_MSG(
        (false),
        NOT_IMPLEMENTED_FOR_THIS_INDEX,
        (nearest_query_range));

    typedef int* iterator;
    typedef const int* const_iterator;

    template <typename DistancesPredicates, typename Predicates>
    inline nearest_query_range(
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

// TODO: awulkiew - consider removing references from predicates

template<typename DistancesPredicates, typename Predicates>
struct nearest_query
{
    inline nearest_query(
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
detail::nearest_query<DistancesPredicates, Predicates>
nearest_queried(
    DistancesPredicates const& dpred,
    size_t k,
    Predicates const& pred)
{
    return detail::nearest_query<DistancesPredicates, Predicates>(dpred, k, pred);
}

template <typename DistancesPredicates>
detail::nearest_query<DistancesPredicates, index::detail::empty>
nearest_queried(
    DistancesPredicates const& dpred,
    size_t k)
{
    return detail::nearest_query<DistancesPredicates, index::detail::empty>(dpred, k, index::detail::empty());
}

} // namespace adaptors

template<typename Index, typename DistancesPredicates, typename Predicates>
index::adaptors::nearest_query_range<Index>
operator|(
    Index const& si,
    index::adaptors::detail::nearest_query<DistancesPredicates, Predicates> const& f)
{
    return index::adaptors::nearest_query_range<Index>(
        si, f.distances_predicates, f.count, f.predicates);
}

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_ADAPTORS_NEAREST_QUERY_HPP
