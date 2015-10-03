#include "MyFrame.hpp"
#include "MyCanvas.hpp"
#include <iomanip> /* for setiosflags and so */
#include "../PDF.hpp"

//#include "utf8.hpp"

/* Private class */
class Metafile:public PDF::Media
{
  private:
		wxPaintDC & dc;
		PDF::CTM m;
		int rot;
		double page_height;
		bool m_debug;
		double m_font_size;
//		double abs(double x) { return x<0?-x:x; }
  public:
    typedef PDF::Point Point;
    Metafile(wxPaintDC & theDC, int r):dc(theDC),rot(r),m_debug(false),m_font_size(10.0) { }
    virtual ~Metafile() { };
		virtual const PDF::CTM & Matrix() { return m; }
		virtual void SetFont(const PDF::Font * font, double size);
    virtual void Text(Point pos, double rotation, std::wstring text, double width, double height);
//    virtual void Text(Point pos, double rotation, std::wstring text);
    virtual void Line(const Point & p1, const Point & p2);
    virtual void Size(Point size);
		virtual void Debug(std::string s)
		{
			if(m_debug)
				std::clog << "DEBUG: " << s << std::endl;
		}
		void debug(bool d) { m_debug = d; }
};


BEGIN_EVENT_TABLE(MyCanvas, wxScrolledWindow)
	EVT_PAINT  (MyCanvas::OnPaint)
	EVT_MOTION (MyCanvas::OnMouseMove)
END_EVENT_TABLE()

MyCanvas::MyCanvas(wxWindow *parent)
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

	Metafile m(dc, m_rotation);
	m.debug(m_debug);
	m_page->draw(&m);
}

void MyCanvas::OnMouseMove(wxMouseEvent &event)
{
	wxClientDC dc(this);
	PrepareDC(dc);
	m_owner->PrepareDC(dc);
#if 0 // XXX different owner in pdfdig!!!
	wxPoint pos = event.GetPosition();
	long x = dc.DeviceToLogicalX( pos.x );
	long y = dc.DeviceToLogicalY( pos.y );
	wxString str;
	str.Printf( wxT("Current mouse position: %d,%d"), (int)x, (int)y );
	static_cast<MyFrame*>(m_owner)->SetStatusText( str );
#endif
}




void Metafile::Size(Point size)
{
	/*
	 * We do not invert y-coordinate in ctm because this makes text rotation
	 * angle calculation invalid.
	 *
	 */
	const double scale = 1.0;
	m.set_unity();
	m.scale(scale, scale);
	switch(rot) {
		case 0:
			page_height = size.y;
			break;
		case 1:
			page_height = size.x;
			m.rotate(90.0);
			m.offset(scale*size.y, 0);
			break;
		case 2:
			page_height = size.y;
			m.rotate(180.0);
			m.offset(scale*size.x, scale*size.y);
			break;
		case 3:
			page_height = size.x;
			m.rotate(270.0);
			m.offset(0, scale*size.x);
			break;
		default:
			break;
	}
	page_height *= scale;
}

void Metafile::SetFont(const PDF::Font * font, double size)
{
	if(size < 4) size = 4;
	m_font_size = size;
//	wxFont* f = wxTheFontList->FindOrCreateFont(6.0*size, wxSWISS, wxNORMAL /*wxITALIC*/, wxNORMAL/*wxBOLD*/);
	wxFont* f = wxTheFontList->FindOrCreateFont((int)size, wxSWISS, wxNORMAL /*wxITALIC*/, wxNORMAL/*wxBOLD*/);
	dc.SetFont(*f);
}

void Metafile::Text(Point pos, double rotation, std::wstring text, double width, double height)
//void Metafile::Text(Point pos, double rotation, std::wstring text)
{
	if(m_debug)
		std::wclog << L"TEXT(" << pos.x << L',' << pos.y << L',' << rotation << ") \"" << text << L"\" (" << text.length() << L" chars)(" << width << L'x' << height << L")" << std::endl;

	/*
	 * Add font height to start coordinate because DrawRotatedText coords begins
	 * at upper left corner, not like pdf (bottom left)
	 */
	double a = rotation*M_PI/180.0;
	pos.x-=m_font_size*sin(a);
	pos.y+=m_font_size*cos(a);
	dc.SetBrush(wxBrush(*wxCYAN, wxSOLID));
	dc.SetPen(wxPen(*wxCYAN, 0, wxSOLID));
	dc.DrawRectangle(wxCoord(pos.x), wxCoord(page_height - pos.y), wxCoord(width*cos(a)+height*sin(a)), wxCoord(height*cos(a)-width*sin(a)));
	dc.DrawRotatedText(text, wxCoord(pos.x), wxCoord(page_height - pos.y), rotation);
}

void Metafile::Line(const Point & p1, const Point & p2)
{
//	dc.SetPen( wxPen( wxT("red"), 8 /* width */, wxSOLID) );
	dc.SetPen(wxPen(*wxBLACK, 0, wxSOLID));
	dc.DrawLine( wxCoord(p1.x), wxCoord(page_height - p1.y), wxCoord(p2.x), wxCoord(page_height - p2.y) );
	if(m_debug)
		std::clog << "LINE " << p1.dump() << '-' << p2.dump() << std::endl;
}

