// Boost.Geometry Index
//
// R-tree implementation
//
// Copyright (c) 2008 Federico J. Fernandez.
// Copyright (c) 2011-2012 Adam Wulkiewicz, Lodz, Poland.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_RTREE_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_RTREE_HPP

#include <algorithm>

#include <boost/tuple/tuple.hpp>
#include <boost/move/move.hpp>

#include <boost/geometry/geometry.hpp>

#include <boost/geometry/extensions/index/assert.hpp>
#include <boost/geometry/extensions/index/nonassignable.hpp>

#include <boost/geometry/extensions/index/translator/translator.hpp>
#include <boost/geometry/extensions/index/rtree/options.hpp>

#include <boost/geometry/extensions/index/predicates.hpp>
#include <boost/geometry/extensions/index/rtree/adaptors.hpp>

#include <boost/geometry/extensions/index/rtree/node/node.hpp>

#include <boost/geometry/extensions/index/algorithms/is_valid.hpp>

#include <boost/geometry/extensions/index/rtree/visitors/insert.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/remove.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/copy.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/destroy.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/spatial_query.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/nearest_query.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/children_box.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/count.hpp>

#include <boost/geometry/extensions/index/rtree/linear/linear.hpp>
#include <boost/geometry/extensions/index/rtree/quadratic/quadratic.hpp>
#include <boost/geometry/extensions/index/rtree/rstar/rstar.hpp>
//#include <boost/geometry/extensions/index/rtree/kmeans/kmeans.hpp>

#include <boost/geometry/extensions/index/inserter.hpp>

// TODO change the name to bounding_tree

/*!
\defgroup rtree_functions Functions related to the rtree (boost::geometry::index::)
*/

namespace boost { namespace geometry { namespace index {

/*!
\brief The R-tree spatial index.

The R-tree spatial index. This is self-balancing spatial index capable to store various types
of Values and balancing algorithms.

\par Parameters
The user must pass a type defining the Parameters which will
be used in rtree creation process. This type is used e.g. to specify balancing algorithm
with specific parameters like min and max number of elements in node.

\par
Predefined algorithms with compile-time parameters are:
\li <tt>boost::geometry::index::linear</tt>,
 \li <tt>boost::geometry::index::quadratic</tt>,
 \li <tt>boost::geometry::index::rstar</tt>.

\par
Predefined algorithms with run-time parameters are:
 \li \c boost::geometry::index::runtime::linear,
 \li \c boost::geometry::index::runtime::quadratic,
 \li \c boost::geometry::index::runtime::rstar.

\par Translator
The Translator translates from Value to Indexable each time r-tree requires it. Which means that this
operation is done for each Value access. Therefore the Translator should return the Indexable by
const reference instead of a value. Default translator can translate all types adapted to Point
or Box concepts (called Indexables). It also handles <tt>std::pair<Indexable, T></tt> and
<tt>boost::tuple<Indexable, ...></tt>. For example, if <tt>std::pair<Box, int></tt> is stored in the
container, the default translator translates from <tt>std::pair<Box, int> const&</tt> to <tt>Box const&</tt>.

\tparam Value       The type of objects stored in the container.
\tparam Parameters  Compile-time parameters.
\tparam Translator  The type of the translator which translates from Value to Indexable.
\tparam Allocator   The allocator used to allocate/deallocate memory, construct/destroy nodes and Values.
*/
template <
    typename Value,
    typename Parameters,
    typename Translator = translator::def<Value>,
    typename Allocator = std::allocator<Value>
>
class rtree
{
    BOOST_COPYABLE_AND_MOVABLE(rtree)

public:
    /*! \brief The type of Value stored in the container. */
    typedef Value value_type;
    /*! \brief R-tree parameters type. */
    typedef Parameters parameters_type;
    /*! \brief Value to Indexable Translator type. */
    typedef Translator translator_type;
    /*! \brief The type of allocator used by the container. */
    typedef Allocator allocator_type;
    /*! \brief Unsigned integral type used by the container. */
    typedef typename allocator_type::size_type size_type;

    /*! \brief The Indexable type to which Value is translated. */
    typedef typename translator::indexable_type<Translator>::type indexable_type;
    /*! \brief The Box type used by the R-tree. */
    typedef typename index::default_box_type<indexable_type>::type box_type;

#if !defined(BOOST_GEOMETRY_INDEX_ENABLE_DEBUG_INTERFACE)
private:
#endif
    typedef typename detail::rtree::options_type<Parameters>::type options_type;
    typedef typename options_type::node_tag node_tag;
    typedef detail::rtree::allocators<allocator_type, value_type, typename options_type::parameters_type, box_type, node_tag> allocators_type;

    typedef typename detail::rtree::node<value_type, typename options_type::parameters_type, box_type, allocators_type, node_tag>::type node;
    typedef typename detail::rtree::internal_node<value_type, typename options_type::parameters_type, box_type, allocators_type, node_tag>::type internal_node;
    typedef typename detail::rtree::leaf<value_type, typename options_type::parameters_type, box_type, allocators_type, node_tag>::type leaf;

public:
    /*!
    \brief The constructor.

    \param parameters   The parameters object.
    \param translator   The translator object.

    \par Throws
    If allocator default constructor throws.
    */
    inline explicit rtree(parameters_type parameters = parameters_type(),
                          translator_type const& translator = translator_type())
        : m_translator(translator)                                          // SHOULDN'T THROW
        , m_parameters(parameters)
        , m_allocators()
        , m_values_count(0)
        , m_leafs_level(0)
        , m_root(0)
    {}

