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

#include <boost/geometry/extensions/index/rtree/linear/linear.hpp>
#include <boost/geometry/extensions/index/rtree/quadratic/quadratic.hpp>
#include <boost/geometry/extensions/index/rtree/rstar/rstar.hpp>
//#include <boost/geometry/extensions/index/rtree/kmeans/kmeans.hpp>

#include <boost/geometry/extensions/index/inserter.hpp>

// TODO change the name to bounding_tree

namespace boost { namespace geometry { namespace index {

/*!
The R-tree spatial index. This is self-balancing spatial index capable to store various types
of Values and balancing algorithms.

\tparam Value       The type of objects stored in the container.
\tparam Parameters  Compile-time parameters. The user must pass a type defining the Parameters which will
                    be used in rtree creation process. This type is used e.g. to specify balancing algorithm
                    with compile-time parameters like min and max number of elements in node.
                    Predefined Algorithms/Parameters are:
                    bgi::linear<MinElements, MaxElements>,
                    bgi::quadratic<MinElements, MaxElements>,
                    bgi::rstar<MinElements, MaxElements, OverlapCostThreshold = 0, ReinsertedElements = MaxElements * 0.3>.
\tparam Translator  The type of the translator which translates from Value to Indexable. This translation is done each time
                    the r-tree wants to know Value's Indexable. Default translator can translate all types adapted to Point
                    or Box concepts (which are Indexables). It also handles std::pair<Indexable, T>, pointers, smart pointers,
                    and iterators. E.g. If std::pair<Box, int> is stored, the default translator translates from
                    std::pair<Box, int> const& to Box const&.
\tparam Allocator   The allocator.
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
    typedef Value value_type;
    typedef Parameters parameters_type;
    typedef Translator translator_type;
    typedef Allocator allocator_type;
    typedef typename allocator_type::size_type size_type;

    typedef typename translator::indexable_type<Translator>::type indexable_type;
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
    The constructor.

    \note Exception-safety: nothrow

    \param parameters   The parameters object.
    \param translator   The translator object.
    \param allocator    The allocator object.
    */
    inline explicit rtree(Parameters parameters = Parameters(), translator_type const& translator = translator_type(), Allocator allocator = Allocator())
        : m_translator(translator)                                          // SHOULDN'T THROW
        , m_parameters(parameters)
        , m_allocators(allocator)
        , m_values_count(0)
        , m_leafs_level(0)
        , m_root(0)
    {}

    /*!
    The constructor.

    \note Exception-safety: strong

    \param first        The beginning of the range of Values.
    \param last         The end of the range of Values.
    \param parameters   The parameters object.
    \param translator   The translator object.
    \param allocator    The allocator object.
    */
    template<typename Iterator>
    inline rtree(Iterator first, Iterator last, Parameters parameters = Parameters(), translator_type const& translator = translator_type(), Allocator allocator = std::allocator<value_type>())
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
            this->raw_destroy(*this, true);
            throw;
        }
    }

