// Boost.Geometry

// Copyright (c) 2021, Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ALGORITHMS_VISIT_HPP
#define BOOST_GEOMETRY_ALGORITHMS_VISIT_HPP

#include <deque>
#include <utility>

#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>

#include <boost/geometry/core/static_assert.hpp>
#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tags.hpp>
#include <boost/geometry/core/visit.hpp>
#include <boost/geometry/util/type_traits.hpp>

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template <typename Geometry, typename Tag = typename tag<Geometry>::type>
struct visit_one
{
    template <typename F, typename G>
    static void apply(F && f, G & g)
    {
        f(g);
    }
};

template <typename Geometry>
struct visit_one<Geometry, void>
{
    BOOST_GEOMETRY_STATIC_ASSERT_FALSE(
        "Not implemented for this Geometry type.",
        Geometry);
};

template <typename Geometry>
struct visit_one<Geometry, dynamic_geometry_tag>
{
    template <typename F, typename Geom>
    static void apply(F && function, Geom & geom)
    {
        traits::visit
            <
                util::remove_cref_t<Geom>
            >::apply(std::forward<F>(function), geom);
    }
};


template <typename Geometry1, typename Geometry2, typename Tag1 = typename tag<Geometry1>::type, typename Tag2 = typename tag<Geometry2>::type>
struct visit_two
{
    template <typename F, typename G1, typename G2>
    static void apply(F && f, G1 & geom1, G2 & geom2)
    {
        f(geom1, geom2);
    }
};

template <typename Geometry1, typename Geometry2, typename Tag2>
struct visit_two<Geometry1, Geometry2, void, Tag2>
{
    BOOST_GEOMETRY_STATIC_ASSERT_FALSE(
        "Not implemented for this Geometry1 type.",
        Geometry1);
};

template <typename Geometry1, typename Geometry2, typename Tag1>
struct visit_two<Geometry1, Geometry2, Tag1, void>
{
    BOOST_GEOMETRY_STATIC_ASSERT_FALSE(
        "Not implemented for this Geometry2 type.",
        Geometry2);
};

template <typename Geometry1, typename Geometry2>
struct visit_two<Geometry1, Geometry2, void, void>
{
    BOOST_GEOMETRY_STATIC_ASSERT_FALSE(
        "Not implemented for these types.",
        Geometry1, Geometry2);
};

template <typename Geometry1, typename Geometry2, typename Tag2>
struct visit_two<Geometry1, Geometry2, dynamic_geometry_tag, Tag2>
{
    template <typename F, typename G1, typename G2>
    static void apply(F && f, G1 & geom1, G2 & geom2)
    {
        traits::visit<util::remove_cref_t<G1>>::apply([&](auto & g1)
        {
            f(g1, geom2);
        }, geom1);
    }
};

template <typename Geometry1, typename Geometry2, typename Tag1>
struct visit_two<Geometry1, Geometry2, Tag1, dynamic_geometry_tag>
{
    template <typename F, typename G1, typename G2>
    static void apply(F && f, G1 & geom1, G2 & geom2)
    {
        traits::visit<util::remove_cref_t<G2>>::apply([&](auto & g2)
        {
            f(geom1, g2);
        }, geom2);
    }
};

template <typename Geometry1, typename Geometry2>
struct visit_two<Geometry1, Geometry2, dynamic_geometry_tag, dynamic_geometry_tag>
{
    template <typename F, typename G1, typename G2>
    static void apply(F && f, G1 & geom1, G2 & geom2)
    {
        traits::visit
            <
                util::remove_cref_t<G1>, util::remove_cref_t<G2>
            >::apply([&](auto & g1, auto & g2)
            {
                f(g1, g2);
            }, geom1, geom2);
    }
};


template <typename Geometry, typename Tag = typename tag<Geometry>::type>
struct visit_breadth_first
{
    template <typename F, typename G>
    static bool apply(F f, G & g)
    {
        return f(g);
    }
};

template <typename Geometry>
struct visit_breadth_first<Geometry, void>
{
    BOOST_GEOMETRY_STATIC_ASSERT_FALSE(
        "Not implemented for this Geometry type.",
        Geometry);
};

