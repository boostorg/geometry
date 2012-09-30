// Boost.Geometry Index
//
// R-tree R*-tree split algorithm implementation
//
// Copyright (c) 2011-2012 Adam Wulkiewicz, Lodz, Poland.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_RSTAR_REDISTRIBUTE_ELEMENTS_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_RSTAR_REDISTRIBUTE_ELEMENTS_HPP

#include <boost/geometry/extensions/index/nonassignable.hpp>

#include <boost/geometry/extensions/index/algorithms/intersection_content.hpp>
#include <boost/geometry/extensions/index/algorithms/union_content.hpp>
#include <boost/geometry/extensions/index/algorithms/margin.hpp>

#include <boost/geometry/extensions/index/rtree/node/node.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/insert.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/is_leaf.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree { namespace visitors {

namespace detail {

namespace rstar {

template <typename Element, typename NodeProxy, size_t Corner, size_t AxisIndex>
class element_axis_corner_less
    : index::nonassignable
{
public:
    element_axis_corner_less(NodeProxy const& node_proxy)
        : m_node_proxy(node_proxy)
    {}

    bool operator()(Element const& e1, Element const& e2) const
    {
        return index::get<Corner, AxisIndex>(m_node_proxy.indexable(e1))
            < index::get<Corner, AxisIndex>(m_node_proxy.indexable(e2));
    }

private:
    NodeProxy const& m_node_proxy;
};

template <typename NodeProxy, size_t Corner, size_t AxisIndex>
struct choose_split_axis_and_index_for_corner
{
    typedef typename NodeProxy::box_type box_type;
    typedef typename index::default_margin_result<box_type>::type margin_type;
    typedef typename index::default_content_result<box_type>::type content_type;

    template <typename Elements>
    static inline void apply(Elements const& elements,
                             size_t & choosen_index,
                             margin_type & sum_of_margins,
                             content_type & smallest_overlap,
                             content_type & smallest_content,
                             NodeProxy const& node_proxy)
    {
        typedef typename Elements::value_type element_type;

        BOOST_GEOMETRY_INDEX_ASSERT(elements.size() == node_proxy.parameters().get_max_elements() + 1, "wrong number of elements");

        // copy elements
        Elements elements_copy = elements;
        
        // sort elements
        element_axis_corner_less<element_type, NodeProxy, Corner, AxisIndex> elements_less(node_proxy);
        std::sort(elements_copy.begin(), elements_copy.end(), elements_less);

        // init outputs
        choosen_index = node_proxy.parameters().get_min_elements();
        sum_of_margins = 0;
        smallest_overlap = (std::numeric_limits<content_type>::max)();
        smallest_content = (std::numeric_limits<content_type>::max)();

        // calculate sum of margins for all distributions
        size_t index_last = node_proxy.parameters().get_max_elements() - node_proxy.parameters().get_min_elements() + 2;
        for ( size_t i = node_proxy.parameters().get_min_elements() ; i < index_last ; ++i )
        {
            // TODO - awulkiew: may be optimized - box of group 1 may be initialized with
            // box of min_elems number of elements and expanded for each iteration by another element

            box_type box1 = node_proxy.elements_box(elements_copy.begin(), elements_copy.begin() + i);
            box_type box2 = node_proxy.elements_box(elements_copy.begin() + i, elements_copy.end());
            
            sum_of_margins += index::margin(box1) + index::margin(box2);

            content_type ovl = index::intersection_content(box1, box2);
            content_type con = index::content(box1) + index::content(box2);

            if ( ovl < smallest_overlap || (ovl == smallest_overlap && con <= smallest_content) )
            {
                choosen_index = i;
                smallest_overlap = ovl;
                smallest_content = con;
            }
        }
    }
};

template <typename NodeProxy, size_t AxisIndex, typename ElementIndexableTag>
struct choose_split_axis_and_index_for_axis
{
    //BOOST_STATIC_ASSERT(0);
};

template <typename NodeProxy, size_t AxisIndex>
struct choose_split_axis_and_index_for_axis<NodeProxy, AxisIndex, box_tag>
{
    typedef typename NodeProxy::box_type box_type;
    typedef typename index::default_margin_result<box_type>::type margin_type;
    typedef typename index::default_content_result<box_type>::type content_type;

