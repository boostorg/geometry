// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_UTIL_SORTED_POINT_VIEW_HPP
#define BOOST_GEOMETRY_UTIL_SORTED_POINT_VIEW_HPP

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <type_traits>
#include <utility>
#include <vector>

#include <boost/iterator/indirect_iterator.hpp>
#include <boost/iterator/iterator_categories.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/range/iterator.hpp>
#include <boost/range/size.hpp>

#include <boost/geometry/core/tag.hpp>

namespace boost { namespace geometry { namespace util
{

// The source stays immutable and alive while this view or its iterators are used.
template <typename Range>
class sorted_point_view
{
    using source_iterator = typename boost::range_iterator<Range const>::type;
    struct cursor
    {
        using value_type = typename std::iterator_traits<source_iterator>::value_type;
        using source_reference = decltype(*std::declval<source_iterator&>());
        using point_reference = std::conditional_t<
            std::is_same<geometry::tag_t<source_reference>, point_tag>::value,
            source_reference, value_type>;
        using reference = std::conditional_t<std::is_lvalue_reference<point_reference>::value,
            std::add_lvalue_reference_t<std::add_const_t<std::remove_reference_t<point_reference>>>,
            point_reference>;

        explicit cursor(source_iterator const& iterator) : m_iterator(iterator) {}
        reference operator*() const { return *m_iterator; }

        // Read through the stored cursor: its reference may name its own cache.
        mutable source_iterator m_iterator;
    };
    using index_type = std::vector<cursor>;

public:
    using value_type = typename cursor::value_type;
    using const_iterator = boost::indirect_iterator<typename index_type::const_iterator,
        value_type, boost::random_access_traversal_tag, typename cursor::reference>;
    using iterator = const_iterator;

    template <typename Less>
    explicit sorted_point_view(Range const& range, Less const& less)
    {
        static_assert(std::is_convertible<typename boost::iterator_traversal<source_iterator>::type,
                          boost::forward_traversal_tag>::value,
                      "Sorted point views require stable multipass source iterators");
        m_indices.reserve(boost::size(range));
        for (auto it = boost::begin(range), end = boost::end(range); it != end; ++it)
            m_indices.emplace_back(it);
        std::sort(m_indices.begin(), m_indices.end(), [&](cursor const& a, cursor const& b)
        {
            return less(*a, *b);
        });
    }

    template <typename Less>
    sorted_point_view(Range const&&, Less const&) = delete;

    const_iterator begin() const { return const_iterator(m_indices.begin()); }
    const_iterator end() const { return const_iterator(m_indices.end()); }
    std::size_t size() const { return m_indices.size(); }
    bool empty() const { return m_indices.empty(); }

private:
    index_type m_indices;
};

}}} // namespace boost::geometry::util

#endif