    /*!
    \brief The constructor.

    \param parameters   The parameters object.
    \param translator   The translator object.
    \param allocator    The allocator object.

    \par Throws
    If allocator copy constructor throws.
    */
    inline rtree(parameters_type parameters,
                 translator_type const& translator,
                 allocator_type allocator)
        : m_translator(translator)                                          // SHOULDN'T THROW
        , m_parameters(parameters)
        , m_allocators(allocator)
        , m_values_count(0)
        , m_leafs_level(0)
        , m_root(0)
    {}

    /*!
    \brief The constructor.

    \param first        The beginning of the range of Values.
    \param last         The end of the range of Values.
    \param parameters   The parameters object.
    \param translator   The translator object.
    \param allocator    The allocator object.

    \par Throws
    \li If allocator copy constructor throws.
    \li If Value copy constructor or copy assignment throws.
    \li If allocation throws.
    \li When memory allocation for Node fails.
    */
    template<typename Iterator>
    inline rtree(Iterator first, Iterator last,
                 parameters_type parameters = parameters_type(),
                 translator_type const& translator = translator_type(),
                 allocator_type allocator = allocator_type())
        : m_translator(translator)                                          // SHOULDN'T THROW
        , m_parameters(parameters)
        , m_allocators(allocator)
        , m_values_count(0)
        , m_leafs_level(0)
        , m_root(0)
    {
        try
        {
            this->insert(first, last);
        }
        catch(...)
        {
            this->raw_destroy(*this);
            throw;
        }
    }

    /*!
    \brief The constructor.

    \param rng          The range of Values.
    \param parameters   The parameters object.
    \param translator   The translator object.
    \param allocator    The allocator object.

    \par Throws
    \li If allocator copy constructor throws.
    \li If Value copy constructor or copy assignment throws.
    \li If allocation throws.
    \li When memory allocation for Node fails.
    */
    template<typename Range>
    inline explicit rtree(Range const& rng,
                          parameters_type parameters = parameters_type(),
                          translator_type const& translator = translator_type(),
                          allocator_type allocator = allocator_type())
        : m_translator(translator)                                          // SHOULDN'T THROW
        , m_parameters(parameters)
        , m_allocators(allocator)
        , m_values_count(0)
        , m_leafs_level(0)
        , m_root(0)
    {
        try
        {
            this->insert(rng);
        }
        catch(...)
        {
            this->raw_destroy(*this);
            throw;
        }
    }

    /*!
    \brief The destructor.

    \par Throws
    Nothing.
    */
    inline ~rtree()
    {
        this->raw_destroy(*this);
    }

    /*!
    \brief  The copy constructor.

    It uses parameters, translator and allocator from the source tree.

    \param src          The rtree which content will be copied.

    \par Throws
    \li If allocator copy constructor throws.
    \li If Value copy constructor throws.
    \li If allocation throws.
    \li When memory allocation for Node fails.
    */
    inline rtree(rtree const& src)
        : m_translator(src.m_translator)                                          // SHOULDN'T THROW
        , m_parameters(src.m_parameters)
        , m_allocators(src.m_allocators)
        , m_values_count(0)
        , m_leafs_level(0)
        , m_root(0)
    {
        //TODO use Boost.Container allocator_traits_type::select_on_container_copy_construction()

        this->raw_copy(src, *this, false);
    }

    /*!
    \brief The copy constructor.

    It uses Parameters and translator from the source tree.

    \param src          The rtree which content will be copied.
    \param allocator    The allocator which will be used.

    \par Throws
    \li If allocator copy constructor throws.
    \li If Value copy constructor throws.
    \li If allocation throws.
    \li When memory allocation for Node fails.
    */
    inline rtree(rtree const& src, allocator_type const& allocator)
        : m_translator(src.m_translator)                                          // SHOULDN'T THROW
        , m_parameters(src.m_parameters)
        , m_allocators(allocator)
        , m_values_count(0)
        , m_leafs_level(0)
        , m_root(0)
    {
        this->raw_copy(src, *this, false);
    }

    /*!
    \brief The moving constructor.

    It uses parameters, translator and allocator from the source tree.

    \param src          The rtree which content will be moved.

    \par Throws
    If allocator move constructor throws.
    */
    inline rtree(BOOST_RV_REF(rtree) src)
// TODO - use boost::move()
        : m_translator(src.m_translator)                                    // SHOULDN'T THROW
        , m_parameters(src.m_parameters)
        , m_allocators(src.m_allocators)
        , m_values_count(src.m_values_count)
        , m_leafs_level(src.m_leafs_level)
        , m_root(src.m_root)
    {
        src.m_values_count = 0;
        src.m_leafs_level = 0;
        src.m_root = 0;
    }

    /*!
    \brief The assignment operator.

    It uses parameters and translator from the source tree.

    \param src          The rtree which content will be copied.

    \par Throws
    \li If Value copy constructor throws.
    \li If allocation throws.
    \li When nodes allocation fails.
    */
    inline rtree & operator=(BOOST_COPY_ASSIGN_REF(rtree) src)
    {
        if ( this == &src )
            return *this;

        //TODO use Boost.Container allocator_traits_type::propagate_on_container_move_assignment

        // It uses m_allocators
        this->raw_copy(src, *this, true);

        return *this;
    }

