#include <gl/glut.h>
#define BOOST_GEOMETRY_INDEX_RTREE_ENABLE_GL_DRAW

#include <boost/geometry/extensions/index/rtree/rtree.hpp>

#include <boost/geometry/extensions/index/rtree/visitors/gl_draw.hpp>

typedef boost::geometry::model::point<float, 2, boost::geometry::cs::cartesian> P;
typedef boost::geometry::model::box<P> B;
boost::geometry::index::rtree<B> t;

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
        5.0f, 5.0f, 15.0f, 
        5.0f, 5.0f, -1.0f,
        0.0f, 1.0f, 0.0f);
}

void mouse(int button, int state, int x, int y)
{
    if ( state == GLUT_DOWN )
    {
        float x = ( rand() % 10000 ) / 1000.0f;
        float y = ( rand() % 10000 ) / 1000.0f;
        float w = ( rand() % 10000 ) / 100000.0f;
        float h = ( rand() % 10000 ) / 100000.0f;

        boost::geometry::index::insert(t, B(P(x - w, y - h),P(x + w, y + h)));

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
