#include "PdfDoc.hpp"
#include <wx/msgdlg.h>

wxIMPLEMENT_DYNAMIC_CLASS(PdfDoc, wxDocument)

PdfDoc::PdfDoc()
	:cache(file)
{
}

bool PdfDoc::DoOpenDocument(const wxString& filename)
{
	cache.flush();
#ifdef _MSC_VER
	if(! file.open(filename.wc_str(), PDF::File::MODE_READ))
#else
	if(! file.open(static_cast<const char*>(filename.utf8_str()), PDF::File::MODE_READ))
#endif
		return false;
	// XXX Causes a lot of disasters on win32
	GetDocumentManager()->AddFileToHistory(filename);
	try {
		if(! file.load())
			return false;
	}
	catch(PDF::Exception& e) {
		wxMessageBox(wxString::FromUTF8(e.what()), wxT("Exception"));
		return false;
	}
	catch(std::exception& e) {
		wxMessageBox(wxString::FromUTF8(e.what()), wxT("Exception"));
		return false;
	}
	catch(...) {
		wxMessageBox(wxT("Something strange"), wxT("Exception"));
		return false;
	}

	//SetFilename(filename, true);
	Modify(false);
	UpdateAllViews();
	return true;
}

bool PdfDoc::OnSaveDocument(const wxString& filename)
{
	return false;
}

bool PdfDoc::OnCloseDocument()
{
	return file.close();
}

PDF::OH PdfDoc::get_object(const PDF::ObjId & id)
{
	return cache.get_object(id);
}

wxString PdfDoc::get_file_brief() const
{
	std::stringstream ss;
	file.dump(ss);
	return wxString(ss.str().c_str(), wxConvUTF8);
}