    /*!
    \brief The moving assignment.

    It uses parameters and translator from the source tree.

    \param src          The rtree which content will be moved.

    \par Throws
    Only if allocators aren't equal.
    \li If Value copy constructor throws.
    \li If allocation throws.
    \li When nodes allocation fails.
    */
    inline rtree & operator=(BOOST_RV_REF(rtree) src)
    {
        if ( this == &src )
            return *this;

//TODO use Boost.Container allocator_traits_type::propagate_on_container_move_assignment

        if ( m_allocators.allocator == src.m_allocators.allocator )
        {
// TODO - use boost::move()
            m_translator = src.m_translator;                                          // SHOULDN'T THROW
            m_parameters = src.m_parameters;
            //m_allocators = src.m_allocators;

            m_values_count = src.m_values_count;
            m_leafs_level = src.m_leafs_level;
            m_root = src.m_root;

            src.m_values_count = 0;
            src.m_leafs_level = 0;
            src.m_root = 0;
        }
        else
        {
            // It uses m_allocators
            this->raw_copy(src, *this, true);
        }

        return *this;
    }

    /*!
    \brief Swaps contents of two rtrees.

    Parameters, translator and allocators are swapped as well.

    \param other    The rtree which content will be swapped with this rtree content.

    \par Throws
    If allocators swap throws.
    */
    void swap(rtree & other)
    {
        boost::swap(m_translator, other.m_translator);                               // SHOULDN'T THROW
        boost::swap(m_parameters, other.m_parameters);
        m_allocators.swap(other.m_allocators);

        boost::swap(m_values_count, other.m_values_count);
        boost::swap(m_leafs_level, other.m_leafs_level);
        boost::swap(m_root, other.m_root);
    }

    /*!
    \brief Insert a value to the index.

    \param value    The value which will be stored in the container.

    \par Throws
    \li If Value copy constructor or copy assignment throws.
    \li If allocation throws.
    \li When nodes allocation fails.

    \warning
    This operation is not thread safe. If it throws, the R-tree may be left in an inconsistent state,
    elements must not be inserted or removed, methods may return invalid data.
    */
    inline void insert(value_type const& value)
    {
        if ( !m_root )
            this->raw_create();

        this->raw_insert(value);
    }

    /*!
    \brief Insert a range of values to the index.

    \param first    The beginning of the range of values.
    \param last     The end of the range of values.

    \par Throws
    \li If Value copy constructor or copy assignment throws.
    \li If allocation throws.
    \li When nodes allocation fails.

    \warning
    This operation is not thread safe. If it throws, the R-tree may be left in an inconsistent state,
    elements must not be inserted or removed, methods may return invalid data.
    */
    template <typename Iterator>
    inline void insert(Iterator first, Iterator last)
    {
        if ( !m_root )
            this->raw_create();

        for ( ; first != last ; ++first )
            this->raw_insert(*first);
    }

    /*!
    \brief Insert a range of values to the index.

    \param rng      The range of values.

    \par Throws
    \li If Value copy constructor or copy assignment throws.
    \li If allocation throws.
    \li When nodes allocation fails.

    \warning
    This operation is not thread safe. If it throws, the R-tree may be left in an inconsistent state,
    elements must not be inserted or removed, methods may return invalid data.
    */
    template <typename Range>
    inline void insert(Range const& rng)
    {
        if ( !m_root )
            this->raw_create();

        typedef typename boost::range_const_iterator<Range>::type It;
        for ( It it = boost::const_begin(rng); it != boost::const_end(rng) ; ++it )
            this->raw_insert(*it);
    }

    /*!
    \brief Remove a value from the container.

    In contrast to the \c std::set or \c std::map \c erase() method
    this method removes only one value from the container.

    \param value    The value which will be removed from the container.

    \return         1 if the value was removed, 0 otherwise.

    \par Throws
    \li If Value copy constructor or copy assignment throws.
    \li If allocation throws.
    \li When nodes allocation fails.

    \warning
    This operation is not thread safe. If it throws, the R-tree may be left in an inconsistent state,
    elements must not be inserted or removed, methods may return invalid data.
    */
    inline size_type remove(value_type const& value)
    {
        return this->raw_remove(value);
    }

    /*!
    \brief Remove a range of values from the container.

    In contrast to the \c std::set or \c std::map \c erase() method
    it doesn't take iterators pointing to values stored in this container. It removes values equal
    to these passed as a range. Furthermore this method removes only one value for each one passed
    in the range, not all equal values.

    \param first    The beginning of the range of values.
    \param last     The end of the range of values.

    \return         The number of removed values.

    \par Throws
    \li If Value copy constructor or copy assignment throws.
    \li If allocation throws.
    \li When nodes allocation fails.

    \warning
    This operation is not thread safe. If it throws, the R-tree may be left in an inconsistent state,
    elements must not be inserted or removed, methods may return invalid data.
    */
    template <typename Iterator>
    inline size_type remove(Iterator first, Iterator last)
    {
        size_type result = 0;
        for ( ; first != last ; ++first )
            result += this->raw_remove(*first);
        return result;
    }

    /*!
    \brief Remove a range of values from the container.

    In contrast to the \c std::set or \c std::map \c erase() method
    it removes values equal to these passed as a range. Furthermore, this method removes only
    one value for each one passed in the range, not all equal values.

    \param rng      The range of values.

    \return         The number of removed values.

    \par Throws
    \li If Value copy constructor or copy assignment throws.
    \li If allocation throws.
    \li When nodes allocation fails.

    \warning
    This operation is not thread safe. If it throws, the R-tree may be left in an inconsistent state,
    elements must not be inserted or removed, methods may return invalid data.
    */
    template <typename Range>
    inline size_type remove(Range const& rng)
    {
        size_type result = 0;
        typedef typename boost::range_const_iterator<Range>::type It;
        for ( It it = boost::const_begin(rng); it != boost::const_end(rng) ; ++it )
            result += this->raw_remove(*it);
        return result;
    }

