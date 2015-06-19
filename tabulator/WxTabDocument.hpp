#ifndef MYDOCUMENT_HPP_INCLUDED
#define MYDOCUMENT_HPP_INCLUDED
#include <libpdf/PDF.hpp>
#include <string>
#include "WxTabTabulator.hpp"
#include "ErrorReporterInterface.hpp"

class wxDC;
class WxTabDocument
{
	private:
		PDF::File file;
		PDF::Document * doc;
		PDF::Page * page;
		int m_rotation;
		double m_scale;
		int m_cur_page_num;
		wxString filename;
		ErrorReporter * m_error_handler;
	public:
		WxTabDocument();
		~WxTabDocument();
		void SetErrorHandler(ErrorReporter * e) { m_error_handler = e; }
		bool LoadFile(const wxString & fname);
		bool LoadPage(int num);
		void Draw(PDF::Media * mf);
		void Draw(wxDC * dc);
		void ExportPage(int pagenum, Tabulator::Table::Exporter * exporter);
		wxString GetName() const { return filename; }
		int GetPagesNum() const { return doc?doc->get_pages_count():0; }
		int GetPageNum() const { return m_cur_page_num; }
		PDF::Page * GetPageObject() { return page; }
		void Refresh() { }
		void Rotate(int r)
		{
			m_rotation = r;
			tabulator.metafile.set_rotation(r);
			tabulator.full_process(page);
			Refresh();
		}
		void Scale(unsigned int pr) { m_scale = pr/100.0; }
		WxTabTabulator tabulator;
		void retab() { tabulator.full_process(page); }
		bool export_ok() const { return tabulator.ok(); }
		bool Open(const wxString & fn, unsigned int page = 1);
};

extern WxTabDocument * theDocument;


#endif /* MYDOCUMENT_HPP_INCLUDED */

