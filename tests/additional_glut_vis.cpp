// Boost.Geometry Index
// Additional tests

// Copyright (c) 2011-2012 Adam Wulkiewicz, Lodz, Poland.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <gl/glut.h>

#define BOOST_GEOMETRY_INDEX_ENABLE_DEBUG_INTERFACE

#include <boost/foreach.hpp>

#include <boost/geometry/extensions/index/rtree/rtree.hpp>

#include <boost/geometry/geometries/ring.hpp>
#include <boost/geometry/geometries/polygon.hpp>

#include <boost/geometry/extensions/index/rtree/visitors/gl_draw.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/print.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/are_boxes_ok.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/are_levels_ok.hpp>

namespace bg = boost::geometry;
namespace bgi = bg::index;

typedef bg::model::point<float, 2, boost::geometry::cs::cartesian> P;
typedef bg::model::box<P> B;
//bgi::rtree<B> t(2, 1);
typedef bg::model::ring<P> R;
typedef bg::model::polygon<P> Poly;

bgi::rtree<
    B,
    bgi::rstar<4, 2>
> t;
std::vector<B> vect;

size_t found_count = 0;
P search_point;
float min_distance = 10;
float max_distance = 30;
size_t count = 5;
std::vector<B> nearest_boxes;
B search_box;
R search_ring;
Poly search_poly;

enum query_mode_type {
    qm_knn, qm_c, qm_d, qm_i, qm_o, qm_w, qm_nc, qm_nd, qm_ni, qm_no, qm_nw, qm_all, qm_ri, qm_pi
} query_mode = qm_knn;

bool search_valid = false;

