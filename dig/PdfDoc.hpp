#ifndef PDFDOC_HPP_INCLUDED
#define PDFDOC_HPP_INCLUDED

#include <string>
#include <PDF.hpp>
#include "wx/docview.h"

class PdfDoc:public wxDocument
{
	DECLARE_DYNAMIC_CLASS(PdfDoc)
	private:
		PDF::File file;
		/// pointer to the object cache
		PDF::ObjectsCache cache;
		/// Document Catalog dictionary
		PDF::OH doc_root;
		PDF::Document* document;
	public:
		PdfDoc();
		~PdfDoc();
		virtual bool DoOpenDocument(const wxString& filename);
		virtual bool OnSaveDocument(const wxString& filename);
		virtual bool OnCloseDocument();

		/// returns root object id for a given generation
		const PDF::ObjId & get_root(long generation=0) const { return file.get_root(generation); }
		const long generations_num() const { return file.generations_num(); }
		PDF::OH get_object(const PDF::ObjId & id);
		wxString get_file_brief() const;
		/// Pages shortcuts
		PDF::OH get_page_node(long pagenum) { return document ? document->get_page_node(pagenum) : PDF::OH(); }
		unsigned int get_pages_count() const { return document ? document->get_pages_count() : 0; }
};

#endif /* PDFDOC_HPP_INCLUDED */

