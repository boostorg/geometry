#include <boost/geometry.hpp>
#include <boost/geometry/strategies/relate/geographic.hpp>
#include <boost/geometry/strategies/line_interpolate/geographic.hpp>
#include <cmath>
#include <functional>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>

namespace bg = boost::geometry;
using S = bg::model::point<double, 2, bg::cs::spherical_equatorial<bg::degree>>;
using R = bg::model::point<double, 2, bg::cs::spherical_equatorial<bg::radian>>;
using G = bg::model::point<double, 2, bg::cs::geographic<bg::degree>>;
template<class P> using Line = bg::model::linestring<P>;
template<class P> using Multi = bg::model::multi_linestring<Line<P>>;
template<class P> using Box = bg::model::box<P>;
constexpr double pi = 3.1415926535897932384626433832795;

void require(bool condition, std::string const& message)
{
    if (!condition) throw std::runtime_error(message);
}

void near(double actual, double expected, double tolerance = 1e-7)
{
    require(std::isfinite(actual) && std::abs(actual - expected) <= tolerance,
            "expected " + std::to_string(expected) + ", got " + std::to_string(actual));
}

template<class P> void point(P const& value, double lon, double lat, double tolerance = 1e-7)
{
    double actual = bg::get_as_radian<0>(value) * 180 / pi;
    near(std::remainder(actual - lon, 360), 0, tolerance);
    near(bg::get_as_radian<1>(value) * 180 / pi, lat, tolerance);
}

template<class P> void piece(Line<P> const& line, double lon1, double lat1,
                             double lon2, double lat2, double tolerance = 1e-7)
{
    require(!line.empty(), "empty output component");
    point(line.front(), lon1, lat1, tolerance);
    point(line.back(), lon2, lat2, tolerance);
}

double peak() { return std::atan(std::tan(20 * pi / 180) / std::cos(60 * pi / 180)) * 180 / pi; }
double crossing(double latitude)
{
    return std::acos(std::tan(latitude * pi / 180) / std::tan(peak() * pi / 180)) * 180 / pi;
}

Line<S> arc() { return {{-60, 20}, {60, 20}}; }

template<class P, class Geometry> Multi<P> clip(Geometry const& input, Box<P> const& box, bool reverse = false)
{
    Multi<P> result;
    if (reverse) bg::intersection(box, input, result);
    else bg::intersection(input, box, result);
    for (auto const& component : result)
    {
        require(!component.empty(),"empty output component");
        for (auto const& p : component)
        {
            double lon=bg::get_as_radian<0>(p)*180/pi;
            double lat=bg::get_as_radian<1>(p)*180/pi;
            double west=bg::get_as_radian<bg::min_corner,0>(box)*180/pi;
            double width=(bg::get_as_radian<bg::max_corner,0>(box)*180/pi)-west;
            if (width<0) width+=360;
            double offset=std::fmod(lon-west+720,360);
            require(std::isfinite(lon) && std::isfinite(lat),"non-finite output vertex");
            require(lat>=bg::get_as_radian<bg::min_corner,1>(box)*180/pi-1e-7 &&
                    lat<=bg::get_as_radian<bg::max_corner,1>(box)*180/pi+1e-7,
                    "output vertex lies outside latitude bounds");
            require(width>=360-1e-7 || std::abs(lat)>=90-1e-7 ||
                    offset<=width+1e-7 || offset>=360-1e-7,
                    "output vertex lies outside longitude bounds");
            if constexpr (std::is_same<bg::cs_tag_t<P>,bg::spherical_equatorial_tag>::value)
                near(bg::distance(p,input),0,1e-10);
        }
    }
    return result;
}

void disconnected(bool reverse)
{
    auto result = clip<S>(arc(), Box<S>{{-90, 10}, {90, 30}}, reverse);
    require(result.size() == 2, "latitude turn must leave two disconnected portions");
    piece(result[0], -60, 20, -crossing(30), 30);
    piece(result[1], crossing(30), 30, 60, 20);
}

