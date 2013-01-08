// Boost.Geometry Index
//
// Spatial query range adaptor
//
// Copyright (c) 2011-2012 Adam Wulkiewicz, Lodz, Poland.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_ADAPTORS_SPATIAL_QUERY_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_ADAPTORS_SPATIAL_QUERY_HPP

/*!
\defgroup adaptors Spatial indexes adaptors (boost::geometry::index::adaptors::)
*/

namespace boost { namespace geometry { namespace index {

namespace adaptors {

template <typename Index>
class spatial_query_range
{
    BOOST_MPL_ASSERT_MSG(
        (false),
        NOT_IMPLEMENTED_FOR_THIS_INDEX,
        (spatial_query_range));

    typedef int* iterator;
    typedef const int* const_iterator;

    template <typename Predicates>
    inline spatial_query_range(
        Index const&,
        Predicates const&)
    {}

    inline iterator begin() { return 0; }
    inline iterator end() { return 0; }
    inline const_iterator begin() const { return 0; }
    inline const_iterator end() const { return 0; }
};

namespace detail {

// TODO: awulkiew - consider removing reference from predicates

template<typename Predicates>
struct spatial_query
{
    inline explicit spatial_query(Predicates const& pred)
        : predicates(pred)
    {}

    Predicates const& predicates;
};

} // namespace detail

/*!
\brief The spatial query index adaptor generator.

\ingroup adaptors

\param pred   Spatial predicates.
*/
template <typename Predicates>
detail::spatial_query<Predicates>
spatial_queried(Predicates const& pred)
{
    return detail::spatial_query<Predicates>(pred);
}

} // namespace adaptors

template<typename Index, typename Predicates>
index::adaptors::spatial_query_range<Index>
operator|(
    Index const& si,
    index::adaptors::detail::spatial_query<Predicates> const& f)
{
    return index::adaptors::spatial_query_range<Index>(si, f.predicates);
}

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_ADAPTORS_SPATIAL_QUERY_HPP
