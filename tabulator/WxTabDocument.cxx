#include "WxTabDocument.hpp"
#include <iostream>
#include <wx/wx.h>
#include "../wxview/PagePaintHelper.h"

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

//===========================================================

WxTabDocument::WxTabDocument()
	:doc(NULL),page(NULL)
	,m_rotation(0),m_scale(1.0),m_cur_page_num(0)
	,m_error_handler(NULL)
{
	theReporter = new WxErrorReporter;
}

WxTabDocument::~WxTabDocument()
{
	delete page;
	delete doc;
	delete theReporter;
}

// doc/view support
bool WxTabDocument::DoOpenDocument(const wxString& file)
{
	if(LoadFile(file))
		return true;
	wxLogError(_("Failed to load document from the file \"%s\"."), file.c_str());
	return false;
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
		file.open(fn.wc_str(), PDF::File::MODE_READ);
#else
		file.open(static_cast<const char*>(fn.utf8_str()), PDF::File::MODE_READ);
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
		tabulator.options.reset();
		return true;
	}
	catch(std::string& s) {
		std::cerr << "Some strange exception: " << s << std::endl;
		if(m_error_handler)
			m_error_handler->ReportError("Some strange exception", s.c_str());
	}
	catch(std::exception& e) {
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

void WxTabDocument::DrawPage(PDF::Media * mf)
{
	page->draw(mf);
}

void WxTabDocument::DrawGrid(wxDC * dc)
{
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

wxSize WxTabDocument::GetPageSize() const
{
	PDF::Rect mb = page->get_meadia_box();
	wxSize s(mb.x2 + mb.x1, mb.y2 + mb.y1); // Add some margins
	s *= m_scale;
	switch(m_rotation)
	{
		case 1:
		case 3: return wxSize(s.y, s.x);
		default: return s;
	}
}