void cap(double latitude)
{
    auto result = clip<S>(arc(), Box<S>{{-90, latitude}, {90, 80}});
    require(result.size() == 1, "interior cap must survive with both input endpoints outside");
    piece(result[0], -crossing(latitude), latitude, crossing(latitude), latitude, 2e-6);
}

void own_envelope()
{
    Line<G> input{{-179.9958, -47.2835}, {179.9972, -47.2753}};
    Box<G> box;
    bg::envelope(input, box);
    for (bool reverse : {false, true})
    {
        auto result = clip<G>(input, box, reverse);
        require(result.size() == 1, "own envelope must retain one continuous segment");
        piece(result.front(), -179.9958, -47.2835, 179.9972, -47.2753);
    }
}

void dateline(bool wide)
{
    auto result = clip<S>(Line<S>{{160, 0}, {-160, 0}},
                          wide ? Box<S>{{-170, -5}, {170, 5}} : Box<S>{{170, -5}, {190, 5}});
    require(result.size() == (wide ? 2 : 1), "longitude interval topology is wrong");
    if (wide)
    {
        piece(result[0], 160, 0, 170, 0);
        piece(result[1], -170, 0, -160, 0);
    }
    else piece(result[0], 170, 0, -170, 0);
}

void units()
{
    Line<R> input{{-pi/3, pi/9}, {pi/3, pi/9}};
    auto result = clip<R>(input, Box<R>{{-pi/2, pi/6}, {pi/2, pi/2}});
    require(result.size() == 1, "radian clipping cap missing");
    piece(result[0], -crossing(30), 30, crossing(30), 30);
}

void segment()
{
    bg::model::segment<S> input{S{-60,20}, S{60,20}};
    for (bool reverse : {false, true})
    {
        auto result = clip<S>(input, Box<S>{{-90,10}, {90,30}}, reverse);
        require(result.size() == 2, "segment overload must split at latitude turn");
        piece(result[0], -60,20,-crossing(30),30);
        piece(result[1], crossing(30),30,60,20);
    }
}

void traversal()
{
    Line<S> input{{60,20}, {0,peak()}, {-60,20}};
    auto result = clip<S>(input, Box<S>{{-90,30}, {90,80}});
    require(result.size() == 1, "retained connecting vertex must join adjacent arcs");
    piece(result[0], crossing(30),30,-crossing(30),30);
    bool found = false;
    for (auto const& p : result[0]) found |= std::abs(bg::get<0>(p)) < 1e-10 && std::abs(bg::get<1>(p)-peak()) < 1e-10;
    require(found, "included original turning vertex was discarded");
}

void members()
{
    Multi<S> input{{{-60,20},{0,peak()}}, {{0,peak()},{60,20}}};
    for (bool reverse : {false,true})
    {
        auto result = clip<S>(input, Box<S>{{-90,30},{90,80}},reverse);
        require(result.size() == 2, "different multiline members must remain separate");
        piece(result[0],-crossing(30),30,0,peak());
        piece(result[1],0,peak(),crossing(30),30);
    }
}

void point_contacts()
{
    auto result = clip<S>(arc(), Box<S>{{-90,30},{90,30}});
    require(result.size() == 2, "zero-height parallel must retain both isolated contacts");
    for (auto const& line : result) require(line.size() == 1, "isolated contact must be a one-point linestring");
    point(result[0][0],-crossing(30),30);
    point(result[1][0],crossing(30),30);
}

void tangent()
{
    auto result = clip<S>(arc(), Box<S>{{-90,peak()},{90,90}});
    std::cerr << bg::wkt(result) << '\n';
    require(result.size() == 1 && result[0].size() == 1, "tangent must produce one point");
    point(result[0][0],0,peak(),2e-6);
}

void meridian()
{
    auto result = clip<S>(Line<S>{{10,-50},{10,50}}, Box<S>{{10,-20},{10,20}});
    require(result.size() == 1, "zero-width box must retain coincident meridian");
    piece(result[0],10,-20,10,20);
}

