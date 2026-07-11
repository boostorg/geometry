// Boost.Geometry (aka GGL, Generic Geometry Library)
// Robustness Test - SVG rendering for confetti_buffer

// Copyright (c) 2026 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Draws the coloured confetti: each streamer's buffer as a translucent coloured
// ribbon, with the merged buffer outline on top, on a dark or light background.

#ifndef BOOST_GEOMETRY_TEST_CONFETTI_BUFFER_SVG_HPP
#define BOOST_GEOMETRY_TEST_CONFETTI_BUFFER_SVG_HPP

#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/io/svg/svg_mapper.hpp>

// Convert an HSV colour (h in [0,360), s and v in [0,1]) to an "rgb(r,g,b)" string.
inline std::string hsv_color(double h, double s, double v)
{
    double const c = v * s;
    double const x = c * (1.0 - std::fabs(std::fmod(h / 60.0, 2.0) - 1.0));
    double const m = v - c;
    double r = 0, g = 0, b = 0;
    if (h < 60)       { r = c; g = x; }
    else if (h < 120) { r = x; g = c; }
    else if (h < 180) { g = c; b = x; }
    else if (h < 240) { g = x; b = c; }
    else if (h < 300) { r = x; b = c; }
    else              { r = c; b = x; }
    auto to255 = [](double d) { return static_cast<int>(d * 255.0 + 0.5); };
    std::ostringstream out;
    out << "rgb(" << to255(r + m) << "," << to255(g + m) << "," << to255(b + m) << ")";
    return out.str();
}

// Map a position t in [0,1) to a hue, biased towards a warm, confetti-like
// palette: red/orange/yellow dominate, with some magenta and light green, and
// rare cyan. Blue is skipped entirely. The mapping is a smooth (piecewise-linear)
// gradient in an "arc" coordinate: 300..360 is magenta..red, and +360 continues
// into 0..190 (red..orange..yellow..green..cyan).
inline double confetti_hue(double t)
{
    static const double ts[] = {0.00, 0.15, 0.80, 0.94, 1.00}; // fractions of the range
    static const double as[] = {300.0, 360.0, 425.0, 480.0, 515.0}; // arc coordinates
    double a = as[4];
    for (int i = 1; i < 5; i++)
    {
        if (t <= ts[i])
        {
            double const f = (t - ts[i - 1]) / (ts[i] - ts[i - 1]);
            a = as[i - 1] + f * (as[i] - as[i - 1]);
            break;
        }
    }
    return std::fmod(a, 360.0);
}

// Draw the coloured ribbons (one buffered streamer each) plus the merged buffer
// outline. The ribbons are already buffered by the caller (reusing the strategy).
template <typename MultiPolygon>
void create_svg(std::string const& filename,
        std::vector<MultiPolygon> const& ribbons,
        MultiPolygon const& merged,
        bool dark, double distance)
{
    namespace bg = boost::geometry;
    using point_type = typename bg::point_type<MultiPolygon>::type;
    using box_type = bg::model::box<point_type>;

    box_type box;
    bool have_box = false;
    auto expand_with = [&](auto const& geometry)
    {
        if (bg::is_empty(geometry))
        {
            return;
        }
        auto const envelope = bg::return_envelope<box_type>(geometry);
        if (! have_box) { box = envelope; have_box = true; }
        else { bg::expand(box, envelope); }
    };
    expand_with(merged);
    for (auto const& ribbon : ribbons)
    {
        expand_with(ribbon);
    }
    if (! have_box)
    {
        return;
    }
    bg::buffer(box, box, distance + 2.0);

    // Make the box square, so it fills the square canvas without white margins.
    {
        double const minx = bg::get<bg::min_corner, 0>(box);
        double const miny = bg::get<bg::min_corner, 1>(box);
        double const maxx = bg::get<bg::max_corner, 0>(box);
        double const maxy = bg::get<bg::max_corner, 1>(box);
        double const cx = (minx + maxx) / 2.0;
        double const cy = (miny + maxy) / 2.0;
        double const half = std::max(maxx - minx, maxy - miny) / 2.0;
        bg::set<bg::min_corner, 0>(box, cx - half);
        bg::set<bg::min_corner, 1>(box, cy - half);
        bg::set<bg::max_corner, 0>(box, cx + half);
        bg::set<bg::max_corner, 1>(box, cy + half);
    }

    std::ofstream svg(filename.c_str());
    bg::svg_mapper<point_type> mapper(svg, 1000, 1000);
    mapper.add(box);

    // Background.
    mapper.map(box, dark
        ? "fill:rgb(16,18,28);stroke:none"
        : "fill:rgb(250,250,252);stroke:none");

    // One translucent coloured ribbon per streamer.
    int const n = static_cast<int>(ribbons.size());
    for (int index = 0; index < n; index++)
    {
        double const t = n > 0 ? static_cast<double>(index) / n : 0.0;
        double const hue = confetti_hue(t);
        std::string const color = hsv_color(hue, 0.85, dark ? 1.0 : 0.85);

        std::ostringstream style;
        style << "fill:" << color << ";fill-opacity:" << (dark ? 0.35 : 0.32)
              << ";stroke:" << color << ";stroke-width:1;stroke-opacity:0.9";
        mapper.map(ribbons[index], style.str());
    }

    // The merged buffer (all streamers at once) as a bright outline on top.
    mapper.map(merged, dark
        ? "fill:none;stroke:rgb(255,255,255);stroke-width:1.2;stroke-opacity:0.85"
        : "fill:none;stroke:rgb(20,20,20);stroke-width:1.2;stroke-opacity:0.8");
}

#endif // BOOST_GEOMETRY_TEST_CONFETTI_BUFFER_SVG_HPP
