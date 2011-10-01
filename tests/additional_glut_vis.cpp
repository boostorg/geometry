// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.Index - example
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <gl/glut.h>

#include <boost/geometry/extensions/index/rtree/rtree.hpp>

#include <boost/geometry/extensions/index/rtree/visitors/gl_draw.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/print.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/are_boxes_ok.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/are_levels_ok.hpp>

namespace bg = boost::geometry;
namespace bgi = bg::index;

typedef bg::model::point<float, 2, boost::geometry::cs::cartesian> P;
typedef bg::model::box<P> B;
//bgi::rtree<B> t(2, 1);

bgi::rtree<
    B,
    bgi::rstar<4, 2> > t;
std::vector<B> vect;

size_t found_count = 0;
P search_point;
float min_distance = 20;
float max_distance = 30;
size_t count = 10;
std::vector<B> nearest_boxes;
B search_box;

enum query_mode_type {
    qm_knn, qm_c, qm_d, qm_i, qm_o, qm_w, qm_nc, qm_nd, qm_ni, qm_no, qm_nw
} query_mode = qm_knn;

bool search_valid = false;

void knn()
{
    float x = ( rand() % 1000 ) / 10.0f;
    float y = ( rand() % 1000 ) / 10.0f;

    search_point = P(x, y);
    nearest_boxes.clear();
    found_count = t.nearest(
        bgi::bounded(
        search_point,
        bgi::far(min_distance),
        bgi::near(max_distance)),
        count,
        std::back_inserter(nearest_boxes)
        );

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
}

template <typename Predicate>
void query()
{
    float x = ( rand() % 1000 ) / 10.0f;
    float y = ( rand() % 1000 ) / 10.0f;
    float w = 10 + ( rand() % 1000 ) / 100.0f;
    float h = 10 + ( rand() % 1000 ) / 100.0f;

    search_box = B(P(x - w, y - h), P(x + w, y + h));
    nearest_boxes.clear();
    found_count = t.query(Predicate(search_box), std::back_inserter(nearest_boxes) );

    if ( found_count > 0 )
    {
        std::cout << "search box: ";
        bgi::detail::rtree::visitors::detail::print_indexable(std::cout, search_box);
        std::cout << "\nfound: ";
        for ( size_t i = 0 ; i < nearest_boxes.size() ; ++i )
        {
            bgi::detail::rtree::visitors::detail::print_indexable(std::cout, nearest_boxes[i]);
            std::cout << '\n';
        }
    }
    else
        std::cout << "nearest not found\n";
}

void search()
{
    if ( query_mode == qm_knn )
        knn();
    else if ( query_mode == qm_c )
        query< bgi::detail::covered_by<B> >();
    else if ( query_mode == qm_d )
        query< bgi::detail::disjoint<B> >();
    else if ( query_mode == qm_i )
        query< bgi::detail::intersects<B> >();
    else if ( query_mode == qm_o )
        query< bgi::detail::overlaps<B> >();
    else if ( query_mode == qm_w )
        query< bgi::detail::within<B> >();
    else if ( query_mode == qm_nc )
        query< bgi::detail::not_covered_by<B> >();
    else if ( query_mode == qm_nd )
        query< bgi::detail::not_disjoint<B> >();
    else if ( query_mode == qm_ni )
        query< bgi::detail::not_intersects<B> >();
    else if ( query_mode == qm_no )
        query< bgi::detail::not_overlaps<B> >();
    else if ( query_mode == qm_nw )
        query< bgi::detail::not_within<B> >();

    search_valid = true;
}

void draw_knn_area()
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

void draw_query_area()
{
    float x1 = boost::geometry::get<bg::min_corner, 0>(search_box);
    float y1 = boost::geometry::get<bg::min_corner, 1>(search_box);
    float x2 = boost::geometry::get<bg::max_corner, 0>(search_box);
    float y2 = boost::geometry::get<bg::max_corner, 1>(search_box);
    float z = t.depth();

    // search box
    glBegin(GL_LINE_LOOP);
        glVertex3f(x1, y1, z);
        glVertex3f(x2, y1, z);
        glVertex3f(x2, y2, z);
        glVertex3f(x1, y2, z);
    glEnd();
}

void render_scene(void)
{
    glClear(GL_COLOR_BUFFER_BIT);

    boost::geometry::index::gl_draw(t);

    if ( search_valid )
    {
        glColor3f(1.0f, 0.5f, 0.0f);

        if ( query_mode == qm_knn )
            draw_knn_area();
        else
            draw_query_area();

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
        120.0f, 120.0f, 120.0f, 
        50.0f, 50.0f, -1.0f,
        0.0f, 1.0f, 0.0f);
}

void mouse(int button, int state, int x, int y)
{
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

        std::cout << ( bgi::are_boxes_ok(t) ? "boxes OK\n" : "WRONG BOXES!\n" );
        std::cout << ( bgi::are_levels_ok(t) ? "levels OK\n" : "WRONG LEVELS!\n" );
        std::cout << "\n";

        search_valid = false;
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

        std::cout << ( bgi::are_boxes_ok(t) ? "boxes OK\n" : "WRONG BOXES!\n" );
        std::cout << ( bgi::are_levels_ok(t) ? "levels OK\n" : "WRONG LEVELS!\n" );
        std::cout << "\n";

        search_valid = false;
    }
    else if ( button == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN )
    {
        search();
    }

    glutPostRedisplay();
}

std::string current_line;

void keyboard(unsigned char key, int x, int y)
{
    if ( key == '\r' || key == '\n' )
    {
        if ( current_line == "t" )
        {
            std::cout << "\n" << t << "\n";
        }
        else
        {
            if ( current_line == "knn" )
                query_mode = qm_knn;
            else if ( current_line == "c" )
                query_mode = qm_c;
            else if ( current_line == "d" )
                query_mode = qm_d;
            else if ( current_line == "i" )
                query_mode = qm_i;
            else if ( current_line == "o" )
                query_mode = qm_o;
            else if ( current_line == "w" )
                query_mode = qm_w;
            else if ( current_line == "nc" )
                query_mode = qm_nc;
            else if ( current_line == "nd" )
                query_mode = qm_nd;
            else if ( current_line == "ni" )
                query_mode = qm_ni;
            else if ( current_line == "no" )
                query_mode = qm_no;
            else if ( current_line == "nw" )
                query_mode = qm_nw;
            
            search();
            glutPostRedisplay();
        }

        current_line.clear();
        std::cout << '\n';
    }
    else
    {
        current_line += key;
        std::cout << key;
    }
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_SINGLE | GLUT_RGBA);
    glutInitWindowPosition(100,100);
    glutInitWindowSize(800, 600);
    glutCreateWindow("boost::geometry::index::rtree GLUT test");

    glutDisplayFunc(render_scene);
    glutReshapeFunc(resize);
    glutMouseFunc(mouse);
    glutKeyboardFunc(keyboard);

    glutMainLoop();

    return 0;
}
