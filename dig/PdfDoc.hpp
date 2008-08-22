#ifndef PDFDOC_HPP_INCLUDED
#define PDFDOC_HPP_INCLUDED

#include <string>
#include <PDF.hpp>
#include "wx/docview.h"

class PdfDoc:public wxDocument
{
	DECLARE_DYNAMIC_CLASS(DrawingDocument)
	private:
		PDF::File file;
		/// pointer to the object cache
		PDF::ObjectsCache cache;
		/// Document Catalog dictionary
		PDF::OH doc_root;
	public:
		PdfDoc();
		virtual bool OnOpenDocument(const wxString& filename);
		virtual bool OnSaveDocument(const wxString& filename);
		virtual bool OnCloseDocument();

		/// returns root object id for a given generation
		const PDF::ObjId & get_root(long generation=0) const { return file.get_root(generation); }
		PDF::OH get_object(const PDF::ObjId & id);

/*		bool open(std::string fname);
		void close();
		*/
};

extern PdfDoc * theDoc;

#endif /* PDFDOC_HPP_INCLUDED */
