// Boost.Geometry Index
// Unit Test

// Copyright (c) 2011-2013 Adam Wulkiewicz, Lodz, Poland.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <rtree/test_rtree.hpp>

#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/allocators/allocator.hpp>

template <typename Point, typename Parameters>
void test_rtree_interprocess(Parameters const& parameters = Parameters())
{
    namespace bi = boost::interprocess;
    struct shm_remove
    {
        shm_remove() { bi::shared_memory_object::remove("shmem"); }
        ~shm_remove(){ bi::shared_memory_object::remove("shmem"); }
    } remover;

    bi::managed_shared_memory segment(bi::create_only, "shmem", 65535);
    typedef bi::allocator<Point, bi::managed_shared_memory::segment_manager> shmem_alloc;

    test_rtree_for_box<Point>(parameters, shmem_alloc(segment.get_segment_manager()));

    //typedef bgi::rtree<Value, Parameters, bgi::translator::def<Value>, shmem_alloc> Tree;

    //Tree * tree = segment.construct<Tree>("Tree")(parameters, bgi::translator::def<Value>(), shmem_alloc(segment.get_segment_manager()));
    //typedef typename Tree::bounds_type B;
    //
    //std::vector<Value> input;
    //B qbox;
    //
    //generate_rtree(*tree, input, qbox);

    //test_intersects(*tree, input, qbox);
    //test_disjoint(*tree, input, qbox);
    //test_covered_by(*tree, input, qbox);
    //test_overlaps(*tree, input, qbox);
    ////test_touches(*tree, input, qbox);
    //test_within(*tree, input, qbox);

    //typedef typename bgi::detail::traits::point_type<B>::type P;
    //P pt;
    //bg::centroid(qbox, pt);

    //test_nearest_query_k(*tree, input, pt, 10);
    //test_nearest_query_not_found(*tree, generate_outside_point<P>::apply());

    //segment.destroy_ptr(tree);
}
