#include "WxTabDocument.hpp"
#include <iostream>
#include <wx/wx.h>

WxTabDocument * theDocument = NULL;

class WxErrorReporter:public ErrorReporter
{
public:
	virtual void ReportError(const char * prefix, const char * msg)
	{
		wxMessageBox(wxString(msg, wxConvUTF8), wxString(prefix, wxConvUTF8));
	}
};

WxErrorReporter * theReporter = NULL;

/* Private class */
class Metafile:public PDF::Media
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
    Metafile(wxDC & theDC, int r=0, double sc=1.0, bool deb=false):dc(theDC),m_rotation(r),m_font_size(10.0),m_scale(sc),m_debug(deb) { }
    virtual ~Metafile() { };
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

void Metafile::Size(Point size)
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

void Metafile::SetFont(const PDF::Font * font, double size)
{
	if(size < 4) size = 4;
	m_font_size = size;
	wxFont* f = wxTheFontList->FindOrCreateFont(int(size/1.8 /* empirical constant to get wxWidgets font of needed size */), wxSWISS, wxNORMAL /*wxITALIC*/, wxNORMAL/*wxBOLD*/);
	dc.SetFont(*f);
}

void Metafile::Text(Point pos, double rotation, std::wstring text, double width, double height)
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

void Metafile::Line(const Point & p1, const Point & p2)
{
	dc.SetPen(wxPen(*wxBLACK, 0, wxSOLID));
	dc.DrawLine( wxCoord(p1.x), wxCoord(m_page_height - p1.y), wxCoord(p2.x), wxCoord(m_page_height - p2.y) );
	if(m_debug)
		std::clog << "LINE " << p1.dump() << '-' << p2.dump() << std::endl;
}




//===========================================================

WxTabDocument::WxTabDocument()
	:doc(NULL),page(NULL),m_rotation(0),m_scale(1.0),m_cur_page_num(0),m_error_handler(NULL)
{
	theReporter = new WxErrorReporter;
}

WxTabDocument::~WxTabDocument()
{
	delete page;
	delete theReporter;
}

bool WxTabDocument::LoadFile(const wxString & fn)
{
	try {
		filename = fn;
		if(doc) {
			delete page;
			page = NULL;
			delete doc;
			doc = NULL;
			file.close();
		}

		file.debug(0);
#ifdef _MSC_VER
		file.open(fn.wc_str());
#else
		file.open(fn.utf8_str());
#endif
		if(! file.load()) {
			file.close();
			return false;
		}

		std::clog << "+ File loaded" << std::endl;
//		f.dump();

		doc = new PDF::Document(file, 5/*debug level*/);
		std::clog << "+ Document loaded" << std::endl;
		// PDF::Object::m_debug=true;
		// doc.dump();
		return true;
	}
	catch(std::string s) {
		std::cerr << "Some strange exception: " << s << std::endl;
		if(m_error_handler)
			m_error_handler->ReportError("Some strange exception", s.c_str());
	}
	catch(std::exception & e) {
		std::cerr << "Unknown exception:\n  " << e.what() << std::endl;
		if(m_error_handler)
			m_error_handler->ReportError("Unknown exception", e.what());
	}
	catch(...) {
		std::cerr << "Unknown exception!" << std::endl;
		if(m_error_handler)
			m_error_handler->ReportError("Unknown error", "Catched unknown exception");
	}
	/* reached only after exception */
	file.close();
	return false;
}

bool WxTabDocument::LoadPage(int num)
{
	if(page) delete page;
	page = NULL;
	try {
		if(num <= 0)
			num = 1;
		if((unsigned int)num > doc->get_pages_count())
			num = doc->get_pages_count();
		page = new PDF::Page();
		page->debug(5);
		std::clog << "Loading " << num << "th page" << std::endl;
		page->load(doc->get_page_node(num-1));
		std::clog << page->dump() << std::endl;
		m_cur_page_num = num;
		tabulator.full_process(page);
		return true;
	}
	catch(std::string s) {
		std::cerr << "Some strange exception: " << s << std::endl;
		if(m_error_handler)
			m_error_handler->ReportError("Some strange exception", s.c_str());
	}
	catch(std::exception & e) {
		std::cerr << "Unknown exception:\n  " << e.what() << std::endl;
		if(m_error_handler)
			m_error_handler->ReportError("Unknown exception", e.what());
	}
	catch(...) {
		std::cerr << "Unknown exception!" << std::endl;
		if(m_error_handler)
			m_error_handler->ReportError("Unknown error", "Catched unknown exception");
	}
	/* reached only after exception */
	return false;
}

void WxTabDocument::Draw(PDF::Media * mf)
{
	page->draw(mf);
}

void WxTabDocument::Draw(wxDC * dc)
{
	Metafile mf(*dc, m_rotation, m_scale);
	page->draw(&mf);
	tabulator.Draw(dc, m_scale);
}

bool WxTabDocument::Open(const wxString & fn, unsigned int page)
{
	if(! LoadFile(fn)) return false;
	if(! LoadPage(page)) return false;
	retab();
	return true;
}

void WxTabDocument::ExportPage( int pagenum, Tabulator::Table::Exporter * exporter )
{
	PDF::Page * p=new PDF::Page();
	if(p)
	{
		std::clog << "Page " << pagenum << std::endl;
		p->load(doc->get_page_node(pagenum-1)); // 0-based

		std::clog << "Drawing page " << pagenum << std::endl;
		tabulator.full_process(p); // do the final pretty structure construction!
		tabulator.output(exporter);
		delete p;
	}
}

