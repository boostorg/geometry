// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.Index - R*-tree insert algorithm implementation
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_RSTAR_RTREE_INSERT_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_RSTAR_RTREE_INSERT_HPP

#include <algorithm>

#include <boost/tuple/tuple.hpp>

#include <boost/geometry/extensions/index/algorithms/area.hpp>
#include <boost/geometry/extensions/index/algorithms/margin.hpp>
#include <boost/geometry/extensions/index/algorithms/overlap.hpp>
#include <boost/geometry/extensions/index/algorithms/union_area.hpp>

#include <boost/geometry/extensions/index/rtree/rtree_node.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/rtree_insert.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/rtree_is_leaf.hpp>

namespace boost { namespace geometry { namespace index {

namespace visitors {

// ----------------------------------------------------------------------------- //
// rstar/choose_next_node
// ----------------------------------------------------------------------------- //

// TODO: awulkiew - it's possible that goodness values may be used to choose next node

// rtree choose_next_node algorithm for rstar variant

template <typename Value, typename Box>
class rtree_rstar_choose_next_node
{
    typedef typename index::detail::rtree_node<Value, Box, rtree_rstar_tag>::type node;
    typedef typename index::detail::rtree_internal_node<Value, Box, rtree_rstar_tag>::type internal_node;
    typedef typename index::detail::rtree_leaf<Value, Box, rtree_rstar_tag>::type leaf;

    typedef typename internal_node::children_type children_type;

public:
    // TODO: awulkiew - alternative lower-cost version of overlap calculation may be used (branch_areas)
    // TODO: awulkiew - further optimization: don't calculate area with the overlap, calculate it only if
    // overlap < smallest_overlap (and current area must be stored) OR
    // overlap == smallest_overlap (and area must be compared)
    template <typename Indexable>
    static inline size_t apply(internal_node & n, Indexable const& indexable)
    {
        assert(!n.children.empty());
        
        bool has_leaves = boost::apply_visitor(
            visitors::rtree_is_leaf<Value, Box, rtree_rstar_tag>(),
            *n.children.front().second);

        if ( !has_leaves )
            return impl<internal_node_areas>(n, indexable);
        else
            return impl<branch_areas>(n, indexable);
    }

private:
    template <typename Areas, typename Indexable>
    static inline size_t impl(internal_node & n, Indexable const& indexable)
    {
        typedef typename children_type::iterator children_iterator;

        //assert(!n.children.empty());

        children_iterator temp_it = n.children.begin();
        children_iterator child_it = temp_it;
        Areas min_areas(n.children, child_it, indexable);

        for (children_iterator it = ++temp_it;
            it != n.children.end(); ++it)
        {
            Areas areas(n.children, it, indexable);

            if ( areas < min_areas )
            {
                child_it = it;
                min_areas = areas;
            }
        }

        // TODO: awulkiew - switch to indexes in the whole class?
        return child_it - n.children.begin();
    }

    struct branch_areas
    {
        typedef typename area_result<Box>::type area_type;

        template <typename Indexable>
        inline branch_areas(children_type const& ch, typename children_type::iterator const& k_it, Indexable const& indexable)
        {
            overlap_area = 0;
            for (typename children_type::const_iterator it = ch.begin(); it != ch.end(); ++it)
                if ( it != k_it )
                    overlap_area += index::overlap(k_it->first, it->first);

            area = index::area(k_it->first);

            diff_area = index::union_area(k_it->first, indexable) - area;
        }

        inline bool operator<(branch_areas &a) const
        {
            return overlap_area < a.overlap_area ||
                ( overlap_area == a.overlap_area && diff_area < a.diff_area ) ||
                ( diff_area == a.diff_area && area < a.area );
        }

        area_type overlap_area;
        area_type diff_area;
        area_type area;
    };

    struct internal_node_areas
    {
        typedef typename area_result<Box>::type area_type;

        template <typename Indexable>
        inline internal_node_areas(children_type const&, typename children_type::iterator const& k_it, Indexable const& indexable)
        {
            area = index::area(k_it->first);
            diff_area = index::union_area(k_it->first, indexable) - area;
        }