template <typename Geometry>
struct visit_breadth_first<Geometry, dynamic_geometry_tag>
{
    template <typename Geom, typename F>
    static bool apply(F function, Geom & geom)
    {
        bool result = true;
        traits::visit<util::remove_cref_t<Geom>>::apply([&](auto & g)
        {
            result = visit_breadth_first<decltype(g)>::apply(function, g);
        }, geom);
        return result;
    }
};

template <typename Geometry>
struct visit_breadth_first<Geometry, geometry_collection_tag>
{
    template <typename F, typename Geom>
    static bool apply(F function, Geom & geom)
    {
        using iter_t = typename boost::range_iterator<Geom>::type;
        std::deque<iter_t> queue;

        iter_t it = boost::begin(geom);
        iter_t end = boost::end(geom);
        for(;;)
        {
            for (; it != end; ++it)
            {
                bool result = true;
                traits::visit_iterator<util::remove_cref_t<Geom>>::apply([&](auto & g)
                {
                    result = visit_or_enqueue(function, g, queue, it);
                }, it);
                if (! result)
                    return false;
            }

            if (queue.empty())
            {
                break;
            }

            // Alternatively store a pointer to GeometryCollection
            // so this call can be avoided.
            traits::visit_iterator<util::remove_cref_t<Geom>>::apply([&](auto & g)
            {
                set_iterators(g, it, end);
            }, queue.front());
            queue.pop_front();
        }

        return true;
    }

private:
    template
    <
        typename F, typename Geom, typename Iterator,
        std::enable_if_t<util::is_geometry_collection<Geom>::value, int> = 0
    >
    static bool visit_or_enqueue(F &, Geom &, std::deque<Iterator> & queue, Iterator iter)
    {
        queue.push_back(iter);
        return true;
    }
    template
    <
        typename F, typename Geom, typename Iterator,
        std::enable_if_t<! util::is_geometry_collection<Geom>::value, int> = 0
    >
    static bool visit_or_enqueue(F & f, Geom & g, std::deque<Iterator> & , Iterator)
    {
        return f(g);
    }

    template
    <
        typename Geom, typename Iterator,
        std::enable_if_t<util::is_geometry_collection<Geom>::value, int> = 0
    >
    static void set_iterators(Geom & g, Iterator & first, Iterator & last)
    {
        first = boost::begin(g);
        last = boost::end(g);
    }
    template
    <
        typename Geom, typename Iterator,
        std::enable_if_t<!util::is_geometry_collection<Geom>::value, int> = 0
    >
    static void set_iterators(Geom &, Iterator &, Iterator &)
    {}
};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


// This function mimics std::visit
// This algorithm treats GeometryCollection as StaticGeometry and calls function
//   for it.
// TODO: We could change the order of the arguments but I'm not sure if that's a good idea.
template <typename UnaryFunction, typename Geometry>
inline void visit(UnaryFunction && function, Geometry & geometry)
{
    dispatch::visit_one
        <
            Geometry
        >::apply(std::forward<UnaryFunction>(function), geometry);
}


// This function mimics std::visit
// This algorithm treats GeometryCollection as StaticGeometry and calls function
//   for it.
// TODO: We could change the order of the arguments but I'm not sure if that's a good idea.
template <typename UnaryFunction, typename Geometry1, typename Geometry2>
inline void visit(UnaryFunction && function, Geometry1 & geometry1, Geometry2 & geometry2)
{
    dispatch::visit_two
        <
            Geometry1, Geometry2
        >::apply(std::forward<UnaryFunction>(function), geometry1, geometry2);
}


// This function works like combined std::visit and recursive std::for_each.
// This algorithm works for all Geometries. It visits DynamicGeometries and calls function for
//   StaticGeometries.
// The number of elements visited may be different than the size of the top-most GeometryCollection
//   if it's recursive.
// TODO: This algorithm is similar to std::for_each so it could be called e.g. std::visit_each or
//   std::for_each_visit but it doesn't work the same way, i.e. it traverses recursive
//   GeometryCollections.
// TODO: Should this algorithm work only for GeometryCollection?
// TODO: We could change the order of the arguments but I'm not sure if that's a good idea.
template <typename UnaryFunction, typename Geometry>
inline void visit_breadth_first(UnaryFunction function, Geometry & geometry)
{
    dispatch::visit_breadth_first<Geometry>::apply(function, geometry);
}


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_VISIT_HPP