    /*!
    \brief Finds values meeting spatial predicates, e.g. intersecting some Box.

    <b>Spatial predicates</b> may be a \c Geometry. In this case Values intersecting the \c Geometry are returned.

    It may be generated by one of the functions listed below:
    \li \c boost::geometry::index::covered_by(),
    \li \c boost::geometry::index::disjoint(),
    \li \c boost::geometry::index::intersects() - default,
    \li \c boost::geometry::index::overlaps(),
    \li \c boost::geometry::index::within(),
    \li <tt>! boost::geometry::index::covered_by()</tt>,
    \li <tt>! boost::geometry::index::disjoint()</tt>,
    \li <tt>! boost::geometry::index::intersects()</tt>,
    \li <tt>! boost::geometry::index::overlaps()</tt>,
    \li <tt>! boost::geometry::index::within()</tt>
    \li \c boost::geometry::index::value().

    Those predicates may be passed together in
    \c std::pair or \c boost::tuple.

    \param pred     The spatial predicates or a Geometry.
    \param out_it   The output iterator of the result range. E.g. an iterator generated by
                    std::back_inserter(container)

    \return         The number of values found.

    \par Throws
    \li If Value copy constructor or copy assignment throws.
    \li If OutIter dereference or increment throws.
    */
    template <typename Predicates, typename OutIter>
    inline size_type spatial_query(Predicates const& pred, OutIter out_it) const
    {
        if ( !m_root )
            return 0;

        detail::rtree::visitors::spatial_query<value_type, options_type, translator_type, box_type, allocators_type, Predicates, OutIter>
            find_v(m_translator, pred, out_it);

        detail::rtree::apply_visitor(find_v, *m_root);

        return find_v.found_count;
    }

    /*!
    \brief Finds one value meeting distances predicates, e.g. nearest to some Point.

    <b>Distances predicates</b> may be a \c Point. In this the case the Value closest to \c Point is returned.

    It is possible to define how distance to Value is calculated. This is done by passing PointRelation.
    It can be generated by following functions:
    \li boost::geometry::index::to_nearest() - default,
    \li boost::geometry::index::to_centroid(),
    \li boost::geometry::index::to_furthest().

    It is possible to define define distances bounds, for example that some distance must be between
    min_distance and max_distance. This is done by passing DistancesPredicates which can be generated
    by following functions:
    \li boost::geometry::index::unbounded() - default,
    \li boost::geometry::index::min_bounded(),
    \li boost::geometry::index::max_bounded(),
    \li boost::geometry::index::bounded().

    MinRelation and MaxRelation describes bounds and can be generated by following functions:
    \li boost::geometry::index::to_nearest(),
    \li boost::geometry::index::to_centroid(),
    \li boost::geometry::index::to_furthest().

    \param dpred    The distances predicates or a Point.

    \param v        The reference to the object which will contain the result.

    \return         The number of values found.

    \par Throws
    If Value copy constructor or copy assignment throws.
    */
    template <typename DistancesPredicates>
    inline size_type nearest_query(DistancesPredicates const& dpred, value_type & v) const
    {
        return raw_nearest_one(dpred, detail::empty(), v);
    }

    /*!
    \brief Finds one value meeting distances predicates and spatial predicates,
    e.g. nearest to some Point and intersecting some Box.

    <b>Distances predicates</b> may be a \c Point. In this the case the Value closest to \c Point is returned.

    It is possible to define how distance to Value is calculated. This is done by passing PointRelation.
    It can be generated by following functions:
    \li boost::geometry::index::to_nearest() - default,
    \li boost::geometry::index::to_centroid(),
    \li boost::geometry::index::to_furthest().

    It is possible to define define distances bounds, for example that some distance must be between
    min_distance and max_distance. This is done by passing DistancesPredicates which can be generated
    by following functions:
    \li boost::geometry::index::unbounded() - default,
    \li boost::geometry::index::min_bounded(),
    \li boost::geometry::index::max_bounded(),
    \li boost::geometry::index::bounded().

    MinRelation and MaxRelation describes bounds and can be generated by following functions:
    \li boost::geometry::index::to_nearest(),
    \li boost::geometry::index::to_centroid(),
    \li boost::geometry::index::to_furthest().

    <b>Spatial predicates</b> may be a \c Geometry. In this case Values intersecting the \c Geometry are returned.

    It may be generated by one of the functions listed below:
    \li \c boost::geometry::index::covered_by(),
    \li \c boost::geometry::index::disjoint(),
    \li \c boost::geometry::index::intersects() - default,
    \li \c boost::geometry::index::overlaps(),
    \li \c boost::geometry::index::within(),
    \li <tt>! boost::geometry::index::covered_by()</tt>,
    \li <tt>! boost::geometry::index::disjoint()</tt>,
    \li <tt>! boost::geometry::index::intersects()</tt>,
    \li <tt>! boost::geometry::index::overlaps()</tt>,
    \li <tt>! boost::geometry::index::within()</tt>
    \li \c boost::geometry::index::value().

    Those predicates may be passed together in
    \c std::pair or \c boost::tuple.

    \param dpred    The distances predicates or a Point.
    \param pred     The spatial predicates or a Geometry
    \param v        The reference to the object which will contain the result.

    \return         The number of values found.

    \par Throws
    If Value copy constructor or copy assignment throws.
    */
    template <typename DistancesPredicates, typename Predicates>
    inline size_type nearest_query(DistancesPredicates const& dpred, Predicates const& pred, value_type & v) const
    {
        return raw_nearest_one(dpred, pred, v);
    }