    /*!
    The constructor.

    \note Exception-safety: strong

    \param rng          The range of Values.
    \param parameters   The parameters object.
    \param translator   The translator object.
    \param allocator    The allocator object.
    */
    template<typename Range>
    inline rtree(Range const& rng, Parameters parameters = Parameters(), translator_type const& translator = translator_type(), Allocator allocator = std::allocator<value_type>())
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
            this->raw_destroy(*this, true);
            throw;
        }
    }

    /*!
    The destructor.

    \note Exception-safety: nothrow
    */
    inline ~rtree()
    {
        this->raw_destroy(*this, true);
    }

    /*!
    The copy constructor.

    \note Exception-safety: strong
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
    The copy constructor.

    \note Exception-safety: strong
    */
    inline rtree(rtree const& src, Allocator const& allocator)
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
    The moving constructor.

    \note Exception-safety: nothrow
    */
    inline rtree(BOOST_RV_REF(rtree) src)
        : m_translator(src.m_translator)                                          // SHOULDN'T THROW
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
    The assignment operator.

    \note Exception-safety: strong
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
    The moving assignment.

    \note Exception-safety: nothrow (if allocators are equal),
                            strong (if allocators aren't equal)
    */
    inline rtree & operator=(BOOST_RV_REF(rtree) src)
    {
        if ( this == &src )
            return *this;

        //TODO use Boost.Container allocator_traits_type::propagate_on_container_move_assignment

        if ( m_allocators.allocator == src.m_allocators.allocator )
        {
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
    Swaps two rtrees.

    \note Exception-safety: nothrow

    \param other    The other rtree.
    */
    void swap(rtree & other)
    {
        std::swap(m_translator, other.m_translator);                               // SHOULDN'T THROW
        std::swap(m_parameters, other.m_parameters);
        std::swap(m_allocators, other.m_allocators);

        std::swap(m_values_count, other.m_values_count);
        std::swap(m_leafs_level, other.m_leafs_level);
        std::swap(m_root, other.m_root);
    }

    /*!
    Insert a value to the index.

    \note Exception-safety: basic

    \param value    The value which will be stored in the container.
    */
    inline void insert(value_type const& value)
    {
        if ( !m_root )
            this->raw_create();

        this->raw_insert(value);
    }

    /*!
    Insert a range of values to the index.

    \note Exception-safety: basic

    \param first    The beginning of the range of values.
    \param last     The end of the range of values.
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
    Insert a range of values to the index.

    \note Exception-safety: basic

    \param rng      The range of values.
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
    Remove a value from the container.

    \note Exception-safety: basic

    \param value    The value which will be removed from the container.
    */
    inline void remove(value_type const& value)
    {
        this->raw_remove(value);
    }

    /*!
    Remove a range of values from the container.

    \note Exception-safety: basic

    \param first    The beginning of the range of values.
    \param last     The end of the range of values.
    */
    template <typename Iterator>
    inline void remove(Iterator first, Iterator last)
    {
        for ( ; first != last ; ++first )
            this->raw_remove(*first);
    }

    /*!
    Remove a range of values from the container.

    \note Exception-safety: basic

    \param rng      The range of values.
    */
    template <typename Range>
    inline void remove(Range const& rng)
    {
        typedef typename boost::range_const_iterator<Range>::type It;
        for ( It it = boost::const_begin(rng); it != boost::const_end(rng) ; ++it )
            this->raw_remove(*it);
    }

    /*!
    Find values meeting spatial predicates, e.g. intersecting some box.

    \note Exception-safety: strong

    \param pred     The spatial predicates. May be a Geometry (in this case default
                    predicate - intersects is used) or generated by bgi::covered_by(geometry),
                    bgi::disjoint(geometry), bgi::intersects(geometry), bgi::overlaps(geometry),
                    bgi::within(geometry), !bgi::covered_by(geometry), !bgi::disjoint(geometry),
                    !bgi::intersects(geometry), !bgi::overlaps(geometry), !bgi::within(geometry)
                    or bgi::value(func). Those predicates may be passed together in std::pair
                    or boost::tuple.
                    
    \param out_it   The output iterator of the result range. E.g. an iterator generated by
                    std::back_inserter(container)

    \return         The number of values found.
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
    Find one value meeting distances predicates, e.g. nearest to some point.

    \note Exception-safety: strong

    \param dpred    The distances predicates. May be a Point. This is default case where Value which
                    nearest point is closest to Point is returned. May be a PointRelation which define
                    how distance to Value is calculated. This may be generated by bgi::to_nearest(Point),
                    bgi::to_centroid(Point) or bgi::to_furthest(Point). DistancesPredicates may also define distances
                    bounds. E.g. that some distance must be between min_distance and max_distance. This may
                    be generated by bgi::unbounded(PointRelation) - default case, bgi::min_bounded(PointRelation, MinRelation),
                    bgi::max_bounded(PointRelation, MaxRelation), bgi::bounded(PointRelation, MinRelation, MaxRelation).
                    MinRelation and MaxRelation describes bounds and may be generated by bgi::to_nearest(dist_bound),
                    bgi::to_centroid(dist_bound) or bgi::to_furthest(dist_bound).

    \param v        The reference to the object which will contain the result.

    \return         The number of values found.
    */
    template <typename DistancesPredicates>
    inline size_type nearest_query(DistancesPredicates const& dpred, value_type & v) const
    {
        return raw_nearest_one(dpred, detail::empty(), v);
    }

    /*!
    Find one value meeting distances predicates and spatial predicates,
    e.g. nearest to some point and intersecting some box.

    \note Exception-safety: strong

    \param dpred    The distances predicates. May be a Point. This is default case where Value which
                    nearest point is closest to Point is returned. May be a PointRelation which define
                    how distance to Value is calculated. This may be generated by bgi::to_nearest(Point),
                    bgi::to_centroid(Point) or bgi::to_furthest(Point). DistancesPredicates may also define distances
                    bounds. E.g. that some distance must be between min_distance and max_distance. This may
                    be generated by bgi::unbounded(PointRelation) - default case, bgi::min_bounded(PointRelation, MinRelation),
                    bgi::max_bounded(PointRelation, MaxRelation), bgi::bounded(PointRelation, MinRelation, MaxRelation).
                    MinRelation and MaxRelation describes bounds and may be generated by bgi::to_nearest(dist_bound),
                    bgi::to_centroid(dist_bound) or bgi::to_furthest(dist_bound).
    \param pred     The spatial predicates. May be a Geometry (in this case default
                    predicate - intersects is used) or generated by bgi::covered_by(geometry),
                    bgi::disjoint(geometry), bgi::intersects(geometry), bgi::overlaps(geometry),
                    bgi::within(geometry), !bgi::covered_by(geometry), !bgi::disjoint(geometry),
                    !bgi::intersects(geometry), !bgi::overlaps(geometry), !bgi::within(geometry)
                    or bgi::value(func). Those predicates may be passed together in std::pair
                    or boost::tuple.
    \param v        The reference to the object which will contain the result.

    \return         The number of values found.
    */
    template <typename DistancesPredicates, typename Predicates>
    inline size_type nearest_query(DistancesPredicates const& dpred, Predicates const& pred, value_type & v) const
    {
        return raw_nearest_one(dpred, pred, v);
    }

    /*!
    Find k values meeting distances predicates, e.g. k nearest values to some point.

    \note Exception-safety: strong

    \param dpred    The distances predicates. May be a Point. This is default case where Value which
                    nearest point is closest to Point is returned. May be a PointRelation which define
                    how distance to Value is calculated. This may be generated by bgi::to_nearest(Point),
                    bgi::to_centroid(Point) or bgi::to_furthest(Point). DistancesPredicates may also define distances
                    bounds. E.g. that some distance must be between min_distance and max_distance. This may
                    be generated by bgi::unbounded(PointRelation) - default case, bgi::min_bounded(PointRelation, MinRelation),
                    bgi::max_bounded(PointRelation, MaxRelation), bgi::bounded(PointRelation, MinRelation, MaxRelation).
                    MinRelation and MaxRelation describes bounds and may be generated by bgi::to_nearest(dist_bound),
                    bgi::to_centroid(dist_bound) or bgi::to_furthest(dist_bound).
    \param k        The max number of values.
    \param out_it   The output iterator of the result range. E.g. a back_insert_iterator.

    \return         The number of values found.
    */
    template <typename DistancesPredicates, typename OutIter>
    inline size_type nearest_query(DistancesPredicates const& dpred, size_t k, OutIter out_it) const
    {
        return raw_nearest_k(dpred, k, detail::empty(), out_it);
    }

    /*!
    Find k values meeting distances predicates and spatial predicates,
    e.g. k nearest values to some point and intersecting some box.

    \note Exception-safety: strong

    \param dpred    The distances predicates. May be a Point. This is default case where Value which
                    nearest point is closest to Point is returned. May be a PointRelation which define
                    how distance to Value is calculated. This may be generated by bgi::to_nearest(Point),
                    bgi::to_centroid(Point) or bgi::to_furthest(Point). DistancesPredicates may also define distances
                    bounds. E.g. that some distance must be between min_distance and max_distance. This may
                    be generated by bgi::unbounded(PointRelation) - default case, bgi::min_bounded(PointRelation, MinRelation),
                    bgi::max_bounded(PointRelation, MaxRelation), bgi::bounded(PointRelation, MinRelation, MaxRelation).
                    MinRelation and MaxRelation describes bounds and may be generated by bgi::to_nearest(dist_bound),
                    bgi::to_centroid(dist_bound) or bgi::to_furthest(dist_bound).
    \param k        The max number of values.
    \param pred     The spatial predicates. May be a Geometry (in this case default
                    predicate - intersects is used) or generated by bgi::covered_by(geometry),
                    bgi::disjoint(geometry), bgi::intersects(geometry), bgi::overlaps(geometry),
                    bgi::within(geometry), !bgi::covered_by(geometry), !bgi::disjoint(geometry),
                    !bgi::intersects(geometry), !bgi::overlaps(geometry), !bgi::within(geometry)
                    or bgi::value(func). Those predicates may be passed together in std::pair
                    or boost::tuple.
    \param out_it   The output iterator of the result range. E.g. a back_insert_iterator.

    \return         The number of values found.
    */
    template <typename DistancesPredicates, typename Predicates, typename OutIter>
    inline size_type nearest_query(DistancesPredicates const& dpred, size_t k, Predicates const& pred, OutIter out_it) const
    {
        return raw_nearest_k(dpred, k, pred, out_it);
    }

    /*!
    Returns the number of stored values.

    \note Exception-safety: nothrow

    \return         The number of stored values.
    */
    inline size_type size() const
    {
        return m_values_count;
    }

    /*!
    Query if the container is empty.

    \note Exception-safety: nothrow

    \return         true if the container is empty.
    */
    inline bool empty() const
    {
        return 0 == m_values_count;
    }

    /*!
    Removes all values stored in the container.

    \note Exception-safety: nothrow.
    */
    inline void clear()
    {
        this->raw_destroy(*this, false);
    }

    /*!
    Returns the box containing all values stored in the container.
    If the container is empty the result of geometry::assign_inverse() is returned.

    \note Exception-safety: nothrow.

    \return     The box containing all values stored in the container or an invalid box if
                there are no values in the container.
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
    Returns parameters.

    \note Exception-safety: nothrow.

    \return     The parameters object.
    */
    inline parameters_type const& parameters() const
    {
        return m_parameters;
    }

    /*!
    Returns the translator object.

    \note Exception-safety: nothrow.

    \return     The translator object.
    */
    inline translator_type const& translator() const
    {
        return m_translator;
    }

    /*!
    Returns allocator used by the rtree.

    \note Exception-safety: nothrow

    \return     The allocator.
    */
    allocator_type get_allocator() const
    {
        return m_allocators.allocator;
    }

#if !defined(BOOST_GEOMETRY_INDEX_ENABLE_DEBUG_INTERFACE)
private:
#endif
    /*!
    Apply a visitor to the nodes structure in order to perform some operator.
    This function is not a part of the 'official' interface. However it makes
    possible e.g. to pass a visitor drawing the tree structure.

    \note Exception-safety: the same as Visitor::operator().

    \param visitor  The visitor object.
    */
    template <typename Visitor>
    inline void apply_visitor(Visitor & visitor) const
    {
        if ( m_root )
            detail::rtree::apply_visitor(visitor, *m_root);
    }

    /*!
    Returns the number of stored objects. Same as size()
    This function is not a part of the 'official' interface.

    \note Exception-safety: nothrow

    \return     The number of stored objects.
    */
    inline size_type values_count() const
    {
        return m_values_count;
    }

    /*!
    Returns the depth of the R-tree.
    This function is not a part of the 'official' interface.

    \note Exception-safety: nothrow.

    \return     The depth of the R-tree.
    */
    inline size_type depth() const
    {
        return m_leafs_level;
    }

private:
    /*!
    Insert a value to the index. Root node must exist.

    \note Exception-safety: basic

    \param value    The value which will be stored in the container.
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
    Remove the value from the container.

    \note Exception-safety: basic

    \param value    The value which will be removed from the container.
    */
    inline void raw_remove(value_type const& value)
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
    }

    /*!
    Create an empty R-tree i.e. new empty root node and clear other attributes.

    \note Exception-safety: strong.
    */
    inline void raw_create()
    {
        BOOST_GEOMETRY_INDEX_ASSERT(0 == m_root, "the tree is already created");

        m_root = detail::rtree::create_node<allocators_type, leaf>::apply(m_allocators);                            // MAY THROW (N: alloc)
        m_values_count = 0;
        m_leafs_level = 0;
    }

    /*!
    Destroy the R-tree i.e. all nodes and clear attributes.

    \note Exception-safety: nothrow.

    \param t    The container which is going to be destroyed.
    */
    inline void raw_destroy(rtree & t, bool destroy_root = true)
    {
        if ( t.m_root )
        {
            if ( destroy_root )
            {
                detail::rtree::visitors::destroy<value_type, options_type, translator_type, box_type, allocators_type> del_v(t.m_root, t.m_allocators);
                detail::rtree::apply_visitor(del_v, *t.m_root);
            }
            else
            {
                detail::rtree::clear_node<value_type, options_type, translator_type, box_type, allocators_type>::apply(*t.m_root, t.m_allocators);
            }

            t.m_root = 0;
        }
        t.m_values_count = 0;
        t.m_leafs_level = 0;
    }

    /*!
    Copy the R-tree i.e. whole nodes structure, values and other attributes.
    It uses destination's allocators to create the new structure.

    \note Exception-safety: strong.

    \param src    The source R-tree.
    \param dst    The destination R-tree.
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
    Find one value meeting distances and spatial predicates.

    \note Exception-safety: strong.
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

        result_type result;

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

        return result.get(v);
    }

    /*!
    Find k values meeting distances and spatial predicates.

    \note Exception-safety: strong.
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
            point_type
        > result_type;

        result_type result(k);

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

        return result.get(out_it);
    }

    translator_type m_translator;
    Parameters m_parameters;
    allocators_type m_allocators;

    size_type m_values_count;
    size_type m_leafs_level;
    node * m_root;
};

/*!
Insert a value to the index.

\param tree The spatial index.
\param v    The value which will be stored in the index.
*/
template <typename Value, typename Options, typename Translator, typename Allocator>
inline void insert(rtree<Value, Options, Translator, Allocator> & tree, Value const& v)
{
    tree.insert(v);
}

/*!
Insert a range of values to the index.

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
Insert a range of values to the index.

\param tree     The spatial index.
\param rng      The range of values.
*/
template<typename Value, typename Options, typename Translator, typename Allocator, typename Range>
inline void insert(rtree<Value, Options, Translator, Allocator> & tree, Range const& rng)
{
    tree.insert(rng);
}

/*!
Remove a value from the index.

\param tree The spatial index.
\param v    The value which will be removed from the index.
*/
template <typename Value, typename Options, typename Translator, typename Allocator>
inline void remove(rtree<Value, Options, Translator, Allocator> & tree, Value const& v)
{
    tree.remove(v);
}

/*!
Remove a range of values from the index.

\param tree     The spatial index.
\param first    The beginning of the range of values.
\param last     The end of the range of values.
*/
template<typename Value, typename Options, typename Translator, typename Allocator, typename Iterator>
inline void remove(rtree<Value, Options, Translator, Allocator> & tree, Iterator first, Iterator last)
{
    tree.remove(first, last);
}

/*!
Remove a range of values from the index.

\param tree     The spatial index.
\param rng      The range of values.
*/
template<typename Value, typename Options, typename Translator, typename Allocator, typename Range>
inline void remove(rtree<Value, Options, Translator, Allocator> & tree, Range const& rng)
{
    tree.remove(rng);
}

/*!
Find values meeting spatial predicates.

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
Find the value meeting distances predicates.

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
Find the value meeting distances and spatial predicates.

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
Find k values meeting distances predicates.

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
Find k values meeting distances and spatial predicates.

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
Remove all values from the index.

\param tree     The spatial index.
*/
template <typename Value, typename Options, typename Translator, typename Allocator>
inline void clear(rtree<Value, Options, Translator, Allocator> & tree)
{
    return tree.clear();
}

/*!
Get the number of values stored in the index.

\param tree     The spatial index.

\return         The number of values stored in the index.
*/
template <typename Value, typename Options, typename Translator, typename Allocator>
inline size_t size(rtree<Value, Options, Translator, Allocator> const& tree)
{
    return tree.size();
}

/*!
Query if there are no values stored in the index.

\param tree     The spatial index.

\return         true if there are no values in the index.
*/
template <typename Value, typename Options, typename Translator, typename Allocator>
inline bool empty(rtree<Value, Options, Translator, Allocator> const& tree)
{
    return tree.empty();
}

/*!
Get the box containing all stored values or an invalid box if the index has no values.

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
