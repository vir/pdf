#ifndef MYDOCUMENT_HPP_INCLUDED
#define MYDOCUMENT_HPP_INCLUDED
#include <libpdf/PDF.hpp>
#include <string>
#include <wx/docview.h>
#include "WxTabTabulator.hpp"
#include "ErrorReporterInterface.hpp"

class wxDC;
class WxTabDocument: public wxDocument
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
	public: // wxDocument
		virtual bool DoOpenDocument(const wxString& file);
	public:
		WxTabDocument();
		~WxTabDocument();
		void SetErrorHandler(ErrorReporter * e) { m_error_handler = e; }
		bool LoadFile(const wxString & fname);
		bool LoadPage(int num);
		void DrawPage(PDF::Media * mf);
		void DrawGrid(wxDC * dc);
		void ExportPage(int pagenum, Tabulator::Table::Exporter * exporter);
		wxString GetName() const { return filename; }
		int GetPagesNum() const { return doc?doc->get_pages_count():0; }
		int GetPageNum() const { return m_cur_page_num; }
		wxSize GetPageSize() const;
		PDF::Page * GetPageObject() { return page; }
		void Refresh() { }
		void Rotate(int r)
		{
			m_rotation = r;
			tabulator.metafile.set_rotation(r);
			tabulator.full_process(page);
			Refresh();
		}
		int Rotation() const { return m_rotation; }
		void Scale(unsigned int pr) { m_scale = pr/100.0; }
		double Scale() const { return m_scale * 100.0; }
		WxTabTabulator tabulator;
		void retab() { tabulator.full_process(page); }
		bool export_ok() const { return tabulator.ok(); }
		bool Open(const wxString & fn, unsigned int page = 1);
		void DumpDebugInfo();
};

extern WxTabDocument * theDocument;


#endif /* MYDOCUMENT_HPP_INCLUDED */