    /*!
    \brief Finds k values meeting distances predicates, e.g. k nearest values to some Point.

    <b>Distances predicates</b> may be a \c Point. In this the case the Value closest to \c Point is returned.

    It is possible to define how distance to Value is calculated. This is done by passing PointRelation.
    It can be generated by following functions:
    \li boost::geometry::index::to_nearest() - default,
    \li boost::geometry::index::to_centroid(),
    \li boost::geometry::index::to_furthest().

    It is possible to define define distances bounds, for example that some distance must be between
    min_distance and max_distance. This is done by passing DistancesPredicates which can be generated
    by following functions:
    \li boost::geometry::index::unbounded() - default,
    \li boost::geometry::index::min_bounded(),
    \li boost::geometry::index::max_bounded(),
    \li boost::geometry::index::bounded().

    MinRelation and MaxRelation describes bounds and can be generated by following functions:
    \li boost::geometry::index::to_nearest(),
    \li boost::geometry::index::to_centroid(),
    \li boost::geometry::index::to_furthest().

    \param dpred    The distances predicates or a Point.
    \param k        The max number of values.
    \param out_it   The output iterator of the result range. E.g. a back_insert_iterator.

    \return         The number of values found.

    \par Throws
    If Value copy constructor or copy assignment throws. If OutIter dereference or increment throws.
    */
    template <typename DistancesPredicates, typename OutIter>
    inline size_type nearest_query(DistancesPredicates const& dpred, size_t k, OutIter out_it) const
    {
        return raw_nearest_k(dpred, k, detail::empty(), out_it);
    }

    /*!
    \brief Finds k values meeting distances predicates and spatial predicates,
    e.g. k nearest values to some Point and intersecting some Box.

    <b>Distances predicates</b> may be a \c Point. In this the case the Value closest to \c Point is returned.

    It is possible to define how distance to Value is calculated. This is done by passing PointRelation.
    It can be generated by following functions:
    \li boost::geometry::index::to_nearest() - default,
    \li boost::geometry::index::to_centroid(),
    \li boost::geometry::index::to_furthest().

    It is possible to define define distances bounds, for example that some distance must be between
    min_distance and max_distance. This is done by passing DistancesPredicates which can be generated
    by following functions:
    \li boost::geometry::index::unbounded() - default,
    \li boost::geometry::index::min_bounded(),
    \li boost::geometry::index::max_bounded(),
    \li boost::geometry::index::bounded().

    MinRelation and MaxRelation describes bounds and can be generated by following functions:
    \li boost::geometry::index::to_nearest(),
    \li boost::geometry::index::to_centroid(),
    \li boost::geometry::index::to_furthest().

    <b>Spatial predicates</b> may be a \c Geometry. In this case Values intersecting the \c Geometry are returned.

    It may be generated by one of the functions listed below:
    \li \c boost::geometry::index::covered_by(),
    \li \c boost::geometry::index::disjoint(),
    \li \c boost::geometry::index::intersects() - default,
    \li \c boost::geometry::index::overlaps(),
    \li \c boost::geometry::index::within(),
    \li <tt>! boost::geometry::index::covered_by()</tt>,
    \li <tt>! boost::geometry::index::disjoint()</tt>,
    \li <tt>! boost::geometry::index::intersects()</tt>,
    \li <tt>! boost::geometry::index::overlaps()</tt>,
    \li <tt>! boost::geometry::index::within()</tt>
    \li \c boost::geometry::index::value().

    Those predicates may be passed together in
    \c std::pair or \c boost::tuple.

    \param dpred    The distances predicates or a Point
    \param k        The max number of values.
    \param pred     The spatial predicates or a Geometry.
    \param out_it   The output iterator of the result range. E.g. a back_insert_iterator.

    \return         The number of values found.

    \par Throws
    If Value copy constructor or copy assignment throws. If OutIter dereference or increment throws.
    */
    template <typename DistancesPredicates, typename Predicates, typename OutIter>
    inline size_type nearest_query(DistancesPredicates const& dpred, size_t k, Predicates const& pred, OutIter out_it) const
    {
        return raw_nearest_k(dpred, k, pred, out_it);
    }

    /*!
    \brief Returns the number of stored values.

    \return         The number of stored values.

    \par Throws
    Nothing.
    */
    inline size_type size() const
    {
        return m_values_count;
    }

    /*!
    \brief Query if the container is empty.

    \return         true if the container is empty.

    \par Throws
    Nothing.
    */
    inline bool empty() const
    {
        return 0 == m_values_count;
    }

    /*!
    \brief Removes all values stored in the container.

    \par Throws
    Nothing.
    */
    inline void clear()
    {
        this->raw_destroy(*this);
    }

    /*!
    \brief Returns the box containing all values stored in the container.

    Returns the box containing all values stored in the container.
    If the container is empty the result of \c geometry::assign_inverse() is returned.

    \return     The box containing all values stored in the container or an invalid box if
                there are no values in the container.

    \par Throws
    Nothing.
    */
    inline box_type box() const
    {
        if ( this->empty() )
        {
            box_type result;
            geometry::assign_inverse(result);
            return result;
        }

        detail::rtree::visitors::children_box<value_type, options_type, translator_type, box_type, allocators_type>
            children_box_v(m_translator);

        detail::rtree::apply_visitor(children_box_v, *m_root);

        return children_box_v.result;
    }