void repeated()
{
    auto result = clip<S>(Line<S>{{-60,20},{-60,20},{60,20},{60,20}}, Box<S>{{-90,10},{90,30}});
    require(result.size() == 2, "repeated endpoints must not create extra components");
    piece(result[0],-60,20,-crossing(30),30);
    piece(result[1],crossing(30),30,60,20);
}

void sphere_strategy()
{
    bg::strategies::relate::geographic<bg::strategy::vincenty> strategy{bg::srs::spheroid<double>{1,1}};
    Multi<G> result;
    bg::intersection(Line<G>{{-60,20},{60,20}}, Box<G>{{-90,30},{90,80}}, result, strategy);
    require(result.size() == 1, "explicit geographic strategy must retain cap");
    piece(result[0],-crossing(30),30,crossing(30),30);
}

void pole()
{
    auto result = clip<S>(Line<S>{{0,80},{180,80}}, Box<S>{{-10,85},{10,90}});
    require(result.size() == 1, "polar meridian must remain connected");
    point(result[0].front(),0,85);
    near(bg::get<1>(result[0].back()),90,2e-6);
    for (auto const& p : result[0])
        require(bg::get<1>(p) >= 85-1e-7, "polar clip retained an excluded vertex");
}

void reentry()
{
    Line<S> input{{-60,20},{60,20},{-60,20}};
    auto result = clip<S>(input,Box<S>{{-90,10},{90,30}});
    require(result.size() == 3, "excluded middle arcs must not join separate visits");
    piece(result[0],-60,20,-crossing(30),30);
    piece(result[1],crossing(30),30,crossing(30),30);
    piece(result[2],-crossing(30),30,-60,20);
    bool found = false;
    for (auto const& p : result[1]) found |= std::abs(bg::get<0>(p)-60) < 1e-10;
    require(found,"returning portion discarded its included turning vertex");
}

void equator_boundary()
{
    auto result = clip<S>(Line<S>{{-40,0},{40,0}},Box<S>{{-20,0},{20,10}});
    require(result.size()==1,"boundary-aligned arc must remain connected");
    piece(result[0],-20,0,20,0);
}

void custom_spheroid(double polar_radius)
{
    bg::srs::spheroid<double> spheroid{1,polar_radius};
    bg::strategies::relate::geographic<bg::strategy::vincenty> strategy{spheroid};
    bg::strategy::distance::geographic<bg::strategy::vincenty> distance{spheroid};
    G first{-60,20}, last{60,20};
    Multi<G> result;
    bg::intersection(Line<G>{first,last},Box<G>{{-30,0},{30,80}},result,strategy);
    require(result.size()==1,"custom spheroid arc was split or lost");
    near(bg::get<0>(result[0].front()),-30);
    near(bg::get<0>(result[0].back()),30);
    double whole=bg::distance(first,last,distance);
    for (auto const& p : result[0])
        near(bg::distance(first,p,distance)+bg::distance(p,last,distance),whole,1e-9);
    bg::strategies::line_interpolate::geographic<bg::strategy::vincenty> interpolation{spheroid};
    for (auto const& endpoint : {result[0].front(),result[0].back()})
    {
        double low=0, high=whole;
        G interpolated;
        for (unsigned iteration=0; iteration<64; ++iteration)
        {
            double middle=(low+high)/2;
            bg::line_interpolate(Line<G>{first,last},middle,interpolated,interpolation);
            if (bg::get<0>(interpolated)<bg::get<0>(endpoint)) low=middle;
            else high=middle;
        }
        near(bg::get<1>(endpoint),bg::get<1>(interpolated),1e-7);
    }
    auto repeated = Multi<G>{};
    bg::intersection(result,Box<G>{{-30,0},{30,80}},repeated,strategy);
    require(repeated.size()==1,"re-clipping retained geometry changed connectivity");
    piece(repeated[0],bg::get<0>(result[0].front()),bg::get<1>(result[0].front()),
          bg::get<0>(result[0].back()),bg::get<1>(result[0].back()));
}

