#ifndef PAGEPAINTHELPER_H_INCLUDED
#define PAGEPAINTHELPER_H_INCLUDED

#include <wx/wx.h>
#include "../libpdf/Media.hpp"

class PagePaintHelper:public PDF::Media
{
private:
	wxDC & dc;
	PDF::CTM m;
	int    m_rotation;
	double m_page_height;
	double m_font_size;
	double m_scale;
	unsigned int m_break_op;
public:
	std::ostream * m_draw_debug_stream;
	std::ostream * m_page_debug_stream;
	typedef PDF::Point Point;
	PagePaintHelper(wxDC & theDC, int r=0, double sc=1.0);
	void set_draw_debug_stream(std::ostream* strm) { m_draw_debug_stream = strm; }
	void set_page_debug_stream(std::ostream* strm) { m_page_debug_stream = strm; }
	void set_break(unsigned int brop) { m_break_op = brop; }
	virtual ~PagePaintHelper() { };
	virtual const PDF::CTM & Matrix() { return m; }
	virtual void SetFont(const PDF::Font * font, double size);
	virtual void Text(PDF::Rect pos, double angle, std::wstring text, bool visible, const PDF::GraphicsState& gs);
	virtual void Line(const PDF::Point & p1, const PDF::Point & p2, const PDF::GraphicsState& gs);
	virtual void Size(PDF::Point size);
	virtual void Debug(unsigned int opnum, std::string s, const PDF::GraphicsState& gs);
};



#endif /* PAGEPAINTHELPER_H_INCLUDED */