        inline bool operator<(internal_node_areas &a) const
        {
            return diff_area < a.diff_area ||
                ( diff_area == a.diff_area && area < a.area );
        }

        area_type diff_area;
        area_type area;
    };
};

// ----------------------------------------------------------------------------- //
// rstar/sorts
// ----------------------------------------------------------------------------- //

// elements ids less comparator

template <typename Elements, typename Translator, size_t Corner, size_t AxisIndex>
class rtree_rstar_elements_indexes_less
{
public:
    rtree_rstar_elements_indexes_less(Elements const& elements, Translator const& tr)
        : m_elements(elements), m_tr(tr)
    {}

    bool operator()(size_t e1_index, size_t e2_index)
    {
        return
            index::detail::get<Corner, AxisIndex>(
                index::detail::rtree_element_indexable(m_elements[e1_index], m_tr)
            )
            <
            index::detail::get<Corner, AxisIndex>(
                index::detail::rtree_element_indexable(m_elements[e2_index], m_tr)
            );
    }

private:
    Elements const& m_elements;
    Translator const& m_tr;
};

template <typename T, size_t Dimension>
class rtree_rstar_vectors
{
public:
    inline rtree_rstar_vectors(size_t count)
        : m_count(count), m_ids(Dimension * 2 * count)
    {}

    template <size_t AxisIndex, size_t Corner>
    inline typename std::vector<T>::iterator begin()
    {
        BOOST_STATIC_ASSERT(AxisIndex < Dimension);
        BOOST_STATIC_ASSERT(Corner < 2);
        return m_ids.begin() + ( AxisIndex * 2 + Corner ) * m_count;
    }

    template <size_t AxisIndex, size_t Corner>
    inline typename std::vector<T>::iterator end()
    {
        BOOST_STATIC_ASSERT(AxisIndex < Dimension);
        BOOST_STATIC_ASSERT(Corner < 2);
        return m_ids.begin() + ( AxisIndex * 2 + Corner + 1) * m_count;
    }

    template <size_t AxisIndex, size_t Corner>
    inline T const& get(size_t index) const
    {
        BOOST_STATIC_ASSERT(AxisIndex < Dimension);
        BOOST_STATIC_ASSERT(Corner < 2);
        assert(index < m_count);
        return m_ids[( AxisIndex * 2 + Corner ) * m_count + index];
    }

    template <size_t AxisIndex, size_t Corner>
    inline void set(size_t index, T const& v)
    {
        BOOST_STATIC_ASSERT(AxisIndex < Dimension);
        BOOST_STATIC_ASSERT(Corner < 2);
        assert(index < m_count);
        m_ids[( AxisIndex * 2 + Corner ) * m_count + index] = v;
    }

    inline T const& get(size_t axis, size_t corner, size_t index) const
    {
        assert(axis < Dimension);
        assert(corner < 2);
        assert(index < m_count);
        return m_ids[( axis * 2 + corner ) * m_count + index];
    }

    inline size_t vector_size() const
    {
        return m_count;
    }

private:
    size_t m_count;
    std::vector<T> m_ids;
};

// init indexes of elements and sort them

template <typename Elements, typename Translator, size_t Dimension>
class rtree_rstar_sorts_fill_ids
{
    BOOST_STATIC_ASSERT(0 < Dimension);

public:
    template <size_t D>
    static inline void apply(rtree_rstar_vectors<size_t, D> & ids, Elements const& elements, Translator const& tr)
    {
        rtree_rstar_sorts_fill_ids<Elements, Translator, Dimension - 1>::apply(ids, elements, tr);

        fill<min_corner>(ids, elements, tr);
        fill<max_corner>(ids, elements, tr);
    }

private:
    template <size_t Corner, size_t D>
    static inline void fill(rtree_rstar_vectors<size_t, D> & ids, Elements const& elements, Translator const& tr)
    {
        for ( size_t i = 0 ; i < elements.size() ; ++i )
            ids.set<Dimension - 1, Corner>(i, i);

        rtree_rstar_elements_indexes_less<Elements, Translator, Corner, Dimension - 1> less(elements, tr);
        std::sort(ids.begin<Dimension - 1, Corner>(), ids.end<Dimension - 1, Corner>(), less);
    }
};

template <typename Elements, typename Translator>
class rtree_rstar_sorts_fill_ids<Elements, Translator, 1>
{
public:
    template <size_t D>
    static inline void apply(rtree_rstar_vectors<size_t, D> & ids, Elements const& elements, Translator const& tr)
    {
        fill<min_corner>(ids, elements, tr);
        fill<max_corner>(ids, elements, tr);
    }

private:
    template <size_t Corner, size_t D>
    static inline void fill(rtree_rstar_vectors<size_t, D> & ids, Elements const& elements, Translator const& tr)
    {
        for ( size_t i = 0 ; i < elements.size() ; ++i )
            ids.set<0, Corner>(i, i);

        rtree_rstar_elements_indexes_less<Elements, Translator, Corner, 0> less(elements, tr);
        std::sort(ids.begin<0, Corner>(), ids.end<0, Corner>(), less);
    }
};

// sorts for each axis for min and max

template <typename Elements, typename Translator, size_t Dimension>
class rtree_rstar_sorts
{
public:
    inline rtree_rstar_sorts(Elements const& elements, Translator const& tr)
        : m_elems(elements), m_tr(tr), m_ids(m_elems.size())
    {
        rtree_rstar_sorts_fill_ids<Elements, Translator, Dimension>::apply(m_ids, elements, tr);
    }

