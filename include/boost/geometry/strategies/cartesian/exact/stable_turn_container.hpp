// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_STABLE_TURN_CONTAINER_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_STABLE_TURN_CONTAINER_HPP

#include <algorithm>
#include <cstddef>
#include <deque>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

#include <boost/assert.hpp>
#include <boost/iterator/indirect_iterator.hpp>
#include <boost/geometry/strategies/cartesian/exact/operation_geometry.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian
{

template <typename Root, typename Turn, bool Local = false>
class stable_turn_container
{
    using record_type = intersection_record<Root>;
    using stage_type = intersection_stage<Root>;
    using context_type = geometry_context<Root>;
    using pointers = std::vector<Turn*>;

    static_assert(std::is_base_of<record_type, Turn>::value,
                  "Stable turns must embed their numerical record");

    using storage = std::deque<Turn>;

    stage_type m_stage;
    std::conditional_t<Local, std::unique_ptr<storage>, storage*> m_owner = nullptr;
    stage_type* m_previous;
    pointers m_active;
    bool m_finished = false;

public:
    using value_type = Turn;
    using size_type = typename pointers::size_type;
    using difference_type = typename pointers::difference_type;
    using reference = Turn&;
    using const_reference = Turn const&;
    using iterator = boost::indirect_iterator<typename pointers::iterator>;
    using const_iterator = boost::indirect_iterator<typename pointers::const_iterator, Turn const>;

    template <typename A, typename B>
    stable_turn_container(context_type& context, A const& a, B const& b)
        : m_stage(context, a, b)
        , m_previous(context.active_stage)
    {
        context.active_stage = &m_stage;
    }

    stable_turn_container(stable_turn_container const&) = delete;
    stable_turn_container& operator=(stable_turn_container const&) = delete;
    stable_turn_container& operator=(stable_turn_container&&) = delete;

    stable_turn_container(stable_turn_container&& other) noexcept
        : m_stage(other.m_stage)
        , m_owner(std::move(other.m_owner))
        , m_previous(other.m_previous)
        , m_active(std::move(other.m_active))
        , m_finished(other.m_finished)
    {
        if (!m_finished)
        {
            BOOST_ASSERT(!other.m_stage.segments[0] && !other.m_stage.segments[1]);
            BOOST_ASSERT(m_stage.context.active_stage == &other.m_stage);
            m_stage.context.active_stage = &m_stage;
        }
        other.m_owner = nullptr;
        other.m_finished = true;
    }

    ~stable_turn_container() { finish(); }

    iterator begin() { return iterator(m_active.begin()); }
    iterator end() { return iterator(m_active.end()); }
    const_iterator begin() const { return const_iterator(m_active.begin()); }
    const_iterator end() const { return const_iterator(m_active.end()); }
    const_iterator cbegin() const { return begin(); }
    const_iterator cend() const { return end(); }
    bool empty() const { return m_active.empty(); }
    size_type size() const { return m_active.size(); }
    reference operator[](size_type i) { return *m_active[i]; }
    const_reference operator[](size_type i) const { return *m_active[i]; }
    reference front() { return *m_active.front(); }
    const_reference front() const { return *m_active.front(); }
    reference back() { return *m_active.back(); }
    const_reference back() const { return *m_active.back(); }
    void reserve(size_type size) { m_active.reserve(size); }
    void clear() { m_active.clear(); }

    void push_back(Turn const& candidate)
    {
        BOOST_ASSERT(!m_finished);
        if (!m_owner)
        {
            if constexpr (Local)
                m_owner = std::make_unique<storage>();
            else
                m_owner = &m_stage.context.template retain<storage>();
        }
        auto& turn = m_owner->emplace_back(candidate);
        if (!turn.point.is_native())
        {
            auto const& seed = turn.point.record();
            if (seed.is_seed)
            {
                BOOST_ASSERT(seed.context == &m_stage.context);
                auto& record = static_cast<record_type&>(turn);
                record = seed;
                record.is_seed = false;
                turn.point = point_ref<Root>(record);
            }
        }
        m_active.push_back(std::addressof(turn));
    }

    template <typename Less>
    void sort(Less less) { sort(begin(), end(), less); }

    template <typename Less>
    void sort(iterator first, iterator last, Less less)
    {
        std::sort(first.base(), last.base(), [&](Turn const* a, Turn const* b)
        {
            return less(*a, *b);
        });
    }

    template <typename Predicate>
    void erase_if(Predicate predicate)
    {
        m_active.erase(std::remove_if(m_active.begin(), m_active.end(),
            [&](Turn* turn) { return predicate(*turn); }), m_active.end());
    }

    template <typename Predicate>
    void unique(Predicate predicate)
    {
        m_active.erase(std::unique(m_active.begin(), m_active.end(),
            [&](Turn const* a, Turn const* b) { return predicate(*a, *b); }), m_active.end());
    }

    void finish() noexcept
    {
        if (!m_finished)
        {
            BOOST_ASSERT(m_stage.context.active_stage == &m_stage);
            m_stage.context.active_stage = m_previous;
            m_finished = true;
        }
    }
};

}}}}} // namespace boost::geometry::strategies::detail::exact_cartesian

#endif // BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_STABLE_TURN_CONTAINER_HPP
