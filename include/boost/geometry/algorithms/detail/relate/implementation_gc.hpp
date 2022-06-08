// Boost.Geometry

// Copyright (c) 2022 Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_IMPLEMENTATION_GC_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_IMPLEMENTATION_GC_HPP


#include <boost/geometry/algorithms/detail/relate/interface.hpp>
#include <boost/geometry/algorithms/difference.hpp>
#include <boost/geometry/algorithms/intersection.hpp>
#include <boost/geometry/algorithms/is_empty.hpp>
#include <boost/geometry/algorithms/union.hpp>
#include <boost/geometry/geometries/linestring.hpp>
#include <boost/geometry/geometries/multi_linestring.hpp>
#include <boost/geometry/geometries/multi_point.hpp>
#include <boost/geometry/geometries/multi_polygon.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/views/detail/geometry_collection_view.hpp>


namespace boost { namespace geometry {


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch {

template <typename Geometry1, typename Geometry2>
struct relate<Geometry1, Geometry2, geometry_collection_tag, geometry_collection_tag, -1, -1, false>
{
    static const bool interruption_enabled = true;

    using mpt1_found_t = typename util::sequence_find_if
        <
            typename traits::geometry_types<Geometry1>::type,
            util::is_multi_point
        >::type;
    using mls1_found_t = typename util::sequence_find_if
        <
            typename traits::geometry_types<Geometry1>::type,
            util::is_multi_linestring
        >::type;
    using mpo1_found_t = typename util::sequence_find_if
        <
            typename traits::geometry_types<Geometry1>::type,
            util::is_multi_polygon
        >::type;
    using pt1_t = typename geometry::point_type<Geometry1>::type;
    using mpt1_t = std::conditional_t
        <
            std::is_void<mpt1_found_t>::value,
            geometry::model::multi_point<pt1_t>,
            mpt1_found_t
        >;
    using mls1_t = std::conditional_t
        <
            std::is_void<mls1_found_t>::value,
            geometry::model::multi_linestring<geometry::model::linestring<pt1_t>>,
            mls1_found_t
        >;
    using mpo1_t = std::conditional_t
        <
            std::is_void<mpo1_found_t>::value,
            geometry::model::multi_polygon<geometry::model::polygon<pt1_t>>,
            mpo1_found_t
        >;
    using tuple1_t = boost::tuple<mpt1_t, mls1_t, mpo1_t>;

    using mpt2_found_t = typename util::sequence_find_if
        <
            typename traits::geometry_types<Geometry2>::type,
            util::is_multi_point
        >::type;
    using mls2_found_t = typename util::sequence_find_if
        <
            typename traits::geometry_types<Geometry2>::type,
            util::is_multi_linestring
        >::type;
    using mpo2_found_t = typename util::sequence_find_if
        <
            typename traits::geometry_types<Geometry2>::type,
            util::is_multi_polygon
        >::type;
    using pt2_t = typename geometry::point_type<Geometry2>::type;
    using mpt2_t = std::conditional_t
        <
            std::is_void<mpt2_found_t>::value,
            geometry::model::multi_point<pt2_t>,
            mpt2_found_t
        >;
    using mls2_t = std::conditional_t
        <
            std::is_void<mls2_found_t>::value,
            geometry::model::multi_linestring<geometry::model::linestring<pt2_t>>,
            mls2_found_t
        >;
    using mpo2_t = std::conditional_t
        <
            std::is_void<mpo2_found_t>::value,
            geometry::model::multi_polygon<geometry::model::polygon<pt2_t>>,
            mpo2_found_t
        >;
    using tuple2_t = boost::tuple<mpt2_t, mls2_t, mpo2_t>;

    template <typename Geometry>
    using kind_id = util::index_constant
        <
            util::is_areal<Geometry>::value ? 2
          : util::is_linear<Geometry>::value ? 1
          : 0
        >;

    template <typename Result, typename Strategy>
    static inline void apply(Geometry1 const& geometry1, Geometry2 const& geometry2,
                             Result & result,
                             Strategy const& strategy)
    {
        using gc1_view_t = detail::random_access_view<Geometry1 const>;
        using gc2_view_t = detail::random_access_view<Geometry2 const>;
        gc1_view_t const gc1_view(geometry1);
        gc2_view_t const gc2_view(geometry2);

        bool inters_found[2][3] = {{false, false, false}, {false, false, false}};
        bool disjoint_found[2][3] = {{false, false, false}, {false, false, false}};
        bool disjoint_linear_boundary_found[2] = {false, false};
        bool has_disjoint = false;
        bool has_disjoint_linear = false;

        detail::gc_group_elements(gc1_view, gc2_view, strategy,
            [&](auto const& inters_group)
            {
                tuple1_t tuple1;
                tuple2_t tuple2;

                for (auto const& id : inters_group)
                {
                    BOOST_GEOMETRY_ASSERT(id.source_id == 0 || id.source_id == 1);
                    if (id.source_id == 0)
                    {
                        traits::iter_visit<gc1_view_t>::apply([&](auto const& g1)
                        {
                            merge_geometry(tuple1, g1, strategy);
                        }, boost::begin(gc1_view) + id.gc_id);
                    }
                    else
                    {
                        traits::iter_visit<gc2_view_t>::apply([&](auto const& g2)
                        {
                            merge_geometry(tuple2, g2, strategy);
                        }, boost::begin(gc2_view) + id.gc_id);
                    }
                }

                subtract_elements(tuple1, strategy);
                subtract_elements(tuple2, strategy);

                bool is_ll_handled = false;
                if (! geometry::is_empty(boost::get<1>(tuple1))
                    && ! geometry::is_empty(boost::get<1>(tuple2)))
                {
                    inters_found[0][1] = true;
                    inters_found[1][1] = true;
                    mls2_t mls2_diff_mpo1;
                    geometry::difference(boost::get<1>(tuple2), boost::get<2>(tuple1), mls2_diff_mpo1);
                    mls1_t mls1_diff_mpo2;
                    geometry::difference(boost::get<1>(tuple1), boost::get<2>(tuple2), mls1_diff_mpo2);
                    if (! geometry::is_empty(mls2_diff_mpo1)
                        && ! geometry::is_empty(mls1_diff_mpo2))
                    {
                        is_ll_handled = true;
                        call_relate(mls1_diff_mpo2, mls2_diff_mpo1, result, strategy);
                    }
                }
                
                if (! geometry::is_empty(boost::get<2>(tuple1)))
                {
                    inters_found[0][2] = true;
                    if (! geometry::is_empty(boost::get<2>(tuple2)))
                    {
                        inters_found[1][2] = true;
                        call_relate(boost::get<2>(tuple1), boost::get<2>(tuple2), result, strategy);
                    }
                    if (! geometry::is_empty(boost::get<1>(tuple2)))
                    {
                        inters_found[1][1] = true;
                        if (is_ll_handled)
                        {
                            mls1_t mls2_inters_mpo1;
                            geometry::intersection(boost::get<1>(tuple2), boost::get<2>(tuple1), mls2_inters_mpo1);
                            if (! geometry::is_empty(mls2_inters_mpo1))
                            {
                                call_relate(boost::get<2>(tuple1), mls2_inters_mpo1, result, strategy);
                            }
                        }
                        else
                        {
                            call_relate(boost::get<2>(tuple1), boost::get<1>(tuple2), result, strategy);
                        }
                    }
                    if (! geometry::is_empty(boost::get<0>(tuple2)))
                    {
                        inters_found[1][0] = true;
                        call_relate(boost::get<2>(tuple1), boost::get<0>(tuple2), result, strategy);
                    }
                }
                if (! geometry::is_empty(boost::get<1>(tuple1)))
                {
                    inters_found[0][1] = true;
                    if (! geometry::is_empty(boost::get<2>(tuple2)))
                    {
                        inters_found[1][2] = true;
                        if (is_ll_handled)
                        {
                            mls1_t mls1_inters_mpo2;
                            geometry::intersection(boost::get<1>(tuple1), boost::get<2>(tuple2), mls1_inters_mpo2);
                            if (! geometry::is_empty(mls1_inters_mpo2))
                            {
                                call_relate(mls1_inters_mpo2, boost::get<2>(tuple2), result, strategy);
                            }
                        }
                        else
                        {
                            call_relate(boost::get<1>(tuple1), boost::get<2>(tuple2), result, strategy);
                        }
                    }
                    if (! geometry::is_empty(boost::get<0>(tuple2)))
                    {
                        inters_found[1][0] = true;
                        call_relate(boost::get<1>(tuple1), boost::get<0>(tuple2), result, strategy);
                    }
                }
                if (! geometry::is_empty(boost::get<0>(tuple1)))
                {
                    inters_found[0][0] = true;
                    if (! geometry::is_empty(boost::get<2>(tuple2)))
                    {
                        inters_found[1][2] = true;
                        call_relate(boost::get<0>(tuple1), boost::get<2>(tuple2), result, strategy);
                    }
                    if (! geometry::is_empty(boost::get<1>(tuple2)))
                    {
                        inters_found[1][1] = true;
                        call_relate(boost::get<0>(tuple1), boost::get<1>(tuple2), result, strategy);
                    }
                    if (! geometry::is_empty(boost::get<0>(tuple2)))
                    {
                        inters_found[1][0] = true;
                        call_relate(boost::get<0>(tuple1), boost::get<0>(tuple2), result, strategy);
                    }
                }
            },
            [&](auto const& disjoint_group)
            {
                for (auto const& id : disjoint_group)
                {
                    BOOST_GEOMETRY_ASSERT(id.source_id == 0 || id.source_id == 1);
                    if (id.source_id == 0)
                    {
                        traits::iter_visit<gc1_view_t>::apply([&](auto const& g1)
                        {
                            if (! geometry::is_empty(g1))
                            {
                                static const std::size_t index = kind_id<util::remove_cref_t<decltype(g1)>>::value;
                                disjoint_found[0][index] = true;
                                disjoint_linear_boundary_found[0] = has_linear_boundary(g1, strategy);
                                has_disjoint = true;
                            }
                        }, boost::begin(gc1_view) + id.gc_id);
                    }
                    else
                    {
                        traits::iter_visit<gc2_view_t>::apply([&](auto const& g2)
                        {
                            if (! geometry::is_empty(g2))
                            {
                                static const std::size_t index = kind_id<util::remove_cref_t<decltype(g2)>>::value;
                                disjoint_found[1][index] = true;
                                disjoint_linear_boundary_found[1] = has_linear_boundary(g2, strategy);
                                has_disjoint = true;
                            }
                        }, boost::begin(gc2_view) + id.gc_id);
                    }
                }
            }, true);

        // Based on found disjoint geometries as well as those intersecting set exteriors
        if (has_disjoint)
        {
            using namespace detail::relate;

            if (disjoint_found[0][2] == true)
            {
                update<interior, exterior, '2'>(result);
                update<boundary, exterior, '1'>(result);
            }
            else if (disjoint_found[0][1] == true)
            {
                update<interior, exterior, '1'>(result);
                if (disjoint_linear_boundary_found[0])
                {
                    update<boundary, exterior, '0'>(result);
                }
            }
            else if (disjoint_found[0][0] == true)
            {
                update<interior, exterior, '0'>(result);
            }

            if (disjoint_found[1][2] == true)
            {
                update<exterior, interior, '2'>(result);
                update<exterior, boundary, '1'>(result);
            }
            else if (disjoint_found[1][1] == true)
            {
                update<exterior, interior, '1'>(result);
                if (disjoint_linear_boundary_found[1])
                {
                    update<exterior, boundary, '0'>(result);
                }
            }
            else if (disjoint_found[1][0] == true)
            {
                update<exterior, interior, '0'>(result);
            }
        }
    }

private:
    template <typename Tuple, typename Geometry, typename Strategy>
    static inline void merge_geometry(Tuple& tuple, Geometry const& geometry, Strategy const& strategy)
    {
        static const std::size_t index = kind_id<Geometry>::value;
        typename boost::tuples::element<index, Tuple>::type temp_out;
        geometry::union_(boost::get<index>(tuple), geometry, temp_out, strategy);
        boost::get<index>(tuple) = std::move(temp_out);
    }

    template <typename Tuple, typename Strategy>
    static inline void subtract_elements(Tuple& tuple, Strategy const& strategy)
    {
        if (! geometry::is_empty(boost::get<1>(tuple)))
        {
            if (! geometry::is_empty(boost::get<2>(tuple)))
            {
                typename boost::tuples::element<1, Tuple>::type mls;
                geometry::difference(boost::get<1>(tuple), boost::get<2>(tuple), mls, strategy);
                boost::get<1>(tuple) = std::move(mls);
            }
        }
        if (! geometry::is_empty(boost::get<0>(tuple)))
        {
            if (! geometry::is_empty(boost::get<2>(tuple)))
            {
                typename boost::tuples::element<0, Tuple>::type mpt;
                geometry::difference(boost::get<0>(tuple), boost::get<2>(tuple), mpt, strategy);
                boost::get<0>(tuple) = std::move(mpt);
            }
            if (! geometry::is_empty(boost::get<1>(tuple)))
            {
                typename boost::tuples::element<0, Tuple>::type mpt;
                geometry::difference(boost::get<0>(tuple), boost::get<1>(tuple), mpt, strategy);
                boost::get<0>(tuple) = std::move(mpt);
            }
        }
    }

    template
    <
        typename Geometry, typename Strategy,
        std::enable_if_t<util::is_linear<Geometry>::value, int> = 0
    >
    static inline bool has_linear_boundary(Geometry const& geometry, Strategy const& strategy)
    {
        detail::relate::topology_check<Geometry, Strategy> tc(geometry, strategy);
        return tc.has_boundary();
    }

    template
    <
        typename Geometry, typename Strategy,
        std::enable_if_t<! util::is_linear<Geometry>::value, int> = 0
    >
    static inline bool has_linear_boundary(Geometry const& , Strategy const& )
    {
        return false;
    }


    template <typename Multi1, typename Multi2, typename Result, typename Strategy>
    static inline void call_relate(Multi1 const& multi1, Multi2 const& multi2,
                                   Result& result, Strategy const& strategy)
    {
        dispatch::relate
            <
                Multi1, Multi2
            >::apply(multi1, multi2, result, strategy);
    }
};


template <typename Geometry1, typename Geometry2, typename Tag1, int TopDim1>
struct relate<Geometry1, Geometry2, Tag1, geometry_collection_tag, TopDim1, -1, false>
{
    static const bool interruption_enabled = true;

    template <typename Result, typename Strategy>
    static inline void apply(Geometry1 const& geometry1, Geometry2 const& geometry2,
                             Result & result,
                             Strategy const& strategy)
    {
        using gc1_view_t = geometry::detail::geometry_collection_view<Geometry1>;
        relate<gc1_view_t, Geometry2>::apply(gc1_view_t(geometry1), geometry2, result, strategy);
    }
};

template <typename Geometry1, typename Geometry2, typename Tag2, int TopDim2>
struct relate<Geometry1, Geometry2, geometry_collection_tag, Tag2, -1, TopDim2, false>
{
    static const bool interruption_enabled = true;

    template <typename Result, typename Strategy>
    static inline void apply(Geometry1 const& geometry1, Geometry2 const& geometry2,
                             Result & result,
                             Strategy const& strategy)
    {
        using gc2_view_t = geometry::detail::geometry_collection_view<Geometry2>;
        relate<Geometry1, gc2_view_t>::apply(geometry1, gc2_view_t(geometry2), result, strategy);
    }
};

} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_IMPLEMENTATION_HPP
