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
public:
	bool m_debug;
	typedef PDF::Point Point;
	PagePaintHelper(wxDC & theDC, int r=0, double sc=1.0, bool deb=false):dc(theDC),m_rotation(r),m_font_size(10.0),m_scale(sc),m_debug(deb) { }
	virtual ~PagePaintHelper() { };
	virtual const PDF::CTM & Matrix() { return m; }
	virtual void SetFont(const PDF::Font * font, double size);
	virtual void Text(Point pos, double rotation, std::wstring text, double width, double height);
	virtual void Line(const Point & p1, const Point & p2);
	virtual void Size(Point size);
	virtual void Debug(std::string s)
	{
		if(m_debug)
			std::clog << "DEBUG: " << s << std::endl;
	}
};



#endif /* PAGEPAINTHELPER_H_INCLUDED */
