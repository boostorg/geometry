// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2013 Adam Wulkiewicz, Lodz, Poland.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_ALGEBRA_ALGORITHMS_ROTATION_HPP
#define BOOST_GEOMETRY_EXTENSIONS_ALGEBRA_ALGORITHMS_ROTATION_HPP

#include <boost/geometry/extensions/algebra/geometries/concepts/rotation_quaternion_concept.hpp>

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch {

template <typename V1, typename V2, typename Rotation,
          typename Tag = typename tag<V1>::type,
          typename RTag = typename tag<Rotation>::type
>
struct rotation
{
    BOOST_MPL_ASSERT_MSG(false, NOT_IMPLEMENTED_FOR_THOSE_TAGS, (Tag, Rotation));
};

template <typename V1, typename V2, typename Rotation>
struct rotation<V1, V2, Rotation, vector_tag, rotation_quaternion_tag>
{
    static const bool cs_check =
        ::boost::is_same<typename traits::coordinate_system<V1>::type, cs::cartesian>::value &&
        ::boost::is_same<typename traits::coordinate_system<V2>::type, cs::cartesian>::value;

    BOOST_MPL_ASSERT_MSG(cs_check, NOT_IMPLEMENTED_FOR_THOSE_SYSTEMS, (V1, V2));

    typedef typename select_most_precise<
        typename traits::coordinate_type<V1>::type,
        typename traits::coordinate_type<V2>::type
    >::type cv_type;

    typedef typename select_most_precise<
        cv_type,
        typename traits::coordinate_type<Rotation>::type
    >::type cr_type;

    typedef model::vector<cv_type, 3> vector_type;

    inline static void apply(V1 const& v1, V2 const& v2, Rotation & r)
    {
        // TODO - should store coordinates in more precise variables before the normalization?

        // half angle
        cv_type d = dot(v1, v2);
        cv_type w = ::sqrt(dot(v1, v1) * dot(v2, v2)) + d;

        // rotation angle 0 or pi
        if ( -std::numeric_limits<cv_type>::epsilon() <= w && w <= std::numeric_limits<cv_type>::epsilon() )
        {
            // rotation angle == 0
            if ( 0 <= d )
            {
                set<0>(r, 1); set<0>(r, 0); set<0>(r, 0); set<0>(r, 0);
            }
            // rotation angle == pi
            else
            {
                set<0>(r, 0);
                // find arbitrary rotation axis perpendicular to v1
                assign_cross(vector_type(1, 0, 0), v1, r);
                if ( length_sqr(r) < std::numeric_limits<cr_type>::epsilon() )
                    assign_cross(vector_type(0, 1, 0), v1, r);

                // normalize axis
                cr_type lsqr = length_sqr(r);
                if ( std::numeric_limits<cr_type>::epsilon() < lsqr )
                    scale(r, 1.0f / ::sqrt(lsqr));
            }
        }
        else
        {
            set<0>(r, w);
            // rotation axis
            assign_cross(v1, v2, r);

            // normalize
            cr_type lsqr = length_sqr(r);
            if ( std::numeric_limits<cr_type>::epsilon() < lsqr )
                scale(r, 1.0f / ::sqrt(lsqr));
        }
    }

    // TODO - should return more precise type?
    template <typename V1, typename V2>
    inline static cv_type dot(V1 const& v1, V2 const& v2)
    {
        return get<0>(v1)*get<0>(v2) + get<1>(v1)*get<1>(v2) + get<2>(v1)*get<2>(v2);
    }

    // TODO - should return more precise type?
    inline static cr_type length_sqr(Rotation const& r)
    {
        return get<0>(r)*get<0>(r) + get<1>(r)*get<1>(r) + get<2>(r)*get<2>(r) + get<3>(r)*get<3>(r);
    }

    inline static void assign_cross(V1 const& v1, V2 const& v2, Rotation & r)
    {
        set<1>(r, get<1>(v1)*get<2>(v2) - get<2>(v1)*get<1>(v2));
        set<2>(r, get<2>(v1)*get<0>(v2) - get<0>(v1)*get<2>(v2));
        set<3>(r, get<0>(v1)*get<1>(v2) - get<1>(v1)*get<0>(v2));
    }

    inline static void scale(Rotation & r, cr_type const& v)
    {
        set<0>(r, get<0>(r) * v);
        set<1>(r, get<1>(r) * v);
        set<2>(r, get<2>(r) * v);
        set<3>(r, get<3>(r) * v);
    }
};

} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH

template <typename V1, typename V2, typename Rotation>
inline void rotation(V1 const& v1, V2 const& v2, Rotation & r)
{
    concept::check_concepts_and_equal_dimensions<V1 const, V2 const>();
    // TODO - replace the following by check_equal_dimensions
    concept::check_concepts_and_equal_dimensions<V1 const, Rotation>();

    dispatch::rotation<V1, V2, Rotation>::apply(v1, v2, r);
}

template <typename Rotation, typename V1, typename V2>
inline Rotation return_rotation(V1 const& v1, V2 const& v2)
{
    Rotation r;
    translation(v1, v2, r);
    return r;
}

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_EXTENSIONS_ALGEBRA_ALGORITHMS_ROTATION_HPP
