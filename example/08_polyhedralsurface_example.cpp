// Boost.Geometry (aka GGL, Generic Geometry Library)

// Polyhedral surface example

#include <iostream>
#include <boost/geometry/geometry.hpp>

int main()
{
    using namespace boost::geometry;
    using point_t = model::point<double, 3, cs::cartesian>;
    using ring_t = model::ring<point_t>;
    using polyhedral_t = model::polyhedral_surface<ring_t>;

    // intializing an empty polyhedral surface (deafault constructor)
    polyhedral_t polyhedral2;

    // creating a polyhderal surface using standard initiallized list
    polyhedral_t polyhedral1 = {{{0,0,0}, {0, 1, 0}, {1, 1, 0}, {1, 0, 0}, {0, 0, 0}}, {{0, 0, 0}, {0, 1, 0}, {0, 1, 1}, {0, 0, 1}, {0, 0, 0}},
    {{0, 0, 0}, {1, 0, 0}, {1, 0, 1}, {0, 0, 1}, {0, 0, 0}}, {{1, 1, 1}, {1, 0, 1}, {0, 0, 1}, {0, 1, 1}, {1, 1, 1}}, {{1, 1, 1}, {1, 0, 1}, {1, 0, 0}, {1, 1, 0}, {1, 1, 1}}, 
    {{1, 1, 1}, {1, 1, 0}, {0, 1, 0}, {0, 1, 1}, {1, 1, 1}}};

    // modifying a polyhedral surface
    polyhedral1[0][1] = {1, 1, 1};

    // read polyhedral surface wkt
    read_wkt("POLYHEDRALSURFACE(((0 0 0, 0 1 0, 1 1 0, 1 0 0, 0 0 0)),((0 0 0, 0 1 0, 0 1 1, 0 0 1, 0 0 0)),((0 0 0, 1 0 0, 1 0 1, 0 0 1, 0 0 0)),((1 1 1, 1 0 1, 0 0 1, 0 1 1, 1 1 1)),((1 1 1, 1 0 1, 1 0 0, 1 1 0, 1 1 1)),((1 1 1, 1 1 0, 0 1 0, 0 1 1, 1 1 1)))", polyhedral2);

    // write polyhedral surface wkt
    std::cout << wkt(polyhedral1) << std::endl;

    std::cout << wkt(polyhedral2) << std::endl;

    // clear polyhedral surface
    clear(polyhedral1);
}
