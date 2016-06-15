#include <wx/wx.h>
#include "PagePaintHelper.h"

PagePaintHelper::PagePaintHelper(wxDC & theDC, int r/*=0*/, double sc/*=1.0*/)
	: dc(theDC)
	, m_rotation(r)
	, m_font_size(10.0)
	, m_scale(sc)
	, m_break_op((unsigned int)-1)
	, m_draw_debug_stream(NULL)
	, m_page_debug_stream(NULL)
{
}

void PagePaintHelper::Size(Point size)
{
	/*
	 * We do not invert y-coordinate in ctm because this makes text rotation
	 * angle calculation invalid.
	 */
	m.set_unity();
	m.scale(m_scale, m_scale);
	switch(m_rotation) {
		case 0: m_page_height = size.y; break;
		case 1: m_page_height = size.x; m.rotate(90.0); m.offset(m_scale*size.y, 0); break;
		case 2: m_page_height = size.y; m.rotate(180.0); m.offset(m_scale*size.x, m_scale*size.y); break;
		case 3: m_page_height = size.x; m.rotate(270.0); m.offset(0, m_scale*size.x); break;
		default: break;
	}
	m_page_height *= m_scale;
	if(m_draw_debug_stream)
		*m_draw_debug_stream << "SIZE(" << size.x << ',' << size.y << "), scale: " << m_scale << ", page height: " << m_page_height << std::endl;
}

void PagePaintHelper::SetFont(const PDF::Font * font, double size)
{
	if(size < 4) size = 4;
	m_font_size = size;
	wxFont* f = wxTheFontList->FindOrCreateFont(int(size/1.8 /* empirical constant to get wxWidgets font of needed size */), wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL /*wxITALIC*/, wxFONTWEIGHT_NORMAL/*wxBOLD*/);
	dc.SetFont(*f);
}

void PagePaintHelper::Text(PDF::Rect pos, double angle, std::wstring text, bool visible, const PDF::GraphicsState& gs)
{
	if(m_draw_debug_stream)
		*m_draw_debug_stream << "TEXT(" << pos.x1 << ',' << pos.y1 << ',' << angle << ") \"" << wxString(text).utf8_str() << "\" (" << text.length() << " chars, " << pos.width() << 'x' << pos.height() << ")" << std::endl;

	/*
	 * Add font height to start coordinate because DrawRotatedText coords begins
	 * at upper left corner, not like pdf (bottom left)
	 */
	double a = angle*M_PI/180.0;
	double x = pos.x1 - m_font_size*sin(a);
	double y = pos.y1 + m_font_size*cos(a);
	double w = pos.width();
	double h = pos.height();
	dc.SetBrush(wxBrush(*(visible ? wxCYAN : wxGREEN), wxBRUSHSTYLE_SOLID));
	dc.SetPen(wxPen(*wxCYAN, 0, wxPENSTYLE_SOLID));
	dc.DrawRectangle( wxCoord(x), wxCoord(m_page_height - y),
		wxCoord(w*cos(a) + h*sin(a)), wxCoord(h*cos(a) - w*sin(a)));
	dc.DrawRotatedText(text, wxCoord(x), wxCoord(m_page_height - y), angle);
}

void PagePaintHelper::Line(const PDF::Point & p1, const PDF::Point & p2, const PDF::GraphicsState& gs)
{
	dc.SetPen(wxPen(*wxBLACK, 0, wxPENSTYLE_SOLID));
	dc.DrawLine( wxCoord(p1.x), wxCoord(m_page_height - p1.y), wxCoord(p2.x), wxCoord(m_page_height - p2.y) );
	if(m_draw_debug_stream)
		*m_draw_debug_stream << "LINE " << p1.dump() << '-' << p2.dump() << std::endl;
}

void PagePaintHelper::Debug(unsigned int opnum, std::string s, const PDF::GraphicsState& gs)
{
	if(m_page_debug_stream) {
		*m_page_debug_stream << s << std::endl;
	}
#ifdef _DEBUG
	if(m_break_op == opnum)
		DebugBreak();
#endif
}





