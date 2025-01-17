// Boost.Geometry (aka GGL, Generic Geometry Library)
// Robustness Test

// Copyright (c) 2025 Tinko Bartels, Shenzhen, China.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#define BOOST_GEOMETRY_NO_BOOST_TEST

#include <bitset>
#include <chrono>
#include <iostream>
#include <random>
#include <utility>
#include <vector>

#include <boost/program_options.hpp>

#include <boost/geometry/algorithms/difference.hpp>
#include <boost/geometry/algorithms/union.hpp>
#include <boost/geometry/algorithms/sym_difference.hpp>
#include <boost/geometry/algorithms/intersection.hpp>
#include <boost/geometry/algorithms/is_valid.hpp>
#include <boost/geometry/io/wkt/write.hpp>
#include <boost/geometry/geometries/geometries.hpp>

#define BOOST_GEOMETRY_TEST_ONLY_ONE_TYPE
#ifndef BOOST_GEOMETRY_DEFAULT_TEST_TYPE
#define BOOST_GEOMETRY_DEFAULT_TEST_TYPE int
#endif
#include <geometry_test_common.hpp>

constexpr int chunk_size = 64;
using bits = std::vector<std::bitset<chunk_size>>;
using generator_t = std::mt19937_64;
static_assert(sizeof(generator_t::result_type) >= chunk_size / 8, "Generator output too narrow.");

namespace bg = boost::geometry;

using point = bg::model::d2::point_xy<BOOST_GEOMETRY_DEFAULT_TEST_TYPE>;
using box = bg::model::box<point>;
using mp_t = bg::model::multi_polygon<bg::model::polygon<point>>;

struct grid_settings
{
    int width = 5;
    int height = 5;
    int count = 1;
    generator_t::result_type seed = generator_t::default_seed;
    bool verbose = false;
    std::vector<std::uint64_t> bits1{};
    std::vector<std::uint64_t> bits2{};
};

constexpr int cell_dimension = 2;

std::vector<box> grid_cells(grid_settings const& settings)
{
    std::vector<box> out;
    out.reserve(settings.height * settings.width);
    for (int y = 0; y < settings.height; ++y)
    {
        for (int x = 0; x < settings.width; ++x)
        {
            out.push_back(box{point(x * cell_dimension, y * cell_dimension),
                              point((x + 1) * cell_dimension, (y + 1) * cell_dimension)});
        }
    }
    return out;
}

std::vector<point> test_points(grid_settings const& settings)
{
    std::vector<point> out;
    out.reserve(settings.height * settings.width);
    for (int y = 0; y < settings.height; ++y)
    {
        for (int x = 0; x < settings.width; ++x)
        {
            out.push_back(point(x * cell_dimension + cell_dimension / 2,
                                y * cell_dimension + cell_dimension / 2));
        }
    }
    return out;
}

std::ostream& operator<<(std::ostream& os, std::pair<bits, grid_settings> const& b_gs)
{
    if (b_gs.second.verbose)
    {
        os << '\n';
        for (int y = b_gs.second.height - 1; y >= 0; --y)
        {
            for (int x = 0; x < b_gs.second.width; ++x)
            {
                int index = y * b_gs.second.width + x;
                os << b_gs.first[index / chunk_size][index % chunk_size];
            }
            os << '\n';
        }
    }
    else
    {
        os << '{' << b_gs.first[0].to_ullong();
        for (size_t i = 1; i < b_gs.first.size(); ++i) os << ' ' << b_gs.first[i].to_ullong();
        os << '}';
    }
    return os;
}

bits geometry_to_bits(mp_t const& geometry, std::vector<point> const& test_points)
{
    bits out((test_points.size() + chunk_size - 1) / chunk_size);
    for (size_t i = 0; i < test_points.size(); ++i)
    {
        out[i / chunk_size][i % chunk_size] = bg::within(test_points[i], geometry);
    }
    return out;
}

mp_t bits_to_geometry(bits const& b, std::vector<box> const& grid, std::vector<point> const& points,
                      grid_settings const& settings, std::map<std::string, int>& failures)
{
    mp_t out;
    for (size_t i = 0; i < grid.size(); ++i)
    {
        if (b[i / chunk_size][i % chunk_size])
        {
            mp_t temp;
            bg::union_(out, grid[i], temp);
            out = std::move(temp);
        }
    }
    // Convenience lambda to pair bits with settings to use width/height in operator<<(os, ...)
    const auto b_gs = [&settings](bits const& b) { return std::make_pair(b, settings); };
    std::string reason{};
    if (! bg::is_valid(out, reason))
    {
        if (settings.verbose)
        {
            std::cout << bg::wkt(out) << "\ngenerated from" << b_gs(b)
                      << "is invalid: " << reason << ".\n\n";
        }
        else std::cout << b_gs(b) << " invalid (" << reason << ")\n";
        ++failures["bits_to_geometry validity"];
    }
    if (geometry_to_bits(out, points) != b)
    {
        if (settings.verbose)
        {
            std::cout << "Generating grid from pattern" << b_gs(b)
                      << "results in mismatching geometry: " << bg::wkt(out) << ".\n\n";
        }
        else std::cout << b_gs(b) << " mismatch.\n";
        ++failures["bits_to_geometry mismatch"];
    }
    return out;
}

