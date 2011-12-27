#include <iostream>
#include <string>
#include <vector>

#include <boost/type_traits/is_base_of.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/vector.hpp>

#define BOOST_GEOMETRY_IMPLEMENTATION_STATUS_BUILD true
#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/algorithms/append.hpp>
#include <boost/geometry/algorithms/convert.hpp>
#include <boost/geometry/algorithms/distance.hpp>
#include <boost/geometry/strategies/cartesian/distance_pythagoras.hpp>


typedef boost::geometry::cs::cartesian cartesian;

typedef boost::geometry::model::point<double, 2, cartesian> point_type;
typedef boost::geometry::model::linestring<point_type>      linestring_type;
typedef boost::geometry::model::polygon<point_type>         polygon_type;
typedef boost::geometry::model::box<point_type>             box_type;
typedef boost::geometry::model::ring<point_type>            ring_type;
typedef boost::geometry::model::segment<point_type>         segment_type;

typedef boost::mpl::vector<
    point_type,
    linestring_type,
    polygon_type,
    box_type,
    ring_type,
    segment_type
> all_types;


#define DECLARE_BINARY_ALGORITHM(algorithm) \
    template <typename G1, typename G2> \
    struct algorithm: boost::geometry::dispatch::algorithm<G1, G2> \
    {};

DECLARE_BINARY_ALGORITHM(append)
DECLARE_BINARY_ALGORITHM(distance)
DECLARE_BINARY_ALGORITHM(convert)


template <template <typename, typename> class Dispatcher, typename G2>
struct do_test
{
    template <typename G1>
    void operator()(G1)
    {
        if (boost::is_base_of<boost::geometry::nyi::not_implemented_tag, Dispatcher<G1, G2> >::type::value)
        {
            std::cout << "-\t";
        }
        else
        {
            std::cout << "OK\t";
        }
    }
};

template <template <typename, typename> class Dispatcher, typename Types>
struct test
{
    template <typename G2>
    void operator()(G2)
    {
        boost::mpl::for_each<Types>(do_test<Dispatcher, G2>());
        std::cout << std::endl;
    }
};


template<template <typename, typename> class Dispatcher, typename Types1, typename Types2>
void test_binary_algorithm(const std::string& name)
{
    std::cout << name << std::endl;
    boost::mpl::for_each<Types1>(test<Dispatcher, Types2>());
    std::cout << std::endl;
}


int main()
{
    test_binary_algorithm<append,   all_types, boost::mpl::vector<point_type, std::vector<point_type> > >("APPEND");
    test_binary_algorithm<convert,  all_types, all_types>("CONVERT");
    test_binary_algorithm<distance, all_types, all_types>("DISTANCE");

    return 0;
}