    /*!
    \brief Count Values or Indexables stored in the container.
    
    For indexable_type it returns the number of values which indexables equals the parameter.
    For value_type it returns the number of values which equals the parameter.

    \param vori The value or indexable which will be counted.

    \return     The number of values found.

    \par Throws
    Nothing.
    */
    template <typename ValueOrIndexable>
    size_type count(ValueOrIndexable const& vori) const
    {
        if ( !m_root )
            return 0;

        detail::rtree::visitors::count<ValueOrIndexable, value_type, options_type, translator_type, box_type, allocators_type>
            count_v(vori, m_translator);

        detail::rtree::apply_visitor(count_v, *m_root);

        return count_v.found_count;
    }

    /*!
    \brief Returns parameters.

    \return     The parameters object.

    \par Throws
    Nothing.
    */
    inline parameters_type const& parameters() const
    {
        return m_parameters;
    }

    /*!
    \brief Returns the translator object.

    \return     The translator object.

    \par Throws
    Nothing.
    */
    inline translator_type const& translator() const
    {
        return m_translator;
    }

    /*!
    \brief Returns allocator used by the rtree.

    \return     The allocator.

    \par Throws
    If allocator copy constructor throws.
    */
    allocator_type get_allocator() const
    {
        return m_allocators.allocator;
    }

#if !defined(BOOST_GEOMETRY_INDEX_ENABLE_DEBUG_INTERFACE)
private:
#endif
    /*!
    \brief Apply a visitor to the nodes structure in order to perform some operator.

    This function is not a part of the 'official' interface. However it makes
    possible e.g. to pass a visitor drawing the tree structure.

    \param visitor  The visitor object.

    \par Throws
    If Visitor::operator() throws.
    */
    template <typename Visitor>
    inline void apply_visitor(Visitor & visitor) const
    {
        if ( m_root )
            detail::rtree::apply_visitor(visitor, *m_root);
    }

    /*!
    \brief Returns the number of stored objects. Same as size().

    This function is not a part of the 'official' interface.

    \return     The number of stored objects.

    \par Throws
    Nothing.
    */
    inline size_type values_count() const
    {
        return m_values_count;
    }

    /*!
    \brief Returns the depth of the R-tree.

    This function is not a part of the 'official' interface.

    \return     The depth of the R-tree.

    \par Throws
    Nothing.
    */
    inline size_type depth() const
    {
        return m_leafs_level;
    }

private:
    /*!
    \pre Root node must exist - m_root != 0.

    \brief Insert a value to the index.

    \param value    The value which will be stored in the container.

    \par Exception-safety
    basic
    */
    inline void raw_insert(value_type const& value)
    {
        BOOST_GEOMETRY_INDEX_ASSERT(m_root, "The root must exist");
        BOOST_GEOMETRY_INDEX_ASSERT(index::is_valid(m_translator(value)), "Indexable is invalid");

        detail::rtree::visitors::insert<
            value_type,
            value_type, options_type, translator_type, box_type, allocators_type,
            typename options_type::insert_tag
        > insert_v(m_root, m_leafs_level, value, m_parameters, m_translator, m_allocators);

        detail::rtree::apply_visitor(insert_v, *m_root);

// TODO
// Think about this: If exception is thrown, may the root be removed?
// Or it is just cleared?

// TODO
// If exception is thrown, m_values_count may be invalid
        ++m_values_count;
    }

    /*!
    \brief Remove the value from the container.

    \param value    The value which will be removed from the container.

    \par Exception-safety
    basic
    */
    inline size_type raw_remove(value_type const& value)
    {
        // TODO: awulkiew - assert for correct value (indexable) ?
        BOOST_GEOMETRY_INDEX_ASSERT(m_root, "The root must exist");
        BOOST_GEOMETRY_INDEX_ASSERT(0 < m_values_count, "can't remove, there are no elements in the rtree");

        detail::rtree::visitors::remove<
            value_type, options_type, translator_type, box_type, allocators_type
        > remove_v(m_root, m_leafs_level, value, m_parameters, m_translator, m_allocators);

        detail::rtree::apply_visitor(remove_v, *m_root);

// TODO
// Think about this: If exception is thrown, may the root be removed?
// Or it is just cleared?

// TODO
// If exception is thrown, m_values_count may be invalid
        --m_values_count;

        return remove_v.is_value_removed() ? 1 : 0;
    }

    /*!
    \brief Create an empty R-tree i.e. new empty root node and clear other attributes.

    \par Exception-safety
    strong
    */
    inline void raw_create()
    {
        BOOST_GEOMETRY_INDEX_ASSERT(0 == m_root, "the tree is already created");

        m_root = detail::rtree::create_node<allocators_type, leaf>::apply(m_allocators);                            // MAY THROW (N: alloc)
        m_values_count = 0;
        m_leafs_level = 0;
    }

    /*!
    \brief Destroy the R-tree i.e. all nodes and clear attributes.

    \param t    The container which is going to be destroyed.

    \par Exception-safety
    nothrow
    */
    inline void raw_destroy(rtree & t)
    {
        if ( t.m_root )
        {
            detail::rtree::visitors::destroy<value_type, options_type, translator_type, box_type, allocators_type> del_v(t.m_root, t.m_allocators);
            detail::rtree::apply_visitor(del_v, *t.m_root);

            t.m_root = 0;
        }
        t.m_values_count = 0;
        t.m_leafs_level = 0;
    }