    // TODO: awulkiew - should those methods be here? Just leave get_element?

    template <size_t AxisIndex, size_t Corner>
    inline typename index::detail::rtree_element_indexable_type<typename Elements::value_type, Translator>::type const&
    get_indexable(size_t index) const
    {
        size_t id = m_ids.get<AxisIndex, Corner>(index);
        return index::detail::rtree_element_indexable(m_elems[id], m_tr);
    }

    inline typename index::detail::rtree_element_indexable_type<typename Elements::value_type, Translator>::type const&
    get_indexable(size_t axis, size_t corner, size_t index) const
    {
        size_t id = m_ids.get(axis, corner, index);
        return index::detail::rtree_element_indexable(m_elems[id], m_tr);
    }

    template <size_t AxisIndex, size_t Corner>
    inline typename Elements::value_type const& get_element(size_t index) const
    {
        size_t id = m_ids.get<AxisIndex, Corner>(index);
        return m_elems[id];
    }

    inline typename Elements::value_type const& get_element(size_t axis, size_t corner, size_t index) const
    {
        size_t id = m_ids.get(axis, corner, index);
        return m_elems[id];
    }

    inline size_t sort_size() const
    {
        return m_ids.vector_size();
    }

private:
    Elements const& m_elems;
    Translator const& m_tr;

    rtree_rstar_vectors<size_t, Dimension> m_ids;
};

// ----------------------------------------------------------------------------- //
// rstar/goodness_values
// ----------------------------------------------------------------------------- //

// calculate margins, areas and overlaps

template <typename Box, size_t AxisIndex, size_t Corner>
class rtree_rstar_calculate_goodness_values_for_axis
{
    typedef typename geometry::traits::coordinate_type<
        typename geometry::traits::point_type<Box>::type
    >::type coordinate_type;

    static const size_t dimension = geometry::traits::dimension<
        typename geometry::traits::point_type<Box>::type
    >::value;

