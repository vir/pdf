#include "MyFrame.hpp"
#include "MyCanvas.hpp"
#include <iomanip> /* for setiosflags and so */
#include "../PDF.hpp"
#include "MyDocument.hpp"

//#include "utf8.hpp"

/* Private class */
class Metafile:public PDF::Media
{
  private:
		wxPaintDC & dc;
		PDF::CTM m;
		int rot;
		double height;
//		double abs(double x) { return x<0?-x:x; }
  public:
    typedef PDF::Point Point;
    Metafile(wxPaintDC & theDC, int r):dc(theDC),rot(r) { }
    virtual ~Metafile() { };
		virtual const PDF::CTM & Matrix() { return m; }
    virtual void Text(Point pos, double rotation, std::wstring text);
    virtual void Line(const Point & p1, const Point & p2);
    virtual void Size(Point size)
		{
			m.set_unity();
			switch(rot) {
				case 0:
					height = size.y;
					/* Normal page layout, invert coordinates for html's upsidedown y axis */
//					m.scale(1, -1);
//					m.offset(0, size.y);
					break;
				case 1:
					/* Rotate landscape page 90deg CW */
					height = size.x;
					m.rotate(90.0);
//					m.scale(-1, 1);
//					m.offset(size.y, 0);
					m.offset(size.y, 0);
				case 2:
					height = size.y;
					m.rotate(180.0);
					m.offset(size.x, size.y);
					break;
				case 3:
					height = size.x;
					m.rotate(270.0);
					m.offset(0, size.x);
					break;
				default:
					break;
			}
			std::clog << "Device matix set to:" << std::endl;
			m.dump();
		}
		virtual void Debug(std::string s)
		{
			std::clog << "DEBUG: " << s << std::endl;
		}
};


BEGIN_EVENT_TABLE(MyCanvas, wxScrolledWindow)
	EVT_PAINT  (MyCanvas::OnPaint)
	EVT_MOTION (MyCanvas::OnMouseMove)
END_EVENT_TABLE()

MyCanvas::MyCanvas(MyFrame *parent)
	: wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL | wxVSCROLL | wxNO_FULL_REPAINT_ON_RESIZE)
{
	m_owner = parent;
	m_rotation = 0;
}

void MyCanvas::OnPaint(wxPaintEvent &WXUNUSED(event))
{
	wxPaintDC dc(this);
	PrepareDC(dc);

	m_owner->PrepareDC(dc);

	dc.Clear();

#if 0
	dc.DrawText(wxT("Testing"), 40, 60); 
	dc.SetFont( *wxSWISS_FONT );
	dc.DrawRotatedText(wxT("Rotated text") , 40, 80, 45);
//	wxPen pen = *wxRED_PEN;
//	dc.SetPen(pen);
	dc.SetPen( wxPen( wxT("red"), 8 /* width */, wxSOLID) );
	dc.DrawLine( 10, 30,70, 90 );
#endif
	Metafile m(dc, m_rotation);
	theDocument->GetPageObject()->draw(&m);
}

void MyCanvas::OnMouseMove(wxMouseEvent &event)
{
#if wxUSE_STATUSBAR
	wxClientDC dc(this);
	PrepareDC(dc);
	m_owner->PrepareDC(dc);

	wxPoint pos = event.GetPosition();
	long x = dc.DeviceToLogicalX( pos.x );
	long y = dc.DeviceToLogicalY( pos.y );
	wxString str;
	str.Printf( wxT("Current mouse position: %d,%d"), (int)x, (int)y );
	m_owner->SetStatusText( str );
#else
	wxUnusedVar(event);
#endif // wxUSE_STATUSBAR
}





void Metafile::Text(Point pos, double rotation, std::wstring text)
{
//	dc.DrawText(text/*wxT("Testing")*/, pos.x, pos.y); 
	dc.SetFont( *wxSWISS_FONT );
	dc.DrawRotatedText(text, pos.x, height - pos.y, rotation);
	std::clog << "TEXT " << text.length() << " chars at " << pos.dump() << "(" << rotation << " degree angle)" << std::endl;
}

void Metafile::Line(const Point & p1, const Point & p2)
{
//	wxPen pen = *wxRED_PEN;
//	dc.SetPen(pen);

//	dc.SetPen( wxPen( wxT("red"), 8 /* width */, wxSOLID) );
	dc.DrawLine( p1.x, height - p1.y, p2.x, height - p2.y );
	std::clog << "LINE " << p1.dump() << '-' << p2.dump() << std::endl;
}

