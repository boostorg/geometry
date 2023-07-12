// NOTE: this is used in the Haussdorf algorithm.
// However, it does not speed it up, it slows it down (for this test)
// Without: 47 ms
// With: 312 ms
// #define BOOST_GEOMETRY_ENABLE_SIMILARITY_RTREE

#include <boost/geometry.hpp>

#include <boost/geometry/io/geojson/geojson_writer.hpp>

#include <boost/geometry/algorithms/similarity.hpp>

#include <fstream>

const double g_scale = 1.0e6;

namespace
{

template <typename Geometry>
double fixed_hausdorff(const Geometry& geometry1, const Geometry& geometry2)
{
    namespace bg = boost::geometry;
    return std::max(bg::discrete_hausdorff_distance(geometry1, geometry2),
        bg::discrete_hausdorff_distance(geometry2, geometry1));
}

}


struct geojson_visitor
{
    geojson_visitor()
    {
        m_stream_coll << std::setprecision(20);
        m_stream_quad << std::setprecision(20);
    }
    template <typename T, typename I> void visit_quadrilateral(T const& ring,
        I const& a, I const& b, I const& c, I const& d) 
    {
        m_quad.feature(ring);
        m_quad.add_property("role", 7);
        m_quad.add_property("area", boost::geometry::area(ring));
    }
    template <typename C> void visit_projections(int source, C const& collection) 
    {
        for (const auto& info : collection)
        {
            m_collection.feature(info.point);
            m_collection.add_property("source", source);
            m_collection.add_property("is_projection", info.is_projection ? 1 : 0);
            m_collection.add_property("distance", info.distance);
            m_collection.add_property("source_index", info.source_index);
            m_collection.add_property("segment_index", info.segment_index);
            m_collection.add_property("offset", info.offset);
        }
    }

    std::ofstream m_stream_coll{"/tmp/collection.geojson"};
    std::ofstream m_stream_quad{"/tmp/quad.geojson"};
    boost::geometry::geojson_writer m_collection{m_stream_coll};
    boost::geometry::geojson_writer m_quad{m_stream_quad};
};


template <typename Geometry>
void test_case(Geometry const& p, Geometry const& q, double scale = g_scale, const char* filename = nullptr)
{
    std::cout << "Frechet: " << scale * boost::geometry::discrete_frechet_distance(p, q)
        << " Hausdorff: " << scale * fixed_hausdorff(p, q)
        << " Average: " << scale * boost::geometry::similarity_distance(p, q)
        << std::endl;

    if (filename != nullptr)
    {
        std::ofstream fout{filename};
        fout << std::setprecision(20);
        boost::geometry::geojson_writer writer(fout);
        writer.feature(p);
        writer.add_property("name", "p");
        
        writer.feature(q);
        writer.add_property("name", "q");

        for (const auto& pnt : p)
        {
            writer.feature(pnt);
            writer.add_property("name", "p");
        }
        for (const auto& pnt : q)
        {
            writer.feature(pnt);
            writer.add_property("name", "q");
        }
    }
}

template <typename Geometry>
void performance_test(Geometry const& p, Geometry const& q)
{
    std::size_t n = 100000;
    auto start = std::chrono::steady_clock::now();
    for (std::size_t i = 0; i < n; i++)
    {
        double f = boost::geometry::discrete_frechet_distance(p, q);
    }
    auto finish = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count();
    std::cout << "Frechet: " << std::setprecision(6) << elapsed << " ms" << std::endl;

    start = std::chrono::steady_clock::now();
    for (std::size_t i = 0; i < n; i++)
    {
        double h = fixed_hausdorff(p, q);
    }
    finish = std::chrono::steady_clock::now();
    elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count();
    std::cout << "Hausdorff: " << std::setprecision(6) << elapsed << " ms" << std::endl;

    start = std::chrono::steady_clock::now();
    for (std::size_t i = 0; i < n; i++)
    {
        double h = boost::geometry::similarity_distance(p, q);
    }
    finish = std::chrono::steady_clock::now();
    elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count();
    std::cout << "Average: " << std::setprecision(6) << elapsed << " ms" << std::endl;
}