void geographic_radians()
{
    using P = bg::model::point<double,2,bg::cs::geographic<bg::radian>>;
    bg::strategies::relate::geographic<bg::strategy::thomas> strategy{bg::srs::spheroid<double>{3,3}};
    Multi<P> result;
    bg::intersection(Box<P>{{-pi/2,pi/6},{pi/2,pi/2}},
                     Line<P>{{-pi/3,pi/9},{pi/3,pi/9}},result,strategy);
    require(result.size()==1,"radian geographic strategy lost cap");
    piece(result[0],-crossing(30),30,crossing(30),30);
}

void coincident()
{
    for (S const p : {S{17,23},S{-47,81},S{180,-27},S{6.31,52.71}})
    {
        auto result = clip<S>(Line<S>{p,p},Box<S>{{-180,-90},{180,90}});
        require(result.size()==1 && result[0].size()==1,"coincident segment must retain its one finite point");
        point(result[0][0],bg::get<0>(p),bg::get<1>(p));
    }
}

void long_arc()
{
    auto result = clip<S>(Line<S>{{0,0},{179.9,0.1}},Box<S>{{80,0},{100,90}});
    double latitude = std::atan(std::tan(0.1*pi/180) * std::sin(80*pi/180) / std::sin(179.9*pi/180))*180/pi;
    require(result.size()==1,"long non-antipodal arc must retain the middle portion");
    piece(result[0],80,latitude,100,latitude);
}

void asymmetric_cap()
{
    double last_latitude=std::atan(std::tan(peak()*pi/180)*std::cos(20*pi/180))*180/pi;
    double bound=peak()-1e-8;
    auto result=clip<S>(Line<S>{{-60,20},{20,last_latitude}},Box<S>{{-90,bound},{90,80}});
    require(result.size()==1,"off-center narrow latitude maximum was missed");
    piece(result[0],-crossing(bound),bound,crossing(bound),bound,2e-6);
}

void pole_contact()
{
    auto result=clip<S>(Line<S>{{0,80},{180,80}},Box<S>{{40,90},{50,90}});
    std::cerr << bg::wkt(result) << '\n';
    require(result.size()==1 && result[0].size()==1,"all meridians share the pole boundary point");
    near(bg::get<1>(result[0][0]),90,2e-6);
}

int main(int argc, char** argv)
{
    std::map<std::string,std::function<void()>> cases{
        {"own_envelope",own_envelope}, {"disconnected",[]{disconnected(false);}},
        {"box_first",[]{disconnected(true);}}, {"cap",[]{cap(30);}},
        {"narrow_cap",[]{cap(peak()-1e-8);}}, {"dateline",[]{dateline(false);}},
        {"wide_box",[]{dateline(true);}}, {"radians",units}, {"segment",segment},
        {"traversal",traversal}, {"multiline",members}, {"point_contacts",point_contacts},
        {"tangent",tangent}, {"meridian",meridian}, {"repeated",repeated},
        {"sphere_strategy",sphere_strategy},
        {"pole",pole}, {"reentry",reentry}, {"equator_boundary",equator_boundary},
        {"oblate",[]{custom_spheroid(0.98);}}, {"prolate",[]{custom_spheroid(1.01);}},
        {"geographic_radians",geographic_radians},
        {"coincident",coincident}, {"long_arc",long_arc},
        {"asymmetric_cap",asymmetric_cap}, {"pole_contact",pole_contact},
        {"empty",[]{require(clip<S>(Line<S>{},Box<S>{{-180,-90},{180,90}}).empty(),"empty input produced output");}},
        {"full_longitude",[]{auto r=clip<S>(Line<S>{{175,0},{-175,0}},Box<S>{{-180,-10},{180,10}}); require(r.size()==1,"full longitude box lost arc");piece(r[0],175,0,-175,0);}},
        {"outside",[]{require(clip<S>(arc(),Box<S>{{-90,peak()+0.01},{90,90}}).empty(),"disjoint cap produced output");}}
    };
    if (argc != 2 || !cases.count(argv[1])) return 2;
    try { cases.at(argv[1])(); }
    catch (std::exception const& error) { std::cerr << argv[1] << ": " << error.what() << '\n'; return 1; }
}