bits gen_bits(generator_t& generator, int bits_size)
{
    bits b((bits_size + chunk_size - 1) / chunk_size);
    std::generate(b.begin(), b.end(), std::ref(generator));
    if (bits_size % chunk_size != 0)
    {
        std::bitset<chunk_size> bm;
        bm.set();
        bm >>= chunk_size - bits_size % chunk_size;
        b.back() &= bm;
    }
    return b;
}

bits to_bits(std::vector<std::uint64_t> const& in)
{
    bits out;
    out.reserve(in.size());
    for (auto const& ullong : in) out.push_back(std::bitset<chunk_size>(ullong));
    return out;
}

template <typename BitOp>
bits apply_for_each(bits a, bits const& b, BitOp const& bit_op)
{
    for (size_t i = 0; i < a.size(); ++i) a[i] = bit_op(a[i], b[i]);
    return a;
}

template<typename BitOp, typename GeoOp>
void test_op(bits const& bits1, bits const& bits2, mp_t const& geo1, mp_t const& geo2,
             std::string const& op_label, BitOp const& bit_op, GeoOp const& geo_op,
             std::vector<point> const& test_points, std::vector<box> const& grid,
             grid_settings const& settings, std::map<std::string, int>& failures)
{
    auto test_geo = geo_op(geo1, geo2);
    // Convenience lambda to pair bits with settings to use width/height in operator<<(os, ...)
    const auto b_gs = [&settings](bits const& b) { return std::make_pair(b, settings); };
    std::string reason{};
    if (! bg::is_valid(test_geo, reason))
    {
        if (settings.verbose)
        {
            std::cout << op_label << "(\n\t" << bg::wkt(geo1) << ",\n\t " << bg::wkt(geo2) << "\n),"
                      << "\ngenerated from" << b_gs(bits1) << "and" << b_gs(bits2) << "is invalid: "
                      << reason << ".\n\n";
        }
        else
        {
           std::cout << op_label << '(' << b_gs(bits1) << ", " << b_gs(bits2) << " invalid ("
                     << reason << ").\n";
        }
        ++failures[op_label + " validity"];
    }
    const bits expected = apply_for_each(bits1, bits2, bit_op);
    const bits obtained = geometry_to_bits(test_geo, test_points);
    if (obtained != expected)
    {
        if (settings.verbose)
        {
            std::cout << op_label << "(\n\t" << bg::wkt(geo1) << ",\n\t" << bg::wkt(geo2) << "\n),"
                      << "\ngenerated from" << b_gs(bits1) << "and" << b_gs(bits2)
                      << "is incorrect.\nExpected: "
                      << bg::wkt(bits_to_geometry(expected, grid, test_points, settings, failures))
                      << "\ncorresponding to" << b_gs(expected) << "Obtained: "
                      << bg::wkt(test_geo) << "\ncorresponding to" << b_gs(obtained) << "\n";
        }
        else std::cout << op_label << '(' << b_gs(bits1) << ", " << b_gs(bits2) << ") mismatch.\n";
        ++failures[op_label + " mismatch"];
    }
}

void test_bits(bits const& bits1, bits const& bits2,
               std::vector<box> const& grid, std::vector<point> const& test_points,
               grid_settings const& settings, std::map<std::string, int>& failures)
{
    const auto geo1 = bits_to_geometry(bits1, grid, test_points, settings, failures);
    const auto geo2 = bits_to_geometry(bits2, grid, test_points, settings, failures);
    test_op(bits1, bits2, geo1, geo2, "union", std::bit_or<>{},
            [](mp_t const& g1, mp_t const& g2) { mp_t g; bg::union_(g1, g2, g); return g; },
            test_points, grid, settings, failures);
    test_op(bits1, bits2, geo1, geo2, "intersection", std::bit_and<>{},
            [](mp_t const& g1, mp_t const& g2) { mp_t g; bg::intersection(g1, g2, g); return g; },
            test_points, grid, settings, failures);
    test_op(bits1, bits2, geo1, geo2, "sym_difference", std::bit_xor<>{},
            [](mp_t const& g1, mp_t const& g2) { mp_t g; bg::sym_difference(g1, g2, g); return g; },
            test_points, grid, settings, failures);
    test_op(bits1, bits2, geo1, geo2, "difference g1 \\ g2",
            [](std::bitset<chunk_size> b1, std::bitset<chunk_size> b2) { return b1 & (~b2); },
            [](mp_t const& g1, mp_t const& g2) { mp_t g; bg::difference(g1, g2, g); return g; },
            test_points, grid, settings, failures);
    test_op(bits1, bits2, geo1, geo2, "difference g2 \\ g1",
            [](std::bitset<chunk_size> b1, std::bitset<chunk_size> b2) { return b2 & (~b1); },
            [](mp_t const& g1, mp_t const& g2) { mp_t g; bg::difference(g2, g1, g); return g; },
            test_points, grid, settings, failures);
}