    /*!
    \brief Copy the R-tree i.e. whole nodes structure, values and other attributes.
    It uses destination's allocators to create the new structure.

    \param src    The source R-tree.
    \param dst    The destination R-tree.

    \par Exception-safety
    strong
    */
    inline void raw_copy(rtree const& src, rtree & dst, bool copy_all_internals) const
    {
        detail::rtree::visitors::copy<value_type, options_type, translator_type, box_type, allocators_type> copy_v(dst.m_allocators);

        if ( src.m_root )
            detail::rtree::apply_visitor(copy_v, *src.m_root);                              // MAY THROW (V, E: alloc, copy, N: alloc)

        if ( copy_all_internals )
        {
            dst.m_translator = src.m_translator;                                            // SHOULDN'T THROW

            dst.m_parameters = src.m_parameters;
            //dst.m_allocators = dst.m_allocators;
        }

        if ( dst.m_root )
        {
            detail::rtree::visitors::destroy<value_type, options_type, translator_type, box_type, allocators_type> del_v(dst.m_root, dst.m_allocators);
            detail::rtree::apply_visitor(del_v, *dst.m_root);
            dst.m_root = 0;
        }

        dst.m_root = copy_v.result;
        dst.m_values_count = src.m_values_count;
        dst.m_leafs_level = src.m_leafs_level;
    }

    /*!
    \brief Find one value meeting distances and spatial predicates.

    \par Exception-safety
    strong
    */
    template <typename DistancesPredicates, typename Predicates>
    inline size_type raw_nearest_one(DistancesPredicates const& dpred, Predicates const& pred, value_type & v) const
    {
        if ( !m_root )
            return 0;

        typedef typename detail::point_relation<DistancesPredicates>::type point_relation;
        typedef typename detail::relation<point_relation>::value_type point_type;

        typedef detail::rtree::visitors::nearest_query_result_one<
            value_type,
            translator_type,
            point_type
        > result_type;

        result_type result(v);

        detail::rtree::visitors::nearest_query<
            value_type,
            options_type,
            translator_type,
            box_type,
            allocators_type,
            DistancesPredicates,
            Predicates,
            result_type
        > nearest_v(m_parameters, m_translator, dpred, pred, result);

        detail::rtree::apply_visitor(nearest_v, *m_root);

        return result.finish();
    }

    /*!
    \brief Find k values meeting distances and spatial predicates.

    \par Exception-safety
    strong
    */
    template <typename DistancesPredicates, typename Predicates, typename OutIter>
    inline size_type raw_nearest_k(DistancesPredicates const& dpred, size_t k, Predicates const& pred, OutIter out_it) const
    {
        if ( !m_root )
            return 0;

        typedef typename detail::point_relation<DistancesPredicates>::type point_relation;
        typedef typename detail::relation<point_relation>::value_type point_type;

        typedef detail::rtree::visitors::nearest_query_result_k<
            value_type,
            translator_type,
            point_type,
            OutIter
        > result_type;

        result_type result(k, out_it);

        detail::rtree::visitors::nearest_query<
            value_type,
            options_type,
            translator_type,
            box_type,
            allocators_type,
            DistancesPredicates,
            Predicates,
            result_type
        > nearest_v(m_parameters, m_translator, dpred, pred, result);

        detail::rtree::apply_visitor(nearest_v, *m_root);

        return result.finish();
    }

    translator_type m_translator;
    Parameters m_parameters;
    allocators_type m_allocators;