    typedef typename area_result<Box>::type area_type;
    typedef typename margin_result<Box>::type margin_type;
    typedef typename overlap_result<Box>::type overlap_type;
    typedef boost::tuple<Box, Box, margin_type, overlap_type, area_type> goodness_type;

public:
    template <typename Elements, typename Translator>
    static inline void apply(
        rtree_rstar_vectors<goodness_type, dimension> & goodness_vectors,
        rtree_rstar_sorts<Elements, Translator, dimension> const& sorts,
        size_t min_elems,
        size_t max_elems)
    {
        assert(sorts.sort_size() == max_elems + 1);
        assert(goodness_vectors.vector_size() == max_elems - 2 * min_elems + 2);

        size_t median_index_last = max_elems - min_elems + 2;
        size_t i = 0;
        for ( size_t median_index = min_elems ; median_index < median_index_last ; ++median_index, ++i )
        {
            Box left_box = calculate_box(sorts, 0, median_index);
            Box right_box = calculate_box(sorts, median_index, sorts.sort_size());

            margin_type margin = index::margin(left_box) + index::margin(right_box);
            area_type area = index::area(left_box) + index::area(right_box);
            overlap_type overlap = index::overlap(left_box, right_box);

            goodness_vectors.set<AxisIndex, Corner>(
                i,
                boost::make_tuple(left_box, right_box, margin, overlap, area));
        }
    }
private:
    template <typename Elements, typename Translator>
    static inline Box calculate_box(
        rtree_rstar_sorts<Elements, Translator, dimension> const& sorts,
        size_t index_first,
        size_t index_last)
    {
        assert(index_first < index_last);

        Box result;
        geometry::convert(sorts.template get_indexable<AxisIndex, Corner>(index_first), result);
        ++index_first;
        for ( ; index_first < index_last ; ++index_first )
            geometry::expand(result, sorts.template get_indexable<AxisIndex, Corner>(index_first));
        return result;
    }
};

// calculate goodness values for all axis

template <typename Box, size_t Dimension>
struct rtree_rstar_calculate_goodness_values
{
    BOOST_STATIC_ASSERT(0 < Dimension);

    typedef typename area_result<Box>::type area_type;
    typedef typename margin_result<Box>::type margin_type;
    typedef typename overlap_result<Box>::type overlap_type;
    typedef boost::tuple<Box, Box, margin_type, overlap_type, area_type> goodness_type;

    template <typename Elements, typename Translator, size_t D>
    static inline void apply(
        rtree_rstar_vectors<goodness_type, D> & goodness_vectors,
        rtree_rstar_sorts<Elements, Translator, D> const& sorts,
        size_t min_elems,
        size_t max_elems)
    {
        rtree_rstar_calculate_goodness_values<Box, Dimension - 1>
            ::apply(goodness_vectors, sorts, min_elems, max_elems);

        rtree_rstar_calculate_goodness_values_for_axis<Box, Dimension - 1, min_corner>
            ::apply(goodness_vectors, sorts, min_elems, max_elems);
        rtree_rstar_calculate_goodness_values_for_axis<Box, Dimension - 1, max_corner>
            ::apply(goodness_vectors, sorts, min_elems, max_elems);
    }
};

template <typename Box>
struct rtree_rstar_calculate_goodness_values<Box, 1>
{
    typedef typename area_result<Box>::type area_type;
    typedef typename margin_result<Box>::type margin_type;
    typedef typename overlap_result<Box>::type overlap_type;
    typedef boost::tuple<Box, Box, margin_type, overlap_type, area_type> goodness_type;

    template <typename Elements, typename Translator, size_t D>
    static inline void apply(
        rtree_rstar_vectors<goodness_type, D> & goodness_vectors,
        rtree_rstar_sorts<Elements, Translator, D> const& sorts,
        size_t min_elems,
        size_t max_elems)
    {
        rtree_rstar_calculate_goodness_values_for_axis<Box, 0, min_corner>
            ::apply(goodness_vectors, sorts, min_elems, max_elems);
        rtree_rstar_calculate_goodness_values_for_axis<Box, 0, max_corner>
            ::apply(goodness_vectors, sorts, min_elems, max_elems);
    }
};

// goodness values

template <typename Elements, typename Translator, typename Box>
class rtree_rstar_goodness_values
{
    typedef typename geometry::traits::coordinate_type<
        typename geometry::traits::point_type<Box>::type
    >::type coordinate_type;

    static const size_t dimension = geometry::traits::dimension<
        typename geometry::traits::point_type<Box>::type
    >::value;

    typedef typename area_result<Box>::type area_type;
    typedef typename margin_result<Box>::type margin_type;
    typedef typename overlap_result<Box>::type overlap_type;
    typedef boost::tuple<Box, Box, margin_type, overlap_type, area_type> goodness_type;

public:
    inline rtree_rstar_goodness_values(
        Elements const& elements,
        Translator const& tr,
        size_t min_elems,
        size_t max_elems)
        : m_sorts(elements, tr)
        , m_goodness_vectors(max_elems - 2 * min_elems + 2)
    {
        rtree_rstar_calculate_goodness_values<Box, dimension>::apply(
            m_goodness_vectors, m_sorts, min_elems, max_elems);
    }

