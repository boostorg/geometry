#include <iostream>

#include <boost/type_traits/is_base_of.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/vector.hpp>

#define BOOST_GEOMETRY_IMPLEMENTATION_STATUS_BUILD true
#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/algorithms/distance.hpp>
#include <boost/geometry/strategies/cartesian/distance_pythagoras.hpp>


typedef boost::geometry::cs::cartesian cartesian;

typedef boost::geometry::model::point<double, 2, cartesian> point_type;
typedef boost::geometry::model::linestring<point_type>      line_type;
typedef boost::geometry::model::polygon<point_type>         polygon_type;
typedef boost::geometry::model::box<point_type>             box_type;
typedef boost::geometry::model::ring<point_type>            ring_type;
typedef boost::geometry::model::segment<point_type>         segment_type;

typedef boost::mpl::vector<
    point_type,
    line_type,
    polygon_type,
    box_type,
    ring_type,
    segment_type
> types;


template <typename G1, typename G2>
struct check_distance
  : boost::geometry::dispatch::distance<
        G1,
        G2,
        boost::geometry::strategy_tag_distance_point_point,
        typename boost::geometry::strategy::distance::services::default_strategy<
            boost::geometry::point_tag,
            G1,
            G2
        >::type
    >
{};

template <typename G1, typename G2>
struct check_convert: boost::geometry::dispatch::convert<G1, G2>
{};


template <typename T1>
struct distance_tester
{
    template <typename T2>
    void operator()(T2)
    {
        if (boost::is_base_of<boost::geometry::not_implemented_base, check_distance<T1, T2> >::type::value)
        {
            std::cout << "-\t";
        }
        else
        {
            std::cout << "OK\t";
        }
    }
};

template <>
struct distance_tester<void>
{
    template <typename T>
    void operator()(T)
    {
        boost::mpl::for_each<types>(distance_tester<T>());
        std::cout << std::endl;
    }
};


template <typename T1>
struct convert_tester
{
    template <typename T2>
    void operator()(T2)
    {
        if (boost::is_base_of<boost::geometry::not_implemented_base, check_convert<T1, T2> >::type::value)
        {
            std::cout << "-\t";
        }
        else
        {
            std::cout << "OK\t";
        }
    }
};

template <>
struct convert_tester<void>
{
    template <typename T>
    void operator()(T)
    {
        boost::mpl::for_each<types>(convert_tester<T>());
        std::cout << std::endl;
    }
};


int main()
{
    std::cout << "DISTANCE" << std::endl;
    boost::mpl::for_each<types>(distance_tester<void>());
    std::cout << std::endl;

    std::cout << "CONVERT" << std::endl;
    boost::mpl::for_each<types>(convert_tester<void>());
    std::cout << std::endl;

    return 0;
}
