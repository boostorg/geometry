// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.Index - example
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <gl/glut.h>

//#define BOOST_GEOMETRY_INDEX_USE_VARIANT_NODES
#include <boost/geometry/extensions/index/rtree/rtree.hpp>

#include <boost/geometry/extensions/index/rtree/visitors/gl_draw.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/print.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/are_boxes_ok.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/are_levels_ok.hpp>

typedef boost::geometry::model::point<float, 2, boost::geometry::cs::cartesian> P;
typedef boost::geometry::model::box<P> B;
//boost::geometry::index::rtree<B> t(2, 1);

boost::geometry::index::rtree<
    B,
    boost::geometry::index::rstar<4, 2> > t;
std::vector<B> vect;

size_t found_count = 0;
P search_point;
float min_distance = 20;
float max_distance = 30;
size_t count = 10;
std::vector<B> nearest_boxes;

void draw_search_area()
{
    float x = boost::geometry::get<0>(search_point);
    float y = boost::geometry::get<1>(search_point);
    float z = t.depth();

    // search point
    glBegin(GL_TRIANGLES);
    glVertex3f(x, y, z);
    glVertex3f(x + 1, y, z);
    glVertex3f(x + 1, y + 1, z);
    glEnd();

    // search min circle

    glBegin(GL_LINE_LOOP);
    for(float a = 0 ; a < 3.14158f * 2 ; a += 3.14158f / 180)
        glVertex3f(x + min_distance * ::cos(a), y + min_distance * ::sin(a), z);
    glEnd();

    // search max circle

    glBegin(GL_LINE_LOOP);
    for(float a = 0 ; a < 3.14158f * 2 ; a += 3.14158f / 180)
        glVertex3f(x + max_distance * ::cos(a), y + max_distance * ::sin(a), z);
    glEnd();
}

void render_scene(void)
{
    glClear(GL_COLOR_BUFFER_BIT);

    boost::geometry::index::gl_draw(t);

    if ( found_count > 0 )
    {
        glColor3f(1.0f, 0.5f, 0.0f);
        draw_search_area();

        for ( size_t i = 0 ; i < nearest_boxes.size() ; ++i )
            boost::geometry::index::detail::rtree::visitors::detail::gl_draw_indexable(nearest_boxes[i], t.depth());
    }

    glFlush();
}

void resize(int w, int h)
{
    if ( h == 0 )
        h = 1;

    float ratio = float(w) / h;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glViewport(0, 0, w, h);

    gluPerspective(45, ratio, 1, 1000);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(
        150.0f, 150.0f, 150.0f, 
        50.0f, 50.0f, -1.0f,
        0.0f, 1.0f, 0.0f);
}

void mouse(int button, int state, int x, int y)
{
    namespace bg = boost::geometry;
    namespace bgi = bg::index;

    if ( button == GLUT_LEFT_BUTTON && state == GLUT_DOWN )
    {
        float x = ( rand() % 100 );
        float y = ( rand() % 100 );
        float w = ( rand() % 2 ) + 1;
        float h = ( rand() % 2 ) + 1;

        B b(P(x - w, y - h),P(x + w, y + h));

        boost::geometry::index::insert(t, b);
        vect.push_back(b);

        std::cout << "inserted: ";
        bgi::detail::rtree::visitors::detail::print_indexable(std::cout, b);
        std::cout << '\n';

        std::cout << "\n" << t << "\n";
        std::cout << ( bgi::are_boxes_ok(t) ? "boxes OK\n" : "WRONG BOXES!\n" );
        std::cout << ( bgi::are_levels_ok(t) ? "levels OK\n" : "WRONG LEVELS!\n" );
        std::cout << "\n";
    }
    else if ( button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN )
    {
        if ( vect.empty() )
            return;

        size_t i = rand() % vect.size();
        B b = vect[i];

        bgi::remove(t, b);
        vect.erase(vect.begin() + i);

        std::cout << "removed: ";
        bgi::detail::rtree::visitors::detail::print_indexable(std::cout, b);
        std::cout << '\n';

        std::cout << "\n" << t << "\n";
        std::cout << ( bgi::are_boxes_ok(t) ? "boxes OK\n" : "WRONG BOXES!\n" );
        std::cout << ( bgi::are_levels_ok(t) ? "levels OK\n" : "WRONG LEVELS!\n" );
        std::cout << "\n";
    }
    else if ( button == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN )
    {
        float x = ( rand() % 1000 ) / 10.0f;
        float y = ( rand() % 1000 ) / 10.0f;

        search_point = P(x, y);
        nearest_boxes.clear();
        found_count = t.nearest(bgi::distance_centroid(search_point, min_distance, max_distance), count, std::back_inserter(nearest_boxes));

        if ( found_count > 0 )
        {
            std::cout << "search point: ";
            bgi::detail::rtree::visitors::detail::print_indexable(std::cout, search_point);
            std::cout << "\nfound: ";
            for ( size_t i = 0 ; i < nearest_boxes.size() ; ++i )
            {
                bgi::detail::rtree::visitors::detail::print_indexable(std::cout, nearest_boxes[i]);
                std::cout << '\n';
            }
        }
        else
            std::cout << "nearest not found\n";

        std::cout << "\n" << t << "\n";
        std::cout << ( bgi::are_boxes_ok(t) ? "boxes OK\n" : "WRONG BOXES!\n" );
        std::cout << ( bgi::are_levels_ok(t) ? "levels OK\n" : "WRONG LEVELS!\n" );
        std::cout << "\n";
    }

    glutPostRedisplay();
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_SINGLE | GLUT_RGBA);
    glutInitWindowPosition(100,100);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Mouse click to insert new value");

    glutDisplayFunc(render_scene);
    glutReshapeFunc(resize);
    glutMouseFunc(mouse);

    std::cout << "Mouse click to insert new value";

    glutMainLoop();

    return 0;
}