    inline size_t elements_size() const
    {
        return m_sorts.sort_size();
    }

    inline size_t goodnesses_size() const
    {
        return m_goodness_vectors.vector_size();
    }

    inline goodness_type const& get_goodness(size_t axis, size_t corner, size_t index) const
    {
        return m_goodness_vectors.get(axis, corner, index);
    }

    inline typename Elements::value_type const& get_element(size_t axis, size_t corner, size_t index) const
    {
        return m_sorts.get_element(axis, corner, index);
    }

private:
    rtree_rstar_sorts<Elements, Translator, dimension> m_sorts;
    rtree_rstar_vectors<goodness_type, dimension> m_goodness_vectors;
};

template <typename Elements, typename Translator, typename Box>
class rtree_rstar_choose_split_axis
{
    typedef typename margin_result<Box>::type margin_type;
public:
    static inline std::pair<size_t, size_t> apply(rtree_rstar_goodness_values<Elements, Translator, Box> const& gv)
    {
        std::pair<size_t, size_t> smallest_distr = std::make_pair(0, 0);
        margin_type smallest_margins_sum = calculate_margins_sum(0, 0, gv);

        margin_type margins_sum = calculate_margins_sum(0, 1, gv);
        if ( margins_sum < smallest_margins_sum )
        {
            smallest_distr.second = 1;
            smallest_margins_sum = margins_sum;
        }

        for ( size_t a = 1 ; a < traits::dimension<Box>::value ; ++a )
        {
            for ( size_t c = 0 ; c < 2 ; ++c )
            {
                margins_sum = calculate_margins_sum(a, c, gv);
                if ( margins_sum < smallest_margins_sum )
                {
                    smallest_distr.first = a;
                    smallest_distr.second = c;
                    smallest_margins_sum = margins_sum;
                }
            }
        }
        return smallest_distr;
    }
private:
    static inline typename margin_result<Box>::type
            calculate_margins_sum(size_t axis, size_t corner, rtree_rstar_goodness_values<Elements, Translator, Box> const& gv)
    {
        typename margin_result<Box>::type margins_sum = 0;
        for ( size_t i = 0 ; i < gv.goodnesses_size() ; ++i )
            margins_sum += boost::get<2>(gv.get_goodness(axis, corner, i));
        return margins_sum;
    }
};

template <typename Elements, typename Translator, typename Box>
class rtree_rstar_choose_split_index
{
    typedef typename area_result<Box>::type area_type;
    typedef typename overlap_result<Box>::type overlap_type;
public:
    static inline size_t apply(rtree_rstar_goodness_values<Elements, Translator, Box> const& gv, size_t axis, size_t corner)
    {
        size_t smallest_index = 0;
        overlap_type smallest_overlap = boost::get<3>(gv.get_goodness(axis, corner, 0));
        overlap_type smallest_area = boost::get<4>(gv.get_goodness(axis, corner, 0));

        for ( size_t i = 1 ; i < gv.goodnesses_size() ; ++i )
        {
            overlap_type overlap = boost::get<3>(gv.get_goodness(axis, corner, i));
            if ( overlap < smallest_overlap )
            {
                smallest_overlap = overlap;
                smallest_area = boost::get<4>(gv.get_goodness(axis, corner, i));
                smallest_index = i;
            }
            else if ( overlap == smallest_overlap )
            {
                area_type area = boost::get<4>(gv.get_goodness(axis, corner, i));
                if ( area < smallest_area )
                {
                    smallest_area = area;
                    smallest_index = i;
                }
            }
        }

        return smallest_index;
    }
};

// rtree insert routine for rstar variant

template <typename Value, typename Translator, typename Box>
class rtree_insert<Value, Translator, Box, rtree_rstar_tag> : public boost::static_visitor<>
{
    typedef typename index::detail::rtree_node<Value, Box, rtree_rstar_tag>::type node;
    typedef typename index::detail::rtree_internal_node<Value, Box, rtree_rstar_tag>::type internal_node;
    typedef typename index::detail::rtree_leaf<Value, Box, rtree_rstar_tag>::type leaf;

public:
    inline explicit rtree_insert(node* & root, Value const& v, size_t min_elements, size_t max_elements, Translator const& t)
        : m_value(v), m_tr(t), m_min_elems_per_node(min_elements), m_max_elems_per_node(max_elements)
        , m_root_node(root)
        , m_parent(0), m_current_child_index(0)
    {}

