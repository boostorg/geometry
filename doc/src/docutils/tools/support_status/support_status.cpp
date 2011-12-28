// Boost.Geometry (aka GGL, Generic Geometry Library)
// Tool reporting Implementation Support Status in QBK or plain text format

// Copyright (c) 2011 Bruno Lalande, Paris, France.
// Copyright (c) 2011 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <fstream>
#include <sstream>

#include <boost/type_traits/is_base_of.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/vector.hpp>

#define BOOST_GEOMETRY_IMPLEMENTATION_STATUS_BUILD true
#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/multi/geometries/multi_geometries.hpp>
#include <boost/geometry/multi/multi.hpp>
#include <boost/geometry/algorithms/append.hpp>
#include <boost/geometry/algorithms/convert.hpp>
#include <boost/geometry/algorithms/distance.hpp>
#include <boost/geometry/strategies/cartesian/distance_pythagoras.hpp>

#include "text_outputter.hpp"
#include "qbk_outputter.hpp"

typedef boost::geometry::cs::cartesian cartesian;

typedef boost::geometry::model::point<double, 2, cartesian> point_type;
typedef boost::geometry::model::linestring<point_type>      linestring_type;
typedef boost::geometry::model::polygon<point_type>         polygon_type;
typedef boost::geometry::model::box<point_type>             box_type;
typedef boost::geometry::model::ring<point_type>            ring_type;
typedef boost::geometry::model::segment<point_type>         segment_type;

typedef boost::geometry::model::multi_point<point_type>           multi_point_type;
typedef boost::geometry::model::multi_linestring<linestring_type> multi_linestring_type;
typedef boost::geometry::model::multi_polygon<polygon_type>       multi_polygon_type;

typedef boost::mpl::vector<
    point_type,
    segment_type,
    box_type,
    linestring_type,
    ring_type,
    polygon_type,
    multi_point_type,
    multi_linestring_type,
    multi_polygon_type
> all_types;

#define DECLARE_BINARY_ALGORITHM(algorithm) \
    template <typename G1, typename G2> \
    struct algorithm: boost::geometry::dispatch::algorithm<G1, G2> \
    {};

DECLARE_BINARY_ALGORITHM(append)
DECLARE_BINARY_ALGORITHM(distance)
DECLARE_BINARY_ALGORITHM(convert)


template <template <typename, typename> class Dispatcher, typename Outputter, typename G2 = void>
struct do_test
{
    Outputter& m_outputter;
    inline do_test(Outputter& outputter)
        : m_outputter(outputter)
    {}

    template <typename G1>
    void operator()(G1)
    {
        if (boost::is_base_of<boost::geometry::nyi::not_implemented_tag, Dispatcher<G1, G2> >::type::value)
        {
            m_outputter.nyi();
        }
        else
        {
            m_outputter.ok();
        }
    }
};

template <template <typename, typename> class Dispatcher, typename Types, typename Outputter>
struct test
{
    Outputter& m_outputter;
    inline test(Outputter& outputter)
        : m_outputter(outputter)
    {}

    template <typename G2>
    void operator()(G2)
    {
         m_outputter.template begin_row<G2>();
         boost::mpl::for_each<Types>(do_test<Dispatcher, Outputter, G2>(m_outputter));
         m_outputter.end_row();
    }
};

template <template <typename, typename> class Dispatcher, typename Types1, typename Types2, typename Outputter>
void test_binary_algorithm(std::string const& name)
{
    Outputter outputter(name);
    outputter.header(name);

    outputter.template table_header<Types2>();
    boost::mpl::for_each<Types1>(test<Dispatcher, Types2, Outputter>(outputter));

    outputter.table_footer();
}


template <typename OutputFactory>
void support_status()
{
    test_binary_algorithm<append, all_types, boost::mpl::vector<point_type, std::vector<point_type> >, OutputFactory>("append");
    test_binary_algorithm<distance, all_types, all_types, OutputFactory>("distance");
    test_binary_algorithm<convert, all_types, all_types, OutputFactory>("convert");
}


int main(int argc, char** argv)
{
    if (argc > 1 && ! strcmp(argv[1], "qbk"))
    {
        support_status<qbk_outputter>();
    }
    else
    {
        support_status<text_outputter>();
    }
    return 0;
}