void knn()
{
    float x = ( rand() % 1000 ) / 10.0f;
    float y = ( rand() % 1000 ) / 10.0f;

    search_point = P(x, y);
    nearest_boxes.clear();
    found_count = t.nearest_query(
        bgi::bounded(
            search_point,
            bgi::to_furthest(min_distance),
            bgi::to_nearest(max_distance)
        ),
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
    if ( query_mode != qm_all )
    {
        float x = ( rand() % 1000 ) / 10.0f;
        float y = ( rand() % 1000 ) / 10.0f;
        float w = 10 + ( rand() % 1000 ) / 100.0f;
        float h = 10 + ( rand() % 1000 ) / 100.0f;

        search_box = B(P(x - w, y - h), P(x + w, y + h));
        nearest_boxes.clear();
        found_count = t.spatial_query(Predicate(search_box), std::back_inserter(nearest_boxes) );
    }
    else
    {
        search_box = t.box();
        nearest_boxes.clear();
        found_count = t.spatial_query(Predicate(search_box), std::back_inserter(nearest_boxes) );
    }

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
        std::cout << "boxes not found\n";
}

template <typename Predicate>
void query_ring()
{
    float x = ( rand() % 1000 ) / 10.0f;
    float y = ( rand() % 1000 ) / 10.0f;
    float w = 10 + ( rand() % 1000 ) / 100.0f;
    float h = 10 + ( rand() % 1000 ) / 100.0f;

    search_ring.clear();
    search_ring.push_back(P(x - w, y - h));
    search_ring.push_back(P(x - w/2, y - h));
    search_ring.push_back(P(x, y - 3*h/2));
    search_ring.push_back(P(x + w/2, y - h));
    search_ring.push_back(P(x + w, y - h));
    search_ring.push_back(P(x + w, y - h/2));
    search_ring.push_back(P(x + 3*w/2, y));
    search_ring.push_back(P(x + w, y + h/2));
    search_ring.push_back(P(x + w, y + h));
    search_ring.push_back(P(x + w/2, y + h));
    search_ring.push_back(P(x, y + 3*h/2));
    search_ring.push_back(P(x - w/2, y + h));
    search_ring.push_back(P(x - w, y + h));
    search_ring.push_back(P(x - w, y + h/2));
    search_ring.push_back(P(x - 3*w/2, y));
    search_ring.push_back(P(x - w, y - h/2));
    search_ring.push_back(P(x - w, y - h));
        
    nearest_boxes.clear();
    found_count = t.spatial_query(Predicate(search_ring), std::back_inserter(nearest_boxes) );
    
    if ( found_count > 0 )
    {
        std::cout << "search ring: ";
        BOOST_FOREACH(P const& p, search_ring)
        {
            bgi::detail::rtree::visitors::detail::print_indexable(std::cout, p);
            std::cout << ' ';
        }
        std::cout << "\nfound: ";
        for ( size_t i = 0 ; i < nearest_boxes.size() ; ++i )
        {
            bgi::detail::rtree::visitors::detail::print_indexable(std::cout, nearest_boxes[i]);
            std::cout << '\n';
        }
    }
    else
        std::cout << "boxes not found\n";
}

template <typename Predicate>
void query_poly()
{
    float x = ( rand() % 1000 ) / 10.0f;
    float y = ( rand() % 1000 ) / 10.0f;
    float w = 10 + ( rand() % 1000 ) / 100.0f;
    float h = 10 + ( rand() % 1000 ) / 100.0f;

    search_poly.clear();
    search_poly.outer().push_back(P(x - w, y - h));
    search_poly.outer().push_back(P(x - w/2, y - h));
    search_poly.outer().push_back(P(x, y - 3*h/2));
    search_poly.outer().push_back(P(x + w/2, y - h));
    search_poly.outer().push_back(P(x + w, y - h));
    search_poly.outer().push_back(P(x + w, y - h/2));
    search_poly.outer().push_back(P(x + 3*w/2, y));
    search_poly.outer().push_back(P(x + w, y + h/2));
    search_poly.outer().push_back(P(x + w, y + h));
    search_poly.outer().push_back(P(x + w/2, y + h));
    search_poly.outer().push_back(P(x, y + 3*h/2));
    search_poly.outer().push_back(P(x - w/2, y + h));
    search_poly.outer().push_back(P(x - w, y + h));
    search_poly.outer().push_back(P(x - w, y + h/2));
    search_poly.outer().push_back(P(x - 3*w/2, y));
    search_poly.outer().push_back(P(x - w, y - h/2));
    search_poly.outer().push_back(P(x - w, y - h));

    search_poly.inners().push_back(Poly::ring_type());
    search_poly.inners()[0].push_back(P(x - w/2, y - h/2));
    search_poly.inners()[0].push_back(P(x + w/2, y - h/2));
    search_poly.inners()[0].push_back(P(x + w/2, y + h/2));
    search_poly.inners()[0].push_back(P(x - w/2, y + h/2));
    search_poly.inners()[0].push_back(P(x - w/2, y - h/2));

    nearest_boxes.clear();
    found_count = t.spatial_query(Predicate(search_poly), std::back_inserter(nearest_boxes) );

    if ( found_count > 0 )
    {
        std::cout << "search poly outer: ";
        BOOST_FOREACH(P const& p, search_poly.outer())
        {
            bgi::detail::rtree::visitors::detail::print_indexable(std::cout, p);
            std::cout << ' ';
        }
        std::cout << "\nfound: ";
        for ( size_t i = 0 ; i < nearest_boxes.size() ; ++i )
        {
            bgi::detail::rtree::visitors::detail::print_indexable(std::cout, nearest_boxes[i]);
            std::cout << '\n';
        }
    }
    else
        std::cout << "boxes not found\n";
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
    else if ( query_mode == qm_all )
        query< bgi::detail::intersects<B> >();
    else if ( query_mode == qm_ri )
        query_ring< bgi::detail::intersects<R> >();
    else if ( query_mode == qm_pi )
        query_poly< bgi::detail::intersects<Poly> >();

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

void draw_query_box_area()
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

template <typename Range>
void draw_ring_area(Range const& range)
{
    float z = t.depth();

    // search box
    glBegin(GL_LINE_LOOP);
    
    BOOST_FOREACH(P const& p, range)
    {
        float x = boost::geometry::get<0>(p);
        float y = boost::geometry::get<1>(p);

        glVertex3f(x, y, z);
    }
    glEnd();
}

void draw_query_ring_area()
{
    draw_ring_area(search_ring);
}

void draw_query_poly_area()
{
    draw_ring_area(search_poly.outer());
    BOOST_FOREACH(Poly::ring_type const& r, search_poly.inners())
        draw_ring_area(r);
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
        else if ( query_mode == qm_ri )
            draw_query_ring_area();
        else if ( query_mode == qm_pi )
            draw_query_poly_area();
        else
            draw_query_box_area();

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

    //gluPerspective(45, ratio, 1, 1000);
    glOrtho(-150, 150, -150, 150, -150, 150);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    /*gluLookAt(
        120.0f, 120.0f, 120.0f, 
        50.0f, 50.0f, -1.0f,
        0.0f, 1.0f, 0.0f);*/
    gluLookAt(
        50.0f, 50.0f, 75.0f, 
        50.0f, 50.0f, -1.0f,
        0.0f, 1.0f, 0.0f);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glLineWidth(1.5f);

    srand(1);
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
        else if ( current_line == "rand" )
        {
            for ( size_t i = 0 ; i < 35 ; ++i )
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
            }

            std::cout << ( bgi::are_boxes_ok(t) ? "boxes OK\n" : "WRONG BOXES!\n" );
            std::cout << ( bgi::are_levels_ok(t) ? "levels OK\n" : "WRONG LEVELS!\n" );
            std::cout << "\n";

            search_valid = false;

            glutPostRedisplay();
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
            else if ( current_line == "all" )
                query_mode = qm_all;
            else if ( current_line == "ri" )
                query_mode = qm_ri;
            else if ( current_line == "pi" )
                query_mode = qm_pi;
            
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
    glutInitWindowSize(600, 600);
    glutCreateWindow("boost::geometry::index::rtree GLUT test");

    glutDisplayFunc(render_scene);
    glutReshapeFunc(resize);
    glutMouseFunc(mouse);
    glutKeyboardFunc(keyboard);

    glutMainLoop();

    return 0;
}