    template <typename Elements>
    static inline void apply(Elements const& elements,
                             size_t & choosen_corner,
                             size_t & choosen_index,
                             margin_type & sum_of_margins,
                             content_type & smallest_overlap,
                             content_type & smallest_content,
                             NodeProxy const& node_proxy)
    {
        size_t index1 = 0;
        margin_type som1 = 0;
        content_type ovl1 = (std::numeric_limits<content_type>::max)();
        content_type con1 = (std::numeric_limits<content_type>::max)();

        choose_split_axis_and_index_for_corner<NodeProxy, min_corner, AxisIndex>::
            apply(elements, index1,
                  som1, ovl1, con1,
                  node_proxy);

        size_t index2 = 0;
        margin_type som2 = 0;
        content_type ovl2 = (std::numeric_limits<content_type>::max)();
        content_type con2 = (std::numeric_limits<content_type>::max)();

        choose_split_axis_and_index_for_corner<NodeProxy, max_corner, AxisIndex>::
            apply(elements, index2,
                  som2, ovl2, con2,
                  node_proxy);

        sum_of_margins = som1 + som2;

        if ( ovl1 < ovl2 || (ovl1 == ovl2 && con1 <= con2) )
        {
            choosen_corner = min_corner;
            choosen_index = index1;
            smallest_overlap = ovl1;
            smallest_content = con1;
        }
        else
        {
            choosen_corner = max_corner;
            choosen_index = index2;
            smallest_overlap = ovl2;
            smallest_content = con2;
        }
    }
};

template <typename NodeProxy, size_t AxisIndex>
struct choose_split_axis_and_index_for_axis<NodeProxy, AxisIndex, point_tag>
{
    typedef typename NodeProxy::box_type box_type;
    typedef typename index::default_margin_result<box_type>::type margin_type;
    typedef typename index::default_content_result<box_type>::type content_type;

    template <typename Elements>
    static inline void apply(Elements const& elements,
                             size_t & choosen_corner,
                             size_t & choosen_index,
                             margin_type & sum_of_margins,
                             content_type & smallest_overlap,
                             content_type & smallest_content,
                             NodeProxy const& node_proxy)
    {
        choose_split_axis_and_index_for_corner<NodeProxy, min_corner, AxisIndex>::
            apply(elements, choosen_index,
                  sum_of_margins, smallest_overlap, smallest_content,
                  node_proxy);

        choosen_corner = min_corner;
    }
};

template <typename NodeProxy, size_t Dimension>
struct choose_split_axis_and_index
{
    BOOST_STATIC_ASSERT(0 < Dimension);

    typedef typename NodeProxy::box_type box_type;
    typedef typename index::default_margin_result<box_type>::type margin_type;
    typedef typename index::default_content_result<box_type>::type content_type;

    template <typename Elements>
    static inline void apply(Elements const& elements,
                             size_t & choosen_axis,
                             size_t & choosen_corner,
                             size_t & choosen_index,
                             margin_type & smallest_sum_of_margins,
                             content_type & smallest_overlap,
                             content_type & smallest_content,
                             NodeProxy const& node_proxy)
    {
        typedef typename rtree::element_indexable_type<
            typename Elements::value_type,
            typename NodeProxy::translator_type
        >::type element_indexable_type;

        choose_split_axis_and_index<NodeProxy, Dimension - 1>::
            apply(elements, choosen_axis, choosen_corner, choosen_index,
                  smallest_sum_of_margins, smallest_overlap, smallest_content,
                  node_proxy);

        margin_type sum_of_margins = 0;

        size_t corner = min_corner;
        size_t index = 0;

        content_type overlap_val = (std::numeric_limits<content_type>::max)();
        content_type content_val = (std::numeric_limits<content_type>::max)();

        choose_split_axis_and_index_for_axis<
            NodeProxy,
            Dimension - 1,
            typename index::traits::tag<element_indexable_type>::type
        >::apply(elements, corner, index, sum_of_margins, overlap_val, content_val, node_proxy);

        if ( sum_of_margins < smallest_sum_of_margins )
        {
            choosen_axis = Dimension - 1;
            choosen_corner = corner;
            choosen_index = index;
            smallest_sum_of_margins = sum_of_margins;
            smallest_overlap = overlap_val;
            smallest_content = content_val;
        }
    }
};

template <typename NodeProxy>
struct choose_split_axis_and_index<NodeProxy, 1>
{
    typedef typename NodeProxy::box_type box_type;
    typedef typename index::default_margin_result<box_type>::type margin_type;
    typedef typename index::default_content_result<box_type>::type content_type;

    template <typename Elements>
    static inline void apply(Elements const& elements,
                             size_t & choosen_axis,
                             size_t & choosen_corner,
                             size_t & choosen_index,
                             margin_type & smallest_sum_of_margins,
                             content_type & smallest_overlap,
                             content_type & smallest_content,
                             NodeProxy const& node_proxy)
    {
        typedef typename rtree::element_indexable_type<
            typename Elements::value_type,
            typename NodeProxy::translator_type
        >::type element_indexable_type;

        choosen_axis = 0;

        choose_split_axis_and_index_for_axis<
            NodeProxy,
            0,
            typename index::traits::tag<element_indexable_type>::type
        >::apply(elements, choosen_corner, choosen_index, smallest_sum_of_margins, smallest_overlap, smallest_content, node_proxy);
    }
};

template <size_t Corner, size_t Dimension>
struct partial_sort
{
    BOOST_STATIC_ASSERT(0 < Dimension);