int main()
{
    namespace bg = boost::geometry;
    auto reverse = [](auto const& g) 
    {
        auto result = g;
        std::reverse(result.begin(), result.end());
        return result;
    };

    auto simplify = [](auto const& g, double simplify_distance) 
    {
        std::decay_t<decltype(g)> result;
        bg::simplify(g, result, simplify_distance);
        return result;
    };

    using coordinate_type = double;
    using point = boost::geometry::model::d2::point_xy<coordinate_type>;
    using linestring = boost::geometry::model::linestring<point>;
    using ring = boost::geometry::model::ring<point>;

    // Two lines consistently 0.5 apart
    std::string const simplex1 = "LINESTRING(1.0 1.0, 2.0 1.0)";
    std::string const simplex2 = "LINESTRING(1.0 1.1, 2.0 1.1)";
    // With extra point in between
    std::string const simplex3 = "LINESTRING(1.0 1.1, 1.5 1.1,2.0 1.1)";
    // With a "spike"
    std::string const simplex4 = "LINESTRING(1.0 1.1, 1.49 1.1, 1.5 2.5, 1.51 1.1, 2.0 1.1)";

    std::string const west1 = "LINESTRING(5.134673416614532 52.27074444293976,5.134786069393158 52.27136671543121,5.134778022766113 52.27139353752136,5.134756565093994 52.27141231298447,5.134694874286652 52.27144449949265,5.134069919586182 52.27149546146393,5.133965313434601 52.27146327495575,5.133906304836273 52.27141231298447,5.133863389492035 52.27112531661987,5.133847296237946 52.27108776569366,5.133809745311737 52.27106094360352,5.133764147758484 52.27100998163223,5.133707821369171 52.27053791284561,5.133721232414246 52.27049767971039,5.133863389492035 52.27035015821457)";
    std::string const west2 = "LINESTRING(5.134670734405518 52.27073907852173,5.134778022766113 52.27136135101318,5.134778022766113 52.27139353752136,5.134756565093994 52.27140426635742,5.134692192077637 52.2714364528656,5.134069919586182 52.2714900970459,5.134016275405884 52.27147936820984,5.133962631225586 52.27145791053772,5.133898258209229 52.27140426635742,5.133844614028931 52.27108240127563,5.133801698684692 52.27106094360352,5.133758783340454 52.27100729942322,5.133705139160156 52.2705352306366,5.133715867996216 52.27049231529236,5.13385534286499 52.27034211158752)";
    std::string const east1 = "LINESTRING(5.141247510910034 52.27364659309387,5.140657424926758 52.27396845817566,5.14061450958252 52.2740113735199,5.140560865402222 52.27447271347046,5.140872001647949 52.27483749389648,5.140936374664307 52.27486968040466,5.141386985778809 52.27483749389648)";
    std::string const east2 = "LINESTRING(5.141247510910034 52.27364659309387,5.140665471553802 52.27397114038467,5.14061450958252 52.27401673793793,5.1405930519104 52.27407306432724,5.140560865402222 52.2744807600975,5.140606462955475 52.27453976869583,5.140874683856964 52.27483749389648,5.140941739082336 52.27486968040466,5.141386985778809 52.27483749389648)";
    std::string const complete = "LINESTRING(5.135158896446228 52.26660043001175,5.134890675544739 52.2666272521019,5.134839713573456 52.26665943861008,5.13435423374176 52.26669162511826,5.134295225143433 52.26670503616333,5.13424426317215 52.26672381162643,5.134182572364807 52.26675063371658,5.134131610393524 52.26690083742142,5.134077966213226 52.26698398590088,5.134086012840271 52.26706176996231,5.134115517139435 52.26712614297867,5.134161114692688 52.26717710494995,5.134324729442596 52.26729512214661,5.134397149085999 52.26735413074493,5.134442746639252 52.26739972829819,5.134464204311371 52.26744264364243,5.134584903717041 52.2677618265152)";
    std::string const part1 = "LINESTRING(5.134332776069641 52.26669698953629,5.134295225143433 52.26670503616333,5.13424426317215 52.26672381162643,5.134182572364807 52.26675063371658,5.134131610393524 52.26690083742142,5.134077966213226 52.26698398590088,5.134086012840271 52.26706176996231,5.134115517139435 52.26712614297867,5.134161114692688 52.26717710494995,5.134324729442596 52.26729512214661,5.134397149085999 52.26735413074493,5.134442746639252 52.26739972829819,5.134464204311371 52.26744264364243,5.134584903717041 52.2677618265152)";
    std::string const part2 = "LINESTRING(5.135158896446228 52.26660043001175,5.134890675544739 52.2666272521019,5.134839713573456 52.26665943861008,5.13435423374176 52.26669162511826,5.134332776069641 52.26669698953629)";

    // test_case(bg::from_wkt<linestring>(west1), bg::from_wkt<linestring>(west2));
    // return 0;

    std::cout << "Simplex cases: " << std::endl;
    test_case(bg::from_wkt<linestring>(simplex1), bg::from_wkt<linestring>(simplex2), 1.0, "/tmp/simplex1.geojson");
    test_case(bg::from_wkt<linestring>(simplex1), bg::from_wkt<linestring>(simplex3), 1.0, "/tmp/simplex2.geojson");
    test_case(bg::from_wkt<linestring>(simplex1), bg::from_wkt<linestring>(simplex4), 1.0, "/tmp/simplex3.geojson");

    std::cout << std::endl<< "Real cases: " << std::endl;
    test_case(bg::from_wkt<linestring>(west1), bg::from_wkt<linestring>(west2), g_scale, "/tmp/west.geojson");
    test_case(bg::from_wkt<linestring>(east1), bg::from_wkt<linestring>(east2), g_scale, "/tmp/east.geojson");

    std::cout << std::endl<< "Reversed:" << std::endl;
    test_case(bg::from_wkt<linestring>(west1), reverse(bg::from_wkt<linestring>(west2)));
    test_case(bg::from_wkt<linestring>(east1), reverse(bg::from_wkt<linestring>(east2)));

    std::cout << std::endl<< "Simplified (west): " << std::endl;
    test_case(bg::from_wkt<linestring>(west2), simplify(bg::from_wkt<linestring>(west2), 0.0005), g_scale, "/tmp/simplified1.geojson");
    test_case(bg::from_wkt<linestring>(west2), simplify(bg::from_wkt<linestring>(west2), 0.0001), g_scale, "/tmp/simplified2.geojson");
    test_case(bg::from_wkt<linestring>(west2), simplify(bg::from_wkt<linestring>(west2), 0.00001), g_scale, "/tmp/simplified3.geojson");
    test_case(bg::from_wkt<linestring>(west2), simplify(bg::from_wkt<linestring>(west2), 0.000001), g_scale, "/tmp/simplified4.geojson");

    std::cout << std::endl<< "Simplified (east): " << std::endl;
    test_case(bg::from_wkt<linestring>(east2), simplify(bg::from_wkt<linestring>(east2), 0.0005), g_scale, "/tmp/simplified_east1.geojson");
    test_case(bg::from_wkt<linestring>(east2), simplify(bg::from_wkt<linestring>(east2), 0.0001), g_scale, "/tmp/simplified_east2.geojson");
    test_case(bg::from_wkt<linestring>(east2), simplify(bg::from_wkt<linestring>(east2), 0.00001), g_scale, "/tmp/simplified_east3.geojson");
    test_case(bg::from_wkt<linestring>(east2), simplify(bg::from_wkt<linestring>(east2), 0.000001), g_scale, "/tmp/simplified_east4.geojson");

    std::cout << std::endl<< "Not similar: " << std::endl;
    test_case(bg::from_wkt<linestring>(west1), bg::from_wkt<linestring>(east1));
    test_case(bg::from_wkt<linestring>(west2), bg::from_wkt<linestring>(east2));

    std::cout << std::endl << "Parts (TODO):" << std::endl;
    test_case(bg::from_wkt<linestring>(complete), bg::from_wkt<linestring>(part1));
    test_case(bg::from_wkt<linestring>(complete), bg::from_wkt<linestring>(part2));

    std::cout << std::endl<< "Identical: " << std::endl;
    test_case(bg::from_wkt<linestring>(west1), bg::from_wkt<linestring>(west1));
    test_case(bg::from_wkt<linestring>(east1), bg::from_wkt<linestring>(east1));

    // std::cout << std::endl << "Performance:" << std::endl;
    // performance_test(bg::from_wkt<linestring>(west1), bg::from_wkt<linestring>(west2));

    // With a visitor
    geojson_visitor v;
    auto result = bg::similarity(bg::from_wkt<linestring>(west1), bg::from_wkt<linestring>(west2), v);


    return 0;
}