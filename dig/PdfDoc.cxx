#include "PdfDoc.hpp"

IMPLEMENT_DYNAMIC_CLASS(PdfDoc, wxDocument)

PdfDoc::PdfDoc()
	:cache(file)
{
std::cerr << "Created document" << std::endl;
}

bool PdfDoc::OnOpenDocument(const wxString& filename)
{
	cache.flush();
	if(! file.open(std::string(filename.mb_str())))
		return false;
	// XXX Causes a lot of disasters on win32
	//GetDocumentManager()->AddFileToHistory(filename);
	if(! file.load())
		return false;
//	file.dump();

	SetFilename(filename, true);
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

#if 0
bool PdfDoc::open(std::string fname)
{
	return true;
}

void PdfDoc::close()
{
}
#endif



