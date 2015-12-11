#include <wx/wx.h>
#include "PagePaintHelper.h"

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
}

void PagePaintHelper::SetFont(const PDF::Font * font, double size)
{
	if(size < 4) size = 4;
	m_font_size = size;
	wxFont* f = wxTheFontList->FindOrCreateFont(int(size/1.8 /* empirical constant to get wxWidgets font of needed size */), wxSWISS, wxNORMAL /*wxITALIC*/, wxNORMAL/*wxBOLD*/);
	dc.SetFont(*f);
}

void PagePaintHelper::Text(Point pos, double rotation, std::wstring text, double width, double height)
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
	dc.DrawRectangle(wxCoord(pos.x), wxCoord(m_page_height - pos.y), wxCoord(width*cos(a)+height*sin(a)), wxCoord(height*cos(a)-width*sin(a)));
	dc.DrawRotatedText(text, wxCoord(pos.x), wxCoord(m_page_height - pos.y), rotation);
}

void PagePaintHelper::Line(const Point & p1, const Point & p2)
{
	dc.SetPen(wxPen(*wxBLACK, 0, wxSOLID));
	dc.DrawLine( wxCoord(p1.x), wxCoord(m_page_height - p1.y), wxCoord(p2.x), wxCoord(m_page_height - p2.y) );
	if(m_debug)
		std::clog << "LINE " << p1.dump() << '-' << p2.dump() << std::endl;
}