bool test_all(grid_settings const& settings)
{
    generator_t genenerator(settings.seed);
    const auto grid = grid_cells(settings);
    const auto points = test_points(settings);
    std::map<std::string, int> failures;
    auto const t0 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < settings.count || settings.count == -1; i++)
    {
        const bits bits1 = settings.bits1.size() == 0 ?
                                  gen_bits(genenerator, settings.width * settings.height)
                                : to_bits(settings.bits1);
        const bits bits2 = settings.bits2.size() == 0 ?
                                  gen_bits(genenerator, settings.width * settings.height)
                                : to_bits(settings.bits2);
        test_bits(bits1, bits2, grid, points, settings, failures);
    }
    auto const t = std::chrono::high_resolution_clock::now();
    auto const elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t - t0).count();
    int failure_count = std::accumulate(failures.begin(), failures.end(), 0,
                                        [](int acc, auto const& kv) { return acc + kv.second; });
    std::cout << "\niterations: " << settings.count
              << " errors: " << failure_count
              << " time: " << elapsed_ms / 1000.0 << '\n';
    if (failure_count != 0)
    {
        std::cout << "Failure counts by failure mode:\n";
        for (auto const& fm : failures) std::cout << '\t' << fm.first << ": " << fm.second << '\n';
    }
    return failure_count == 0;
}

bool validate_bits_input(std::vector<std::uint64_t> const& bits_in, size_t bits_size)
{
    if (bits_in.size() == 0) return true;
    if (bits_in.size() != (bits_size + chunk_size - 1) / chunk_size) return false;
    if (bits_size % chunk_size != 0)
    {
        std::bitset<chunk_size> bm;
        bm.set();
        bm >>= chunk_size - bits_size % chunk_size;
        if (bits_in.back() & ~bm.to_ullong()) return false;
    }
    return true;
}

int main(int argc, char** argv)
{
    BoostGeometryWriteTestConfiguration();
    try
    {
        namespace po = boost::program_options;
        po::options_description description("=== random_integer_grids ===\nAllowed options");

        grid_settings settings;

        description.add_options()
            ("help", "Help message")
            ("seed",
             po::value<decltype(settings.seed)>(&settings.seed)->default_value(settings.seed),
             "Initialization seed for random generator")
            ("count",
             po::value<decltype(settings.count)>(&settings.count)->default_value(settings.count),
             "Number of tests (-1 for infinite loop)")
            ("width",
             po::value<decltype(settings.width)>(&settings.width)->default_value(settings.width),
             "Width of grid (>= 1)")
            ("height",
             po::value<decltype(settings.height)>(&settings.height)->default_value(settings.height),
             "Height of grid (>= 1)")
            ("verbose",
             po::bool_switch(&settings.verbose),
             "Print WKT and bit patterns for each failure.")
            ("bits1",
             po::value<decltype(settings.bits1)>(&settings.bits1)->multitoken(),
             "Fixed bit pattern for first operand as list of ullong.")
            ("bits2",
             po::value<decltype(settings.bits2)>(&settings.bits2)->multitoken(),
             "Fixed bit pattern for second operand as list of ullong.")
        ;

        po::variables_map varmap;
        po::store(po::parse_command_line(argc, argv, description), varmap);
        po::notify(varmap);

        if (! validate_bits_input(settings.bits1, settings.height * settings.width))
        {
            std::cout << "bits1 was provided but does not match dimensions.\n";
            return 1;
        }
        if (! validate_bits_input(settings.bits2, settings.height * settings.width))
        {
            std::cout << "bits2 was provided but does not match dimensions.\n";
            return 1;
        }
        if (settings.bits1.size() != 0 && settings.bits2.size() != 0 && settings.count != 1)
        {
            std::cout << "Both bit patterns fixed, count is changed to 1.\n";
            settings.count = 1;
        }
        if (settings.height < 1 || settings.width < 1)
        {
            std::cout << "Invalid dimensions, height and width need to be positive.\n";
            return 1;
        }
        if (varmap.count("help"))
        {
            std::cout << description << std::endl;
            return 1;
        }
        if (! test_all(settings)) return 1;
    }
    catch (std::exception const& e)
    {
        std::cout << "Exception " << e.what() << '\n';
    }
    catch (...)
    {
        std::cout << "Other exception" << '\n';
    }

    return 0;
}
