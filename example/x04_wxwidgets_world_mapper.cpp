// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// wxWidgets World Mapper example


// #define EXAMPLE_WX_USE_GRAPHICS_CONTEXT 1

// For Boost/uBLAS:
#define _SCL_SECURE_NO_WARNINGS 1

#include <fstream>
#include <sstream>

#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>

#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/register/point.hpp>
#include <boost/geometry/multi/multi.hpp>
#include <boost/geometry/geometries/cartesian2d.hpp>
#include <boost/geometry/extensions/algorithms/selected.hpp>
#include <boost/geometry/extensions/gis/io/wkt/read_wkt.hpp>
#include <boost/geometry/extensions/gis/io/wkt/read_wkt_multi.hpp>


// wxWidgets, if these headers are NOT found, adapt include path (and lib path)

#include "wx/wx.h"
#include "wx/math.h"
#include "wx/stockitem.h"


#ifdef EXAMPLE_WX_USE_GRAPHICS_CONTEXT
#include "wx/graphics.h"
#endif


// Adapt wxWidgets points to Boost.Geometry points such that they can be used 
// in e.g. transformations (see below)
BOOST_GEOMETRY_REGISTER_POINT_2D(wxPoint, int, cs::cartesian, x, y)
BOOST_GEOMETRY_REGISTER_POINT_2D(wxRealPoint, double, cs::cartesian, x, y)


typedef boost::geometry::multi_polygon<boost::geometry::polygon_2d> country_type;


// ----------------------------------------------------------------------------
// Read an ASCII file containing WKT's
// ----------------------------------------------------------------------------
template <typename Geometry, typename Box>
void read_wkt(std::string const& filename, std::vector<Geometry>& geometries, Box& m_box)
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
                boost::geometry::combine(m_box, boost::geometry::make_envelope<Box>(geometry));
            }
        }
    }
}





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

private:
    void DrawCountries(wxDC& dc);
    void DrawCountry(wxDC& dc, country_type const& country);

    void OnPaint(wxPaintEvent& );
    void OnMouseMove(wxMouseEvent&);

    typedef boost::geometry::strategy::transform::map_transformer
        <
            boost::geometry::point_2d, wxPoint, 
            true, true
        > map_transformer_type;

    typedef boost::geometry::strategy::transform::inverse_transformer
        <
            wxPoint, boost::geometry::point_2d
        > inverse_transformer_type;

    boost::shared_ptr<map_transformer_type> m_map_transformer;
    boost::shared_ptr<inverse_transformer_type> m_inverse_transformer;

    boost::geometry::box_2d m_box;
    std::vector<country_type> m_countries;
    int m_focus;

    wxBrush m_orange;
    wxFrame* m_owner;

DECLARE_EVENT_TABLE()
};



// ----------------------------------------------------------------------------
class HelloWorldApp: public wxApp
{
public:
    bool OnInit()
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
};



// ----------------------------------------------------------------------------
HelloWorldFrame::HelloWorldFrame(wxFrame *frame, const wxString& title, const wxPoint& pos, const wxSize& size)
    : wxFrame(frame, wxID_ANY, title, pos, size, wxDEFAULT_FRAME_STYLE | wxFULL_REPAINT_ON_RESIZE )
{
    CreateStatusBar(2);
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
    , m_owner(frame)
    , m_focus(-1)
{
    boost::geometry::assign_inverse(m_box);
    read_wkt("data/world.wkt", m_countries, m_box);
    m_orange = wxBrush(wxColour(255, 128, 0), wxSOLID);
}



void HelloWorldCanvas::OnMouseMove(wxMouseEvent &event)
{
    namespace bg = boost::geometry;

    if (m_inverse_transformer)
    {
        // Boiler-plate wxWidgets code
        wxClientDC dc(this);
        PrepareDC(dc);
        m_owner->PrepareDC(dc);

        // Transform the point to Lon/Lat
        bg::point_2d point;
        bg::transform(event.GetPosition(), point, *m_inverse_transformer);

        // Determine selected object
        int i = 0;
        int previous_focus = m_focus;
        m_focus = -1;
        BOOST_FOREACH(country_type const& country, m_countries)
        {
            if (bg::selected(country, point, 0))
            {
                m_focus = i;
            }
            i++;
        }

        // On change:
        if (m_focus != previous_focus)
        {
            // Undraw old focus
            if (previous_focus >= 0)
            {
                dc.SetBrush(*wxWHITE_BRUSH);
                DrawCountry(dc, m_countries[previous_focus]);
            }
            // Draw new focus
            if (m_focus >= 0)
            {
                dc.SetBrush(m_orange);
                DrawCountry(dc, m_countries[m_focus]);
            }
        }

        // Create a string and set it in the status text
        std::ostringstream out;
        out << "Position: " << point.x() << ", " << point.y() << " " << m_focus << " " << previous_focus;
        m_owner->SetStatusText(wxString(out.str().c_str(), wxConvUTF8));
    }
}



void HelloWorldCanvas::OnPaint(wxPaintEvent& )
{
#if defined(EXAMPLE_WX_USE_GRAPHICS_CONTEXT)
    wxPaintDC pdc(this);
    wxGCDC gdc(pdc);
    wxDC& dc = (wxDC&) gdc;
#else
    wxPaintDC dc(this);
#endif

    PrepareDC(dc);

    static bool running = false;
    if (! running)
    {
        running = true;

        // Update the transformers
        wxSize sz = dc.GetSize();
        m_map_transformer.reset(new map_transformer_type(m_box, sz.x, sz.y));
        m_inverse_transformer.reset(new inverse_transformer_type(*m_map_transformer));

        DrawCountries(dc);

        running = false;
    }
}


void HelloWorldCanvas::DrawCountries(wxDC& dc)
{
    namespace bg = boost::geometry;

    dc.SetBackground(*wxLIGHT_GREY_BRUSH);
    dc.Clear();

    BOOST_FOREACH(country_type const& country, m_countries)
    {
        DrawCountry(dc, country);
    }
    if (m_focus != -1)
    {
        dc.SetBrush(m_orange);
        DrawCountry(dc, m_countries[-1]);
    }
}


void HelloWorldCanvas::DrawCountry(wxDC& dc, country_type const& country)
{
    namespace bg = boost::geometry;

    BOOST_FOREACH(bg::polygon_2d const& poly, country)
    {
        bg::linear_ring<wxPoint> wx_ring;
        // Use only outer, holes are (for the moment) ignored
        bg::transform(poly.outer(), wx_ring, *m_map_transformer);

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

// ----------------------------------------------------------------------------


BEGIN_EVENT_TABLE(HelloWorldFrame, wxFrame)
    EVT_CLOSE(HelloWorldFrame::OnCloseWindow)
    EVT_MENU(wxID_EXIT, HelloWorldFrame::OnExit)
END_EVENT_TABLE()


BEGIN_EVENT_TABLE(HelloWorldCanvas, wxWindow)
    EVT_PAINT(HelloWorldCanvas::OnPaint)
    EVT_MOTION(HelloWorldCanvas::OnMouseMove)
END_EVENT_TABLE()


IMPLEMENT_APP(HelloWorldApp)
