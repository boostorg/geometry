// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_BORROWED_TURNS_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_BORROWED_TURNS_HPP

#include <algorithm>
#include <cstddef>
#include <deque>
#include <exception>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

#include <boost/assert.hpp>
#include <boost/iterator/indirect_iterator.hpp>
#include <boost/geometry/strategies/cartesian/exact/borrowed_records.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian
{

template <typename Root, typename Turn>
class borrowed_turn_container
{
    using record_type = borrowed_record<Root>;
    using stage_type = borrowed_stage<Root>;
    using context_type = borrowed_context<Root>;
    using pointers = std::vector<Turn*>;

    static_assert(std::is_base_of<record_type, Turn>::value,
                  "Borrowed turns must embed their numerical record");

    struct retained_owner
    {
        stage_type stage;
        std::deque<Turn> records;

        explicit retained_owner(stage_type const& source)
            : stage(source)
        {}
    };

    stage_type m_stage;
    retained_owner* m_owner = nullptr;
    stage_type* m_previous;
    pointers m_active;
    bool m_finished = false;

    struct restore_stage
    {
        borrowed_turn_container& container;
        ~restore_stage()
        {
            container.m_stage.context.active_stage = container.m_previous;
            container.m_finished = true;
        }
    };

public:
    using value_type = Turn;
    using size_type = typename pointers::size_type;
    using difference_type = typename pointers::difference_type;
    using reference = Turn&;
    using const_reference = Turn const&;
    using iterator = boost::indirect_iterator<typename pointers::iterator>;
    using const_iterator = boost::indirect_iterator<typename pointers::const_iterator, Turn const>;

    template <typename A, typename B>
    borrowed_turn_container(context_type& context, A const& a, B const& b)
        : m_stage(context, a, b)
        , m_previous(context.active_stage)
    {
        context.active_stage = &m_stage;
    }

    borrowed_turn_container(borrowed_turn_container const&) = delete;
    borrowed_turn_container& operator=(borrowed_turn_container const&) = delete;
    borrowed_turn_container& operator=(borrowed_turn_container&&) = delete;

    borrowed_turn_container(borrowed_turn_container&& other) noexcept
        : m_stage(other.m_stage)
        , m_owner(other.m_owner)
        , m_previous(other.m_previous)
        , m_active(std::move(other.m_active))
        , m_finished(other.m_finished)
    {
        if (!m_owner && !m_finished)
        {
            BOOST_ASSERT(!other.m_stage.model[0] && !other.m_stage.model[1]);
            BOOST_ASSERT(m_stage.context.active_stage == &other.m_stage);
            m_stage.context.active_stage = &m_stage;
        }
        other.m_owner = nullptr;
        other.m_finished = true;
    }

    ~borrowed_turn_container() noexcept(false) { finish(); }

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
            BOOST_ASSERT(m_stage.context.active_stage == &m_stage);
            m_owner = &m_stage.context.template retain<retained_owner>(m_stage);
            m_stage.context.active_stage = &m_owner->stage;
        }
        BOOST_ASSERT(!m_owner->stage.finalized);
        auto& turn = m_owner->records.emplace_back(candidate);
        if (!turn.point.is_native())
        {
            auto const& seed = turn.point.record();
            if (seed.is_seed && (seed.stage == &m_stage || seed.stage == &m_owner->stage))
            {
                auto& record = static_cast<record_type&>(turn);
                record.numeric = seed.numeric;
                record.stage = &m_owner->stage;
                record.operations = {{std::addressof(turn.operations[0]),
                                      std::addressof(turn.operations[1])}};
                record.is_seed = false;
                turn.point = borrowed_point<Root>(record);
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

    void finish()
    {
        if (m_finished)
        {
            return;
        }
        auto& stage = m_owner ? m_owner->stage : m_stage;
        BOOST_ASSERT(stage.context.active_stage == &stage);
        restore_stage restore{*this};
        if (std::uncaught_exceptions() != 0 || stage.finalized)
        {
            return;
        }

        // Removed turns can still be referenced by output points or later stages.
        if (m_owner)
        {
            for (auto& turn : m_owner->records)
            {
                for (auto& operation : turn.operations)
                {
                    auto& id = operation.seg_id;
                    if (id.source_index >= 0 && id.source_index < 2 && id.segment_index >= 0)
                    {
                        auto const edge = stage.support(id);
                        BOOST_ASSERT(edge.valid());
                        id = segment_identifier(static_cast<signed_size_type>(edge.range), -1, -1,
                                                static_cast<signed_size_type>(edge.segment));
                    }
                }
            }
        }
        stage.finalized = true;
        for (auto& source : stage.sources)
        {
            source = std::monostate{};
        }
    }
};

}}}}} // namespace boost::geometry::strategies::detail::exact_cartesian

#endif // BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_BORROWED_TURNS_HPP
