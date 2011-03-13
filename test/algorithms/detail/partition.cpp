// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels, Geodan B.V. Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <geometry_test_common.hpp>

#include <algorithms/test_overlay.hpp>

#include <boost/range/algorithm/copy.hpp>

#include <boost/geometry/geometry.hpp>
#include <boost/geometry/algorithms/detail/overlay/select_rings.hpp>
#include <boost/geometry/algorithms/detail/overlay/assign_parents.hpp>
#include <boost/geometry/algorithms/detail/partition.hpp>

#include <boost/geometry/domains/gis/io/wkt/read_wkt.hpp>


template <typename Box>
struct sample_item
{
    int id;
    Box box;
    sample_item(int i = 0, std::string const& wkt = "")
        : id(i)
    {
        if (! wkt.empty())
        {
            bg::read_wkt(wkt, box);
        }
    }
};


struct get_box
{
    template <typename Box, typename InputItem>
    static inline void apply(Box& total, InputItem const& item)
    {
        bg::expand(total, item.box);
    }
};

struct ovelaps_box
{
    template <typename Box, typename InputItem>
    static inline bool apply(Box const& box, InputItem const& item)
    {
        return ! bg::detail::disjoint::disjoint_box_box(box, item.box);
    }
};


template <typename Box>
struct sample_visitor
{
    int count;
    typename bg::area_result<Box>::type area;

    sample_visitor()
        : count(0)
        , area(0)
    {}

    template <typename Item>
    inline void apply(Item const& item1, Item const& item2)
    {
        if (bg::intersects(item1.box, item2.box))
        {
            Box b;
            bg::intersection(item1.box, item2.box, b);
            area += bg::area(b);
            count++;
        }
    }
};



template <typename Box>
void test_geometry(std::string const& wkt_box_list, double expected_area, int expected_count)
{
    std::vector<std::string> wkt_boxes;
    
    boost::split(wkt_boxes, wkt_box_list, boost::is_any_of(";"), boost::token_compress_on);

    typedef sample_item<Box> sample;
    std::vector<sample> boxes;

    int index = 1;
    BOOST_FOREACH(std::string const& wkt, wkt_boxes)
    {
        boxes.push_back(sample(index++, wkt));
    }

    sample_visitor<Box> visitor;
    bg::partition
        <
            Box, get_box, ovelaps_box
        >::apply(boxes, visitor);

    BOOST_CHECK_CLOSE(visitor.area, expected_area, 0.001);
    BOOST_CHECK_EQUAL(visitor.count, expected_count);
}




template <typename P>
void test_all()
{
    typedef bg::model::box<P> box;

    test_geometry<box>(
        // 1           2             3               4             5             6             7
        "box(0 0,1 1); box(0 0,2 2); box(9 9,10 10); box(8 8,9 9); box(4 4,6 6); box(2 4,6 8); box(7 1,8 2)", 
        5, // Area(Intersection(1,2)) + A(I(5,6)) 
        3);

    test_geometry<box>(
        "box(0 0,10 10); box(4 4,6 6); box(3 3,7 7)", 
            4 + 16 + 4,  // A(I(1,2)) + A(I(1,3)) + A(I(2,3))
            3);

    test_geometry<box>(
        "box(0 2,10 3); box(3 1,4 5); box(7 1,8 5)", 
            1 + 1,  // A(I(1,2)) + A(I(1,3)) 
            2);
}




int test_main( int , char* [] )
{
    test_all<bg::model::d2::point_xy<double> >();

    return 0;
}