    template <typename Elements, typename NodeProxy>
    static inline void apply(Elements & elements, const size_t axis, const size_t index, NodeProxy const& node_proxy)
    {
        if ( axis < Dimension - 1 )
        {
            partial_sort<Corner, Dimension - 1>::apply(elements, axis, index, node_proxy);
        }
        else
        {
            BOOST_GEOMETRY_INDEX_ASSERT(axis == Dimension - 1, "unexpected axis value");

            typedef typename Elements::value_type element_type;
            element_axis_corner_less<element_type, NodeProxy, Corner, Dimension - 1> less(node_proxy);
            std::partial_sort(elements.begin(), elements.begin() + index, elements.end(), less);
        }
    }
};

template <size_t Corner>
struct partial_sort<Corner, 1>
{
    template <typename Elements, typename NodeProxy>
    static inline void apply(Elements & elements, const size_t axis, const size_t index, NodeProxy const& node_proxy)
    {
        BOOST_GEOMETRY_INDEX_ASSERT(axis == 0, "unexpected axis value");

        typedef typename Elements::value_type element_type;
        element_axis_corner_less<element_type, NodeProxy, Corner, 0> less(node_proxy);
        std::partial_sort(elements.begin(), elements.begin() + index, elements.end(), less);
    }
};

} // namespace rstar

template <typename Value, typename NodeProxy>
struct redistribute_elements<Value, NodeProxy, rstar_tag>
{
    typedef typename NodeProxy::node node;
    typedef typename NodeProxy::internal_node internal_node;
    typedef typename NodeProxy::leaf leaf;
    typedef typename NodeProxy::box_type box_type;

    static const size_t dimension = index::traits::dimension<box_type>::value;

    typedef typename index::default_margin_result<box_type>::type margin_type;
    typedef typename index::default_content_result<box_type>::type content_type;

    template <typename Node>
    static inline void apply(
        Node & n,
        Node & second_node,
        box_type & box1,
        box_type & box2,
        NodeProxy & node_proxy)
    {
        typedef typename rtree::elements_type<Node>::type elements_type;
        
        elements_type & elements1 = rtree::elements(n);
        elements_type & elements2 = rtree::elements(second_node);

        size_t split_axis = 0;
        size_t split_corner = 0;
        size_t split_index = node_proxy.parameters().get_min_elements();
        margin_type smallest_sum_of_margins = (std::numeric_limits<margin_type>::max)();
        content_type smallest_overlap = (std::numeric_limits<content_type>::max)();
        content_type smallest_content = (std::numeric_limits<content_type>::max)();

        rstar::choose_split_axis_and_index<
            NodeProxy,
            index::traits::dimension<box_type>::value
        >::apply(elements1,
            split_axis, split_corner, split_index,
            smallest_sum_of_margins, smallest_overlap, smallest_content,
            node_proxy);

        // TODO: awulkiew - get rid of following static_casts?

        BOOST_GEOMETRY_INDEX_ASSERT(split_axis < index::traits::dimension<box_type>::value, "unexpected value");
        BOOST_GEOMETRY_INDEX_ASSERT(split_corner == static_cast<size_t>(min_corner) || split_corner == static_cast<size_t>(max_corner), "unexpected value");
        BOOST_GEOMETRY_INDEX_ASSERT(node_proxy.parameters().get_min_elements() <= split_index &&
                                    split_index <= node_proxy.parameters().get_max_elements() - node_proxy.parameters().get_min_elements() + 1,
                                    "unexpected value");
        
        // TODO: awulkiew - check if std::partial_sort produces the same result as std::sort
        if ( split_corner == static_cast<size_t>(min_corner) )
            rstar::partial_sort<min_corner, dimension>::apply(elements1, split_axis, split_index, node_proxy);
        else
            rstar::partial_sort<max_corner, dimension>::apply(elements1, split_axis, split_index, node_proxy);

        // copy elements to node 2 and remove from node 1
        elements2.resize(node_proxy.parameters().get_max_elements() + 1 - split_index);
        std::copy(elements1.begin() + split_index, elements1.end(), elements2.begin());
        elements1.resize(split_index);

        // calculate boxes
        box1 = node_proxy.elements_box(elements1.begin(), elements1.end());
        box2 = node_proxy.elements_box(elements2.begin(), elements2.end());
    }
};

} // namespace detail

}}} // namespace detail::rtree::visitors

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_RSTAR_REDISTRIBUTE_ELEMENTS_HPP
