// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// wxWidgets World Mapper example

#include <fstream>

#include <boost/foreach.hpp>

#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/register/point.hpp>
#include <boost/geometry/multi/multi.hpp>
#include <boost/geometry/geometries/cartesian2d.hpp>
#include <boost/geometry/extensions/gis/io/wkt/read_wkt.hpp>
#include <boost/geometry/extensions/gis/io/wkt/read_wkt_multi.hpp>

// wxWidgets, if these headers are NOT found, adapt include path (and lib path)
#include "wx/wx.h"
#include "wx/math.h"
#include "wx/stockitem.h"


// Adapt wxWidgets points to Boost.Geometry points such that they can be used 
// in e.g. transformations (see below)
BOOST_GEOMETRY_REGISTER_POINT_2D(wxPoint, int, cs::cartesian, x, y)
BOOST_GEOMETRY_REGISTER_POINT_2D(wxRealPoint, double, cs::cartesian, x, y)


typedef boost::geometry::multi_polygon<boost::geometry::polygon_2d> multi_type;


// ----------------------------------------------------------------------------
// Read an ASCII file containing WKT's
// ----------------------------------------------------------------------------
template <typename Geometry, typename Box>
void read_wkt(std::string const& filename, std::vector<Geometry>& geometries, Box& box)
{
    std::ifstream cpp_file(filename.c_str());
    if (cpp_file.is_open())
    {
        while (! cpp_file.eof() )
        {
            std::string line;
            std::getline(cpp_file, line);
            Geometry geometry;
            if (! line.empty())
            {
                Geometry geometry;
                boost::geometry::read_wkt(line, geometry);
                geometries.push_back(geometry);
                boost::geometry::combine(box, boost::geometry::make_envelope<Box>(geometry));
            }
        }
    }
}



// ----------------------------------------------------------------------------
class HelloWorldApp: public wxApp
{
public:
    bool OnInit();
};


// ----------------------------------------------------------------------------
class HelloWorldFrame: public wxFrame
{
public:
    HelloWorldFrame(wxFrame *frame, const wxString& title, const wxPoint& pos, const wxSize& size);

    void OnCloseWindow(wxCloseEvent& );
    void OnExit(wxCommandEvent& );

    DECLARE_EVENT_TABLE()
};


// ----------------------------------------------------------------------------
class HelloWorldCanvas: public wxWindow
{
public:
    HelloWorldCanvas(wxFrame *frame);
    void DrawCountries(wxDC& dc);

private:
    void OnPaint(wxPaintEvent& );

    boost::geometry::box_2d box;
    std::vector<multi_type> countries;


DECLARE_EVENT_TABLE()
};


// ----------------------------------------------------------------------------
bool HelloWorldApp::OnInit()
{
    // Create the main frame window
    HelloWorldFrame *frame = new HelloWorldFrame(NULL, _T("Boost.Geometry for wxWidgets - Hello World!"), wxDefaultPosition, wxSize(640, 480));

    wxMenu *file_menu = new wxMenu;
    file_menu->Append(wxID_EXIT, wxGetStockLabel(wxID_EXIT));
    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(file_menu, _T("&File"));
    frame->SetMenuBar(menuBar);

    int width, height;
    frame->GetClientSize(&width, &height);

    (void) new HelloWorldCanvas(frame);

    // Show the frame
    frame->Show(true);

    return true;
}


// ----------------------------------------------------------------------------
HelloWorldFrame::HelloWorldFrame(wxFrame *frame, const wxString& title, const wxPoint& pos, const wxSize& size):
    wxFrame(frame, wxID_ANY, title, pos, size, wxDEFAULT_FRAME_STYLE | wxFULL_REPAINT_ON_RESIZE )
{
}


void HelloWorldFrame::OnExit(wxCommandEvent& )
{
    this->Destroy();
}

void HelloWorldFrame::OnCloseWindow(wxCloseEvent& )
{
    static bool destroyed = false;
    if (! destroyed)
    {
        this->Destroy();
        destroyed = true;
    }
}


// ----------------------------------------------------------------------------
HelloWorldCanvas::HelloWorldCanvas(wxFrame *frame)
    : wxWindow(frame, wxID_ANY)
{
    boost::geometry::assign_inverse(box);
    read_wkt("data/world.wkt", countries, box);
}


void HelloWorldCanvas::OnPaint(wxPaintEvent& )
{
    wxPaintDC dc(this);
    PrepareDC(dc);

    static bool running = false;
    if (! running)
    {
        running = true;
        DrawCountries(dc);
        running = false;
    }
}

void HelloWorldCanvas::DrawCountries(wxDC& dc)
{
    dc.SetBackground(*wxLIGHT_GREY_BRUSH);
    dc.Clear();

    namespace bg = boost::geometry;

    wxSize sz = GetClientSize();
    boost::geometry::strategy::transform::map_transformer
        <
            bg::point_2d,
            wxPoint, true, true
        > matrix(box, sz.x, sz.y);

    BOOST_FOREACH(multi_type const& country, countries)
    {
        BOOST_FOREACH(bg::polygon_2d const& poly, country)
        {
            bg::linear_ring<wxPoint> wx_ring;
            // Use only outer, holes are (for the moment) ignored
            bg::transform(poly.outer(), wx_ring, matrix);

            // Todo: avoid converting (use pointer-compatible container)
            wxPoint* points = new wxPoint[boost::size(wx_ring)];
            int n = 0;
            BOOST_FOREACH(wxPoint const& p, wx_ring)
            {
                points[n++] = p;
            }
            // end todo

            dc.DrawPolygon(n, points);

            delete[] points;
        }
    }
}


// ----------------------------------------------------------------------------


BEGIN_EVENT_TABLE(HelloWorldFrame, wxFrame)
  EVT_CLOSE(HelloWorldFrame::OnCloseWindow)
  EVT_MENU(wxID_EXIT, HelloWorldFrame::OnExit)
END_EVENT_TABLE()


BEGIN_EVENT_TABLE(HelloWorldCanvas, wxWindow)
  EVT_PAINT(HelloWorldCanvas::OnPaint)
END_EVENT_TABLE()


IMPLEMENT_APP(HelloWorldApp)
