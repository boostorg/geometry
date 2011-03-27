#include <gl/glut.h>
#define BOOST_GEOMETRY_INDEX_RTREE_ENABLE_GL_DRAW

#include <boost/geometry/extensions/index/rtree/rtree.hpp>

#include <boost/geometry/extensions/index/rtree/visitors/gl_draw.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/print.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/rtree_are_boxes_ok.hpp>

typedef boost::geometry::model::point<float, 2, boost::geometry::cs::cartesian> P;
typedef boost::geometry::model::box<P> B;
boost::geometry::index::rtree<B> t(2, 1);

void render_scene(void)
{
    boost::geometry::index::gl_draw(t);
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
    if ( state == GLUT_DOWN )
    {
        float x = ( rand() % 100 );
        float y = ( rand() % 100 );
        float w = ( rand() % 2 ) + 1;
        float h = ( rand() % 2 ) + 1;

        B b(P(x - w, y - h),P(x + w, y + h));

        boost::geometry::index::insert(t, b);

        std::cout << "\n\n\n" << t << "\n\n";
        
        std::cout << "inserted: ";
        boost::geometry::index::visitors::detail::rtree_print_indexable(std::cout, b);
        std::cout << '\n';
        std::cout << ( boost::geometry::index::rtree_are_boxes_ok(t) ? "boxes OK" : "WRONG BOXES!" );

        glutPostRedisplay();
    }
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
