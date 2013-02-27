// Boost.Geometry Index
//
// R-tree algorithms parameters
//
// Copyright (c) 2011-2013 Adam Wulkiewicz, Lodz, Poland.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_INDEX_PARAMETERS_HPP
#define BOOST_GEOMETRY_INDEX_PARAMETERS_HPP

#include <limits>

namespace boost { namespace geometry { namespace index {

// TODO: awulkiew - implement those:
//if ( m_min_elems_per_node < 1 )
//    m_min_elems_per_node = 1;
//if ( m_max_elems_per_node < 2 )
//    m_max_elems_per_node = 2;

/*!
\brief Linear r-tree creation algorithm parameters.

\tparam MaxElements     Maximum number of elements in nodes.
\tparam MinElements     Minimum number of elements in nodes.
*/
template <size_t MaxElements, size_t MinElements>
struct linear
{
    static const size_t max_elements = MaxElements;
    static const size_t min_elements = MinElements;

    static size_t get_max_elements() { return MaxElements; }
    static size_t get_min_elements() { return MinElements; }
};

/*!
\brief Quadratic r-tree creation algorithm parameters.

\tparam MaxElements     Maximum number of elements in nodes.
\tparam MinElements     Minimum number of elements in nodes.
*/
template <size_t MaxElements, size_t MinElements>
struct quadratic
{
    static const size_t max_elements = MaxElements;
    static const size_t min_elements = MinElements;

    static size_t get_max_elements() { return MaxElements; }
    static size_t get_min_elements() { return MinElements; }
};

namespace detail { 

template <size_t MaxElements>
struct default_rstar_reinserted_elements_s
{
    static const size_t value = (MaxElements * 3) / 10;
};

} // namespace detail

/*!
\brief R*-tree creation algorithm parameters.

\tparam MaxElements             Maximum number of elements in nodes.
\tparam MinElements             Minimum number of elements in nodes.
\tparam OverlapCostThreshold    The number of leaf node children elements above which
                                nearly minimum overlap cost is calculated instead of minimum
                                overlap cost. If 0 minimum overlap cost is always calculated.
\tparam ReinsertedElements      Number of elements reinserted by forced reinsertions algorithm.
*/
template <size_t MaxElements,
          size_t MinElements,
          size_t OverlapCostThreshold = 0,
          size_t ReinsertedElements = detail::default_rstar_reinserted_elements_s<MaxElements>::value
          >
struct rstar
{
    static const size_t max_elements = MaxElements;
    static const size_t min_elements = MinElements;
    static const size_t overlap_cost_threshold = OverlapCostThreshold;
    static const size_t reinserted_elements = ReinsertedElements;

    static size_t get_max_elements() { return MaxElements; }
    static size_t get_min_elements() { return MinElements; }
    static size_t get_overlap_cost_threshold() { return OverlapCostThreshold; }
    static size_t get_reinserted_elements() { return ReinsertedElements; }
};

//template <size_t MaxElements, size_t MinElements>
//struct kmeans
//{
//    static const size_t max_elements = MaxElements;
//    static const size_t min_elements = MinElements;
//};

/*!
\brief Linear r-tree creation algorithm parameters - run-time version.
*/
class dynamic_linear
{
public:
    /*!
    \brief The constructor.

    \param max_elements     Maximum number of elements in nodes.
    \param min_elements     Minimum number of elements in nodes.
    */
    dynamic_linear(size_t max_elements, size_t min_elements)
        : m_max_elements(max_elements)
        , m_min_elements(min_elements)
    {}

    size_t get_max_elements() const { return m_max_elements; }
    size_t get_min_elements() const { return m_min_elements; }

private:
    size_t m_max_elements;
    size_t m_min_elements;
};

/*!
\brief Quadratic r-tree creation algorithm parameters - run-time version.
*/
class dynamic_quadratic
{
public:
    /*!
    \brief The constructor.

    \param max_elements     Maximum number of elements in nodes.
    \param min_elements     Minimum number of elements in nodes.
    */
    dynamic_quadratic(size_t max_elements, size_t min_elements)
        : m_max_elements(max_elements)
        , m_min_elements(min_elements)
    {}

    size_t get_max_elements() const { return m_max_elements; }
    size_t get_min_elements() const { return m_min_elements; }

private:
    size_t m_max_elements;
    size_t m_min_elements;
};

namespace detail { 

inline size_t default_rstar_reinserted_elements_d()
{
    return (std::numeric_limits<size_t>::max)();
};

} // namespace detail

/*!
\brief R*-tree creation algorithm parameters - run-time version.
*/
class dynamic_rstar
{
public:
    /*!
    \brief The constructor.

    \param max_elements             Maximum number of elements in nodes.
    \param min_elements             Minimum number of elements in nodes.
    \param overlap_cost_threshold   The number of leaf node children elements above which
                                    nearly minimum overlap cost is calculated instead of minimum
                                    overlap cost. If 0 minimum overlap cost is always calculated.
    \param reinserted_elements      Number of elements reinserted by forced reinsertions algorithm.
    */
    dynamic_rstar(size_t max_elements,
                  size_t min_elements,
                  size_t overlap_cost_threshold = 0,
                  size_t reinserted_elements = detail::default_rstar_reinserted_elements_d())
        : m_max_elements(max_elements)
        , m_min_elements(min_elements)
        , m_overlap_cost_threshold(overlap_cost_threshold)
        , m_reinserted_elements(
            detail::default_rstar_reinserted_elements_d() == reinserted_elements ?
            (max_elements * 3) / 10 :
            reinserted_elements
        )
    {}

    size_t get_max_elements() const { return m_max_elements; }
    size_t get_min_elements() const { return m_min_elements; }
    size_t get_overlap_cost_threshold() const { return m_overlap_cost_threshold; }
    size_t get_reinserted_elements() const { return m_reinserted_elements; }

private:
    size_t m_max_elements;
    size_t m_min_elements;
    size_t m_overlap_cost_threshold;
    size_t m_reinserted_elements;
};

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_INDEX_PARAMETERS_HPP
