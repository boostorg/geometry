#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>

#include <vector>

namespace bg = boost::geometry;

using point = bg::model::d2::point_xy<double>;
using polygon = bg::model::polygon<point>;

int main()
{
    polygon p1, p2;

    bg::read_wkt(
        "POLYGON((-2.47089026 -86.03059246,"
        "-1.161944873 146.3030596,"
        "3.40221214 145.8628015,"
        "3.024695769 141.949088,"
        "2.420586453 111.9554564,"
        "1.208013978 -86.04069936,"
        "-2.47089026 -86.03059246))", p1);

    bg::read_wkt(
        "POLYGON((-6.213278056 -88.01851748,"
        "-6.207382255 -86.51852906,"
        "-6.20539951 -86.01408604,"
        "-2.470873099 -86.02434575,"
        "-2.472221358 -86.53315586,"
        "-2.476196085 -88.03315059,"
        "-6.213278056 -88.01851748))", p2);

    std::vector<polygon> output;
    bg::intersection(p1, p2, output);

    // Expected: no intersection
    return output.empty() ? 0 : 1;
}