    inline void operator()(internal_node & n)
    {
        // save previous traverse inputs and set new ones
        internal_node * parent_bckup = m_parent;
        m_parent = &n;
        size_t current_child_index_bckup = m_current_child_index;

        // choose next node, where value insert traversing should go
        m_current_child_index = rtree_rstar_choose_next_node<Value, Box>::apply(n, m_tr(m_value));

        // TODO: awulkiew - if reinsert is implemented this must be changed
        geometry::expand(n.children[m_current_child_index].first, m_tr(m_value));

        // next traversing step
        boost::apply_visitor(*this, *n.children[m_current_child_index].second);

        // restore previous traverse inputs
        m_parent = parent_bckup;
        m_current_child_index = current_child_index_bckup;

        if ( m_max_elems_per_node < n.children.size() )
        {
            split(n);
        }
    }

    inline void operator()(leaf & n)
    {
        n.values.push_back(m_value);

        if ( m_max_elems_per_node < n.values.size() )
        {
            split(n);
        }
    }

private:
    template <typename Node>
    inline void split(Node & n)
    {
        typedef typename index::detail::rtree_elements_type<Node>::type elements_type;
        typedef typename elements_type::value_type element_type;

        elements_type & elems = index::detail::get_elements(n);

        rtree_rstar_goodness_values<elements_type, Translator, Box> gv(elems, m_tr, m_min_elems_per_node, m_max_elems_per_node);

        std::pair<size_t, size_t> axis_and_corner =
            rtree_rstar_choose_split_axis<elements_type, Translator, Box>::apply(gv);
        size_t axis = axis_and_corner.first;
        size_t corner = axis_and_corner.second;

        size_t choosen_index =
            rtree_rstar_choose_split_index<elements_type, Translator, Box>::apply(gv, axis, corner);

        size_t median_index = m_min_elems_per_node + choosen_index;
        assert(median_index <= m_max_elems_per_node + 1 - m_min_elems_per_node);

        // create new node
        node * right_node = rtree_create_node(Node());
        elements_type & new_elems = index::detail::get_elements(boost::get<Node>(*right_node));

        // update new node's elements
        new_elems.reserve(m_max_elems_per_node + 1 - median_index);
        for ( size_t i = median_index ; i < m_max_elems_per_node + 1 ; ++i)
            new_elems.push_back(gv.get_element(axis, corner, i));

        // fill temporary container with left node's elements
        elements_type left_elems;
        left_elems.reserve(median_index);
        for ( size_t i = 0 ; i < median_index ; ++i )
            left_elems.push_back(gv.get_element(axis, corner, i));
        // update old node's elements
        elems = left_elems;
        
        if ( m_parent != 0 )
        {
            // update old node's box
            m_parent->children[m_current_child_index].first = boost::get<0>(gv.get_goodness(axis, corner, choosen_index));
            // add new node to the parent's children
            m_parent->children.push_back( std::make_pair(
                boost::get<1>(gv.get_goodness(axis, corner, choosen_index)), right_node ) );
        }
        else
        {
            assert(&n == boost::get<Node>(m_root_node));

            // create new root and add nodes
            node * new_root = rtree_create_node(internal_node());
            
            boost::get<internal_node>(*new_root).children.push_back(
                std::make_pair(boost::get<0>(gv.get_goodness(axis, corner, choosen_index)), m_root_node)
            );
            boost::get<internal_node>(*new_root).children.push_back(
                std::make_pair(boost::get<1>(gv.get_goodness(axis, corner, choosen_index)), right_node)
            );
            
            m_root_node = new_root;
        }
    }

    Value const& m_value;
    Translator const& m_tr;
    size_t m_min_elems_per_node;
    size_t m_max_elems_per_node;

    node* & m_root_node;

    // traversing input parameters
    internal_node *m_parent;
    size_t m_current_child_index;
};

} // namespace visitors

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_RSTAR_RTREE_INSERT_HPP
