#include <boost/geometry/extensions/index/rtree/rtree.hpp>

#include <iostream>
#include <fstream>

#include <boost/timer.hpp>
#include <boost/foreach.hpp>

//TEST
#include <gl/glut.h>
#include <boost/geometry/extensions/index/rtree/visitors/gl_draw.hpp>

typedef boost::geometry::model::point<float, 2, boost::geometry::cs::cartesian> P;
typedef boost::geometry::model::box<P> B;
boost::geometry::index::rtree< std::pair<B, size_t> > t;

void render_scene(void)
{
    boost::geometry::index::gl_draw(t, 0, 1, 20000.0f);
}

void resize(int w, int h)
{
    if ( h == 0 )
        h = 1;

    float ratio = float(w) / h;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glViewport(0, 0, w, h);

    gluPerspective(45.0, ratio, 1, 10000000.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(
        2000000.0, 2000000.0, 2000000.0, 
        0.0, 0.0, -1.0,
        0.0, 1.0, 0.0);
}

int main(int argc, char **argv)
{
    boost::timer tim;

    // randomize boxes
    const size_t n = 1000000;
    //const size_t n = 300;
    const size_t ns = 100000;

    std::ifstream file_cfg("config.txt");
    std::ifstream file("test_coords.txt");

    std::cout << "loading data\n";
    std::vector< std::pair<float, float> > coords(n);
    for ( size_t i = 0 ; i < n ; ++i )
    {
        file >> coords[i].first;
        file >> coords[i].second;
    }
    std::cout << "loaded\n";
    
    std::cin.get();

    size_t max_elems, min_elems;
    file_cfg >> max_elems;
    file_cfg >> min_elems;
    std::cout << "max: " << max_elems << ", min: " << min_elems << "\n";

    t = boost::geometry::index::rtree< std::pair<B, size_t> > (max_elems, min_elems);

    //std::cout << "inserting time test...\n";
    //tim.restart();
    //for (size_t i = 0 ; i < n ; ++i )
    //{
    //    float x = coords[i].first;
    //    float y = coords[i].second;
    //    B b(P(x - 0.5f, y - 0.5f), P(x + 0.5f, y + 0.5f));

    //    // Zle wyswietla dane, obcina czesc ulamkowa
    //    // Zle buduje drzewo dla i == 228

    //    //TEST
    //    /*if ( i == 228 )
    //    {
    //        std::cout << std::fixed << x << ", " << y << "\n";
    //        boost::geometry::index::detail::rtree::visitors::detail::print_indexable(std::cout, b);
    //    }*/

    //    t.insert(std::make_pair(b, i));

    //    //TEST
    //    /*if ( !boost::geometry::index::are_boxes_ok(t) )
    //    {
    //        std::ofstream log("log1.txt", std::ofstream::trunc);
    //        log << std::fixed << i << " - " << x << ", " << y << " - inserted: ";
    //        boost::geometry::index::detail::rtree::visitors::detail::print_indexable(log, b);
    //        log << '\n';
    //        log << ( boost::geometry::index::are_boxes_ok(t) ? "boxes OK" : "WRONG BOXES!" );
    //        log << '\n' << t;
    //    }*/
    //}
    //std::cout << "time: " << tim.elapsed() << "s\n";

    {
        std::cout << "loading tree structure...\n";
        std::ifstream is("save.txt");
        t.load(is);
        std::cout << "done.\n";
    }

    std::cout << "searching time test...\n";
    tim.restart();    
    size_t temp = 0;
    for (size_t i = 0 ; i < ns ; ++i )
    {
        float x = coords[i].first;
        float y = coords[i].second;
        std::deque< std::pair<B, size_t> > result = t.find(B(P(x - 10, y - 10), P(x + 10, y + 10)));
        temp += result.size();
    }
    std::cout << "time: " << tim.elapsed() << "s\n";
    std::cout << temp << "\n";

    std::cin.get();

    //TEST
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_SINGLE | GLUT_RGBA);
    glutInitWindowPosition(100,100);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Mouse click to insert new value");

    glutDisplayFunc(render_scene);
    glutReshapeFunc(resize);

    glutMainLoop();

    return 0;
}