    size_type m_values_count;
    size_type m_leafs_level;
    node * m_root;
};

/*!
\brief Insert a value to the index.

\ingroup rtree_functions

\param tree The spatial index.
\param v    The value which will be stored in the index.
*/
template <typename Value, typename Options, typename Translator, typename Allocator>
inline void insert(rtree<Value, Options, Translator, Allocator> & tree, Value const& v)
{
    tree.insert(v);
}

/*!
\brief Insert a range of values to the index.

\ingroup rtree_functions

\param tree     The spatial index.
\param first    The beginning of the range of values.
\param last     The end of the range of values.
*/
template<typename Value, typename Options, typename Translator, typename Allocator, typename Iterator>
inline void insert(rtree<Value, Options, Translator, Allocator> & tree, Iterator first, Iterator last)
{
    tree.insert(first, last);
}

/*!
\brief Insert a range of values to the index.

\ingroup rtree_functions

\param tree     The spatial index.
\param rng      The range of values.
*/
template<typename Value, typename Options, typename Translator, typename Allocator, typename Range>
inline void insert(rtree<Value, Options, Translator, Allocator> & tree, Range const& rng)
{
    tree.insert(rng);
}

/*!
\brief Remove a value from the container.

Remove a value from the container. In contrast to the \c std::set or \c std::map \c erase() method
this function removes only one value from the container.

\ingroup rtree_functions

\param tree The spatial index.
\param v    The value which will be removed from the index.

\return     1 if value was removed, 0 otherwise.
*/
template <typename Value, typename Options, typename Translator, typename Allocator>
inline typename rtree<Value, Options, Translator, Allocator>::size_type
remove(rtree<Value, Options, Translator, Allocator> & tree, Value const& v)
{
    return tree.remove(v);
}

/*!
\brief Remove a range of values from the container.

Remove a range of values from the container. In contrast to the \c std::set or \c std::map \c erase() method
it doesn't take iterators pointing to values stored in this container. It removes values equal
to these passed as a range. Furthermore this function removes only one value for each one passed
in the range, not all equal values.

\ingroup rtree_functions

\param tree     The spatial index.
\param first    The beginning of the range of values.
\param last     The end of the range of values.

\return         The number of removed values.
*/
template<typename Value, typename Options, typename Translator, typename Allocator, typename Iterator>
inline typename rtree<Value, Options, Translator, Allocator>::size_type
remove(rtree<Value, Options, Translator, Allocator> & tree, Iterator first, Iterator last)
{
    return tree.remove(first, last);
}

/*!
\brief Remove a range of values from the container.

Remove a range of values from the container. In contrast to the \c std::set or \c std::map \c erase() method
it removes values equal to these passed as a range. Furthermore this method removes only
one value for each one passed in the range, not all equal values.

\ingroup rtree_functions

\param tree     The spatial index.
\param rng      The range of values.

\return         The number of removed values.
*/
template<typename Value, typename Options, typename Translator, typename Allocator, typename Range>
inline typename rtree<Value, Options, Translator, Allocator>::size_type
remove(rtree<Value, Options, Translator, Allocator> & tree, Range const& rng)
{
    return tree.remove(rng);
}

/*!
\brief Find values meeting spatial predicates.

\ingroup rtree_functions

\param tree     The spatial index.
\param pred     The spatial predicates.
\param out_it   The output iterator of the result range.

\return         The number of found values.
*/
template <typename Value, typename Options, typename Translator, typename Allocator, typename Predicates, typename OutIter>
inline size_t spatial_query(rtree<Value, Options, Translator, Allocator> const& tree, Predicates const& pred, OutIter out_it)
{
    return tree.spatial_query(pred, out_it);
}

/*!
\brief Find the value meeting distances predicates.

\ingroup rtree_functions

\param tree     The spatial index.
\param dpred    The distances predicates.
\param v        The result.

\return         The number of found values.
*/
template <typename Value, typename Options, typename Translator, typename Allocator, typename DistancesPredicates>
inline size_t nearest_query(rtree<Value, Options, Translator, Allocator> const& tree, DistancesPredicates const& dpred, Value & v)
{
    return tree.nearest_query(dpred, v);
}

/*!
\brief Find the value meeting distances and spatial predicates.

\ingroup rtree_functions

\param tree     The spatial index.
\param dpred    The distances predicates.
\param pred     The spatial predicates.
\param v        The result.

\return         The number of found values.
*/
template <typename Value, typename Options, typename Translator, typename Allocator, typename DistancesPredicates, typename Predicates>
inline size_t nearest_query(rtree<Value, Options, Translator, Allocator> const& tree, DistancesPredicates const& dpred, Predicates const& pred, Value & v)
{
    return tree.nearest_query(dpred, pred, v);
}

/*!
\brief Find k values meeting distances predicates.

\ingroup rtree_functions

\param tree     The spatial index.
\param dpred    The distances predicates.
\param k        The max number of values.
\param out_it   The output iterator of the result range.

\return         The number of found values.
*/
template <typename Value, typename Options, typename Translator, typename Allocator, typename DistancesPredicates, typename OutIter>
inline size_t nearest_query(rtree<Value, Options, Translator, Allocator> const& tree, DistancesPredicates const& dpred, size_t k, OutIter out_it)
{
    return tree.nearest_query(dpred, k, out_it);
}

/*!
\brief Find k values meeting distances and spatial predicates.

\ingroup rtree_functions

\param tree     The spatial index.
\param dpred    The distances predicates.
\param k        The max number of values.
\param pred     The spatial predicates.
\param out_it   The output iterator of the result range.

\return         The number of found values.
*/
template <typename Value, typename Options, typename Translator, typename Allocator, typename DistancesPredicates, typename Predicates, typename OutIter>
inline size_t nearest_query(rtree<Value, Options, Translator, Allocator> const& tree, DistancesPredicates const& dpred, size_t k, Predicates const& pred, OutIter out_it)
{
    return tree.nearest_query(dpred, k, pred, out_it);
}

/*!
\brief Remove all values from the index.

\ingroup rtree_functions

\param tree     The spatial index.
*/
template <typename Value, typename Options, typename Translator, typename Allocator>
inline void clear(rtree<Value, Options, Translator, Allocator> & tree)
{
    return tree.clear();
}

/*!
\brief Get the number of values stored in the index.

\ingroup rtree_functions

\param tree     The spatial index.

\return         The number of values stored in the index.
*/
template <typename Value, typename Options, typename Translator, typename Allocator>
inline size_t size(rtree<Value, Options, Translator, Allocator> const& tree)
{
    return tree.size();
}

/*!
\brief Query if there are no values stored in the index.

\ingroup rtree_functions

\param tree     The spatial index.

\return         true if there are no values in the index.
*/
template <typename Value, typename Options, typename Translator, typename Allocator>
inline bool empty(rtree<Value, Options, Translator, Allocator> const& tree)
{
    return tree.empty();
}

/*!
\brief Get the box containing all stored values or an invalid box if the index has no values.

\ingroup rtree_functions

\param tree     The spatial index.

\return         The box containing all stored values or an invalid box.
*/
template <typename Value, typename Options, typename Translator, typename Allocator>
inline typename rtree<Value, Options, Translator, Allocator>::box_type
box(rtree<Value, Options, Translator, Allocator> const& tree)
{
    return tree.box();
}

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_RTREE_HPP
